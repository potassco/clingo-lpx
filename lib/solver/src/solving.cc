#include <clingo-lpx/parsing.hh>
#include <clingo-lpx/solving.hh>
#include <clingo-lpx/util.hh>

#include <algorithm>
#include <clingo/core.hh>
#include <clingo/stats.hh>
#include <cstddef>
#include <exception>
#include <functional>
#include <limits>
#include <mutex>
#include <optional>
#include <ostream>
#include <tuple>
#include <utility>

namespace ClingoLPX {

namespace {

auto as_value(RationalQ const &a, RationalQ *b) -> RationalQ {
    static_cast<void>(b);
    return a;
}

auto as_value(RationalQ const &a, Rational *b) -> Rational {
    static_cast<void>(b);
    return a.as_rational();
}

//! Select the candidate with the best score breaking ties by the smallest index.
//!
//! With a constant score, this selects the candidate with the smallest index
//! (Bland's rule).
template <class Score, class Better = std::less<>> class Selector {
  public:
    //! Offer a candidate and return true if it is the best one so far.
    auto offer(index_t index, Score score) -> bool {
        if (empty() || Better{}(score, score_) || (!Better{}(score_, score) && index < index_)) {
            index_ = index;
            score_ = std::move(score);
            return true;
        }
        return false;
    }

    //! Check whether a candidate has been offered.
    [[nodiscard]] auto empty() const -> bool { return index_ == none; }

    //! Get the index of the best candidate.
    [[nodiscard]] auto index() const -> index_t { return index_; }

  private:
    static constexpr index_t none = std::numeric_limits<index_t>::max();
    index_t index_ = none;
    Score score_{};
};

//! Round the bound of an expression that can only take integer values.
//!
//! Strict relations are replaced by non-strict ones. The function returns
//! false if the relation is an equality with a fractional right-hand side.
auto round_bound(Rational &rhs, Relation &rel) -> bool {
    switch (rel) {
        case Relation::LessEqual: {
            rhs = as_rational(floor(rhs));
            break;
        }
        case Relation::Less: {
            rhs = as_rational(ceil(rhs) - 1);
            rel = Relation::LessEqual;
            break;
        }
        case Relation::GreaterEqual: {
            rhs = as_rational(ceil(rhs));
            break;
        }
        case Relation::Greater: {
            rhs = as_rational(floor(rhs) + 1);
            rel = Relation::GreaterEqual;
            break;
        }
        case Relation::Equal: {
            return is_integer(rhs);
        }
    }
    return true;
}

//! Scale the coefficients of a row to coprime integers.
//!
//! The scaling factor is positive and returned.
auto scale_row(std::vector<std::pair<index_t, Rational>> &row) -> Rational {
    Integer l{1};
    for (auto const &[j, v] : row) {
        l = lcm(l, v.den());
    }
    Integer g{0};
    auto f = as_rational(std::move(l));
    for (auto const &[j, v] : row) {
        g = gcd(g, (v * f).num());
    }
    f /= g;
    for (auto &[j, v] : row) {
        v *= f;
    }
    return f;
}

} // namespace

template <typename Value> void ObjectiveState<Value>::reset() {
    value_ = Value{};
    generation_ = 0;
    bounded_ = true;
}

template <typename Value> void ObjectiveState<Value>::update(std::pair<Value, bool> value) {
    std::unique_lock<std::shared_mutex> lock{mutex_};
    if (bounded_ && (!value.second || generation_ == 0 || value.first > value_)) {
        ++generation_;
        value_ = std::move(value.first);
        bounded_ = value.second;
    }
}

template <typename Value>
auto ObjectiveState<Value>::value(size_t &generation) -> std::optional<std::pair<Value, bool>> {
    std::shared_lock<std::shared_mutex> lock{mutex_};
    if (generation != generation_) {
        generation = generation_;
        return std::make_pair(value_, bounded_);
    }
    return std::nullopt;
}

template <typename Value> auto bound_rel(Relation rel) -> Solver<Value>::BoundRelation {
    switch (rel) {
        case Relation::Less:
        case Relation::LessEqual: {
            return Solver<Value>::BoundRelation::LessEqual;
        }
        case Relation::Greater:
        case Relation::GreaterEqual: {
            return Solver<Value>::BoundRelation::GreaterEqual;
        }
        case Relation::Equal: {
            break;
        }
    }
    return Solver<Value>::BoundRelation::Equal;
}

template <typename Value> auto bound_val(Rational x, Relation rel) -> Value;

template <> auto bound_val<Rational>(Rational x, Relation rel) -> Rational {
    static_cast<void>(rel);
    assert(rel != Relation::Less && rel != Relation::Greater);
    return x;
}

template <> auto bound_val<RationalQ>(Rational x, Relation rel) -> RationalQ {
    switch (rel) {
        case Relation::Less: {
            return RationalQ{std::move(x), -1};
        }
        case Relation::Greater: {
            return RationalQ{std::move(x), 1};
        }
        case Relation::LessEqual:
        case Relation::GreaterEqual:
        case Relation::Equal: {
            break;
        }
    }
    return RationalQ{std::move(x)};
}

template <typename Value> struct Solver<Value>::Prepare {
    Prepare(Solver &slv, SymbolMap const &map) : slv{slv}, map{map} {
        slv.variables_.resize(map.size());
        slv.n_non_basic_ = static_cast<index_t>(map.size());
        for (index_t i = 0; i != slv.n_non_basic_; ++i) {
            slv.variables_[i].index = i;
            slv.variables_[i].reverse_index = i;
        }
    }

    auto get_non_basic(const Clingo::Symbol &var) -> index_t {
        auto jt = map.find(var);
        assert(jt != map.end());
        return slv.variables_[jt->second].reverse_index;
    }

    auto is_integer(std::vector<Term> const &x) -> bool {
        return std::ranges::all_of(x, [this](Term const &term) -> bool {
            auto jt = map.find(term.var);
            assert(jt != map.end());
            return slv.variables_[jt->second].integer;
        });
    }

    auto add_basic() -> index_t {
        auto index = static_cast<index_t>(slv.variables_.size());
        slv.variables_.emplace_back();
        slv.variables_.back().index = index;
        slv.variables_.back().reverse_index = index;
        return slv.n_basic_++;
    }

    auto add_row(std::vector<Term> const &x) -> std::vector<std::pair<index_t, Rational>> {
        std::vector<std::pair<index_t, Rational>> row;
        row.reserve(x.size());

        // add non-basic variables
        for (auto const &term : x) {
            row.emplace_back(get_non_basic(term.var), term.co);
        }

        return row;
    }

    Solver &slv;
    SymbolMap const &map;
};

template <typename Value> auto Solver<Value>::Solver::Bound::compare(Value const &value) const -> bool {
    switch (rel) {
        case BoundRelation::Equal: {
            return value == this->value;
        }
        case BoundRelation::LessEqual: {
            return value <= this->value;
        }
        case BoundRelation::GreaterEqual: {
            break;
        }
    }
    return value >= this->value;
}

template <typename Value> auto Solver<Value>::Solver::Bound::conflicts(Bound const &other) const -> bool {
    switch (rel) {
        case BoundRelation::Equal: {
            return other.rel == BoundRelation::Equal ? value != other.value : other.conflicts(*this);
        }
        case BoundRelation::LessEqual: {
            return other.rel != BoundRelation::LessEqual && value < other.value;
        }
        case BoundRelation::GreaterEqual: {
            return other.rel != BoundRelation::GreaterEqual && other.value < value;
        }
    }
    return value >= this->value;
}

template <typename Value>
auto Solver<Value>::Variable::update_upper(Solver &s, Clingo::Assignment ass, Bound const &bound) -> bool {
    if (!has_upper() || bound.value < upper()) {
        if (!has_upper() || ass.level(upper_bound->lit) < ass.decision_level()) {
            s.bound_trail_.emplace_back(bound.variable, BoundRelation::LessEqual, upper_bound);
        }
        upper_bound = &bound;
    }
    return !has_lower() || lower() <= upper();
}

template <typename Value>
auto Solver<Value>::Variable::update_lower(Solver &s, Clingo::Assignment ass, Bound const &bound) -> bool {
    if (!has_lower() || bound.value > lower()) {
        if (!has_lower() || ass.level(lower_bound->lit) < ass.decision_level()) {
            s.bound_trail_.emplace_back(bound.variable, BoundRelation::GreaterEqual, lower_bound);
        }
        lower_bound = &bound;
    }
    return !has_upper() || lower() <= upper();
}

template <typename Value>
auto Solver<Value>::Variable::update(Solver &s, Clingo::Assignment ass, Bound const &bound) -> bool {
    switch (bound.rel) {
        case BoundRelation::LessEqual: {
            return update_upper(s, ass, bound);
        }
        case BoundRelation::GreaterEqual: {
            return update_lower(s, ass, bound);
        }
        case BoundRelation::Equal: {
            break;
        }
    }
    return update_upper(s, ass, bound) && update_lower(s, ass, bound);
}

template <typename Value> void Solver<Value>::Variable::set_value(Solver &s, index_t lvl, Value const &val, bool add) {
    // We can always assume that the assignment on a previous level was satisfying.
    // Thus, we simply store the old values to be able to restore them when backtracking.
    if (lvl != level) {
        s.assignment_trail_.emplace_back(level, static_cast<index_t>(this - s.variables_.data()), value);
        level = lvl;
    }
    if (add) {
        value += val;
    } else {
        value = val;
    }
}

template <typename Value> auto Solver<Value>::Variable::has_conflict() const -> bool {
    return (has_lower() && value < lower()) || (has_upper() && value > upper());
}

void Statistics::reset() { *this = {}; }

template <typename Value> Solver<Value>::Solver(Options const &options) : options_{options} {}

template <typename Value> auto Solver<Value>::basic_(index_t i) -> Solver<Value>::Variable & {
    assert(i < n_basic_);
    return variables_[variables_[i + n_non_basic_].index];
}

template <typename Value> auto Solver<Value>::non_basic_(index_t j) -> Solver<Value>::Variable & {
    assert(j < n_non_basic_);
    return variables_[variables_[j].index];
}

template <typename Value> void Solver<Value>::enqueue_(index_t i) {
    assert(i < n_basic_);
    auto ii = variables_[i + n_non_basic_].index;
    auto &xi = variables_[ii];
    if (!xi.queued && xi.has_conflict()) {
        conflicts_.emplace_back(ii);
        std::ranges::push_heap(conflicts_, std::greater<>{});
        xi.queued = true;
    }
    // Note that this explicitely marks the row not the variable to propagate.
    if (options_.propagate_mode == PropagateMode::Changed && !variables_[i].propagate) {
        variables_[i].propagate = true;
        propagate_queue_.emplace_back(i);
    }
}

template <typename Value> auto Solver<Value>::get_value(index_t i) const -> Value { return variables_[i].value; }

template <typename Value> auto Solver<Value>::get_objective() const -> std::optional<std::pair<Value, bool>> {
    if (objective_) {
        return std::make_pair(variables_[objective_.var].value, objective_.bounded);
    }
    return std::nullopt;
}

template <typename Value>
auto Solver<Value>::prepare(Clingo::Assignment ass, Clingo::PropagateInit init, SymbolMap const &symbols,
                            std::vector<Inequality> const &inequalities, std::vector<Term> const &objective,
                            std::vector<index_t> const &integers, bool master) -> bool {
    Prepare prep{*this, symbols};
    for (auto i : integers) {
        variables_[i].integer = true;
    }
    ints_.active = !integers.empty();
    // the inequality is false if there is no integer solution
    auto reject = [&](Inequality const &x) -> bool { return !master || init.add_clause({-x.lit}); };
    for (auto const &x : inequalities) {
        if (ass.is_false(x.lit)) {
            continue;
        }

        // transform inequality into row suitable for tableau
        auto row = prep.add_row(x.lhs);
        auto rhs = x.rhs;
        auto rel = x.rel;
        bool integer = ints_.active && !row.empty() && prep.is_integer(x.lhs);

        // check bound against 0
        if (row.empty()) {
            if (!master) {
                continue;
            }
            switch (x.rel) {
                case Relation::Less: {
                    if (x.rhs <= 0 && !init.add_clause({-x.lit})) {
                        return false;
                    }
                    break;
                }
                case Relation::LessEqual: {
                    if (x.rhs < 0 && !init.add_clause({-x.lit})) {
                        return false;
                    }
                    break;
                }
                case Relation::Greater: {
                    if (x.rhs >= 0 && !init.add_clause({-x.lit})) {
                        return false;
                    }
                    break;
                }
                case Relation::GreaterEqual: {
                    if (x.rhs > 0 && !init.add_clause({-x.lit})) {
                        return false;
                    }
                    break;
                }
                case Relation::Equal: {
                    if (x.rhs != 0 && !init.add_clause({-x.lit})) {
                        return false;
                    }
                    break;
                }
            }
        }
        // add a bound to a non-basic variable
        else if (row.size() == 1) {
            auto const &[j, v] = row.front();
            if (v < 0) {
                rel = invert(rel);
            }
            rhs /= v;
            if (integer && !round_bound(rhs, rel)) {
                if (!reject(x)) {
                    return false;
                }
                continue;
            }
            bounds_.emplace(
                x.lit, Bound{bound_val<Value>(std::move(rhs), rel), variables_[j].index, x.lit, bound_rel<Value>(rel)});
        }
        // add an inequality
        else {
            // scale rows over integer variables so that the slack variable is integer, too
            if (integer) {
                rhs *= scale_row(row);
                if (!round_bound(rhs, rel)) {
                    if (!reject(x)) {
                        return false;
                    }
                    continue;
                }
            }
            auto i = prep.add_basic();
            variables_.back().integer = integer;
            bounds_.emplace(x.lit, Bound{
                                       bound_val<Value>(std::move(rhs), rel),
                                       static_cast<index_t>(variables_.size() - 1),
                                       x.lit,
                                       bound_rel<Value>(rel),
                                   });
            for (auto const &[j, v] : row) {
                tableau_.set(i, j, v);
            }
        }
    }

    // add objective function to tableau
    if (!objective.empty()) {
        objective_.active = true;
        auto row = prep.add_row(objective);
        auto add_row = [&, this]() -> auto {
            auto i = prep.add_basic();
            for (auto const &[j, v] : row) {
                tableau_.set(i, j, v);
            }
            return static_cast<index_t>(variables_.size()) - 1;
        };
        if (options_.global_objective.has_value()) {
            objective_.bound_var = add_row();
        }
        objective_.var = add_row();
    }

    for (index_t i = 0; i < n_basic_; ++i) {
        enqueue_(i);
    }

    for (auto const &[lit, bound] : bounds_) {
        variables_[bound.variable].bounds.emplace_back(&bound);
    }
    cuts_.offset = static_cast<index_t>(variables_.size());
    assert_extra(check_tableau_());
    assert_extra(check_basic_());
    assert_extra(check_non_basic_());

    // Add binary clauses for the following bounds:
    //
    //   x >= u implies not x <= l for all l < u.
    if (options_.propagate_conflicts && master) {
        for (auto const &var : variables_) {
            for (auto it_a = var.bounds.begin(), ie = var.bounds.end(); it_a != ie; ++it_a) {
                auto const &ba = **it_a;
                for (auto it_b = it_a + 1; it_b != ie; ++it_b) {
                    auto const &bb = **it_b;
                    if (ba.lit != -bb.lit && !ass.is_false(bb.lit) && ba.conflicts(bb)) {
                        conflict_clause_.clear();
                        conflict_clause_.emplace_back(-ba.lit);
                        conflict_clause_.emplace_back(-bb.lit);
                        if (!init.add_clause(conflict_clause_) || !init.propagate()) {
                            return false;
                        }
                    }
                }
            }
        }
    }

    return true;
}

template <typename Value> void Solver<Value>::debug_() {
    std::cerr << "tableau:" << '\n';
    tableau_.debug("  ");
    if (objective_) {
        auto z = variables_[objective_.var].reverse_index - n_non_basic_;
        std::cerr << "objective variable:\n  y_" << z << std::endl;
    }

    std::cerr << "basic assignment:" << '\n';
    for (index_t i = 0; i < n_basic_; ++i) {
        auto &x_i = basic_(i);
        std::cerr << "  y_" << i << " = " << x_i.value << " for ";
        if (x_i.has_lower()) {
            std::cerr << x_i.lower();
        } else {
            std::cerr << "#inf";
        }
        std::cerr << " <= y_" << i << " <= ";
        if (x_i.has_upper()) {
            std::cerr << x_i.upper();
        } else {
            std::cerr << "#sup";
        }
        std::cerr << '\n';
    }
    std::cerr << "non-basic assignment:" << '\n';
    for (index_t i = 0; i < n_non_basic_; ++i) {
        auto &x_i = non_basic_(i);
        std::cerr << "  x_" << i << " = " << x_i.value << " for ";
        if (x_i.has_lower()) {
            std::cerr << x_i.lower();
        } else {
            std::cerr << "#inf";
        }
        std::cerr << " <= x_" << i << " <= ";
        if (x_i.has_upper()) {
            std::cerr << x_i.upper();
        } else {
            std::cerr << "#sup";
        }
        std::cerr << '\n';
    }
}

template <typename Value> void Solver<Value>::optimize() {
    assert(!objective_ || variables_[objective_.var].reverse_index >= n_non_basic_);
    // First, we select an entering variable x_e among the non-basic variables
    // corresponding to a non-zero coefficient a_ze in the objective function
    // (assuming that the objective value corresponds to basic variable x_z).
    //
    // Case a_ze > 0.
    //   We can select x_e as an entering variable if it is less than its upper
    //   bound (or has no upper bound).
    //
    // Case a_ze < 0.
    //   We can select x_e as an entering variable if it is greater than its
    //   lower bound (or has no lower bound).
    //
    // Second, we select a leaving variable x_l among the basic variables. This
    // has to be a variable that can be adjusted so that the value a_ze*x_e
    // increases.
    //
    // We pick a row l such that a_le is non-zero. It has the form
    //   x_l = C + a_le*x_e.
    // This can be rearranged as
    //   x_e = x_l/a_le - C/a_le.
    // We see that the sign of a_le determines how x_l and x_e correlate.
    //
    // We consider the signs of the coefficients a_ze and a_le.
    // Case a_ze > 0:
    //   Here we try to increase x_e to increase a_ze*x_e.
    //   Case a_le > 0.
    //      Here we try to increase x_l to increase x_e.
    //      Case x_l has no upper bound or setting x_l to its upper bound would
    //      violate x_e's upper bound (or make it tight).
    //        The row is unbounded.
    //      Case x_l can be set to its upper bound.
    //        We obtain an increase of x_e smaller than its upper bound.
    //   Case a_le < 0.
    //      Here we try to decrease x_l to increase x_e.
    //      Case x_l has no lower bound or setting x_l to its lower bound would
    //      violate x_e's upper bound (or make it tight).
    //        The row is unbounded.
    //      Case x_l can be set to its lower bound.
    //        We obtain an increase of x_e smaller than its upper bound.
    //
    //   We now consider which candidate to chose as a leaving variable.
    //   Case all rows for leaving variables candidates are unbounded.
    //     We can set x_e to its upper bound and adjust the corresponding
    //     values of the x_l without causing conflicts. At this point x_e is no
    //     entering variable anymore. If x_e has no upper bound, the objective
    //     is unbounded.
    //   Case there is an x_l that can be set to its bound to make a
    //   constraint tight.
    //     We select the leaving variable that causes the least change to x_e
    //     to guarantee that no bounds are violated. This is the standard
    //     pivoting case. Unlike in algorithm to find a basic feasible
    //     solution, this pivot cannot cause a conflict.
    // Case a_ze < 0:
    //   symmetric
    //
    // By default, the entering variable is chosen with the largest coefficient
    // |a_ze| (Dantzig's rule). Because all coefficients in the objective row
    // share the denominator d_z, comparing the numerators suffices. This rule
    // usually needs few pivots but can cycle on degenerate pivots, which do
    // not change the objective value. After a number of consecutive
    // degenerate pivots (see bland_threshold_()), we switch to choosing the
    // entering variable with the smallest variable index (Bland's rule) until
    // the objective improves again. The leaving variable is always chosen
    // with the smallest variable index among the candidates. Bland's rule
    // cannot cycle, so each phase of degenerate pivots ends, and since every
    // other step strictly improves the objective, the algorithm terminates.
    //
    // The objective variable never leaves the basis because it has no bounds
    // (the bounds for the global objective are added to the copy
    // objective_.bound_var).
    if (!objective_) {
        return;
    }

    auto threshold = bland_threshold_();
    size_t degenerate = 0;

    assert_extra(check_solution_());
    while (true) {
        // the objective assigned to variable y_z
        auto z = variables_[objective_.var].reverse_index - n_non_basic_;

        // select entering variable x_e
        bool pos_a_ze = false;
        bool dantzig = degenerate < threshold && options_.entering_rule == EnteringRule::Dantzig;
        auto sel = Selector<Integer, std::greater<>>{};
        tableau_.update_row(z, [&, this](index_t j, Integer const &a_zj, Integer const &d_z) -> void {
            auto jj = variables_[j].index;
            auto &x_j = variables_[jj];
            bool pos_a_zj = (a_zj > 0) == (d_z > 0);
            if (pos_a_zj ? x_j.has_upper() && x_j.value >= x_j.upper() : x_j.has_lower() && x_j.value <= x_j.lower()) {
                return;
            }
            if (sel.offer(jj, dantzig ? magnitude(a_zj) : Integer{})) {
                pos_a_ze = pos_a_zj;
            }
        });

        // the solution is optimal if there is no entering variable
        if (sel.empty()) {
            assert_extra(check_solution_());
            objective_.bounded = true;
            return;
        }

        // select leaving variable y_l
        auto ee = sel.index();
        auto &x_e = variables_[ee];
        auto v_e = Value{};
        auto e = x_e.reverse_index;
        assert(ee == variables_[e].index);
        auto ll = static_cast<index_t>(variables_.size());
        Value const *bound_l = nullptr;

        tableau_.update_col(e, [&, this](index_t i, Integer const &a_ie, Integer const &d_i) -> auto {
            auto &y_i = basic_(i);
            bool pos_a_ie = ((a_ie > 0) == (d_i > 0));
            bool increase = pos_a_ie == pos_a_ze;
            if (increase ? !y_i.has_upper() : !y_i.has_lower()) {
                return;
            }
            auto ii = variables_[i + n_non_basic_].index;
            assert(i == y_i.reverse_index - n_non_basic_);
            Value const &v_i = increase ? y_i.upper() : y_i.lower();
            // we compute the updated value of x_e (see Solver::pivot_)
            Value v = x_e.value + mul_div(v_i - y_i.value, d_i, a_ie);
            if (pos_a_ze ? x_e.has_upper() && v >= x_e.upper() : x_e.has_lower() && v <= x_e.lower()) {
                return;
            }
            if (bound_l == nullptr || (pos_a_ze ? v < v_e : v > v_e) || (ii < ll && v == v_e)) {
                bound_l = &v_i;
                ll = ii;
                v_e = std::move(v);
            }
        });

        // assign values on the last decision level
        auto level = trail_offset_.empty() ? 0 : trail_offset_.back().level;

        // increase objective value by pivoting
        if (bound_l != nullptr) {
            degenerate = v_e == x_e.value ? degenerate + 1 : 0;
            auto l = variables_[ll].reverse_index - n_non_basic_;
            pivot_(level, l, e, *bound_l);
        } else {
            degenerate = 0;
            // variable x_e is unbounded
            if (pos_a_ze ? !x_e.has_upper() : !x_e.has_lower()) {
                assert_extra(check_solution_());
                objective_.bounded = false;
                return;
            }
            // increase/decrease x_e
            update_(level, e, pos_a_ze ? x_e.upper() : x_e.lower());
        }
    }
}

template <typename Value> void Solver<Value>::store_sat_assignment() {
    for (auto &[level, index, number] : assignment_trail_) {
        variables_[index].level = 0;
    }
    for (auto it = trail_offset_.rbegin(), ie = trail_offset_.rend(); it != ie; ++it) {
        if (it->assignment > 0) {
            it->assignment = 0;
        } else {
            break;
        }
    }
    assignment_trail_.clear();
    cuts_.fresh.clear();
}

template <typename Value>
auto Solver<Value>::update_bound_(Clingo::Assignment ass, Clingo::PropagateControl ctl, Bound const &bound) -> bool {
    auto &x = variables_[bound.variable];
    if (!x.update(*this, ass, bound)) {
        conflict_clause_.clear();
        conflict_clause_.emplace_back(-x.upper_bound->lit);
        conflict_clause_.emplace_back(-x.lower_bound->lit);
        std::ignore = ctl.add_clause(conflict_clause_);
        return false;
    }
    if (x.reverse_index < n_non_basic_) {
        if (x.has_lower() && x.value < x.lower()) {
            update_(ass.decision_level(), x.reverse_index, x.lower());
        } else if (x.has_upper() && x.value > x.upper()) {
            update_(ass.decision_level(), x.reverse_index, x.upper());
        }
    } else {
        enqueue_(x.reverse_index - n_non_basic_);
    }
    return true;
}

template <typename Value> auto Solver<Value>::assert_bound_(Clingo::PropagateControl ctl, Value value) -> bool {
    // Adds a new bound associated with a new literal that is made true by a
    // unit clause. This ensures that the solver takes care of backtracking and
    // reasserting the literal.
    auto lit = ctl.add_literal();
    ctl.add_watch(lit);
    bounds_.emplace(lit, Bound{std::move(value), objective_.bound_var, lit, BoundRelation::GreaterEqual});
    conflict_clause_.clear();
    conflict_clause_.emplace_back(lit);
    return ctl.add_clause(conflict_clause_) && ctl.propagate();
}

template <typename Value>
auto Solver<Value>::integrate_objective(Clingo::PropagateControl ctl, ObjectiveState<Value> &state) -> bool {
    // Here we discard bounded solutions by asserting that the objective value
    // is greater than the current bound + an epsilon value taken from the
    // configuration.
    //
    // Note that this function associates many bounds with the objective
    // variable. In principle, it would be nice to remove the old ones. I think
    // that clasp also offers a method to pop a literal introduced during
    // search.
    if (!options_.global_objective.has_value()) {
        return true;
    }
    auto value = state.value(objective_.generation);
    if (!value.has_value()) {
        return true;
    }
    if (!value->second) {
        objective_.discard_bounded = true;
        return true;
    }
    return assert_bound_(ctl,
                         std::move(value->first) + as_value(*options_.global_objective, static_cast<Value *>(nullptr)));
}

template <typename Value> auto Solver<Value>::discard_bounded(Clingo::PropagateControl ctl) -> bool {
    // Here we discard bounded solutions by asserting that the objective
    // is greater than the current optimal objective.
    if (!objective_ || !options_.global_objective.has_value() || !objective_.bounded || !objective_.discard_bounded) {
        return true;
    }
    return assert_bound_(ctl, variables_[objective_.var].value + 1);
}

template <typename Value>
auto Solver<Value>::solve(Clingo::Assignment ass, Clingo::PropagateControl ctl, Clingo::SolverLiteralSpan lits)
    -> bool {
    index_t i{0};
    index_t j{0};
    Value const *v{nullptr};

    auto level = ass.decision_level();

    if (trail_offset_.empty() || trail_offset_.back().level < level) {
        trail_offset_.emplace_back(TrailOffset{
            level,
            static_cast<index_t>(bound_trail_.size()),
            static_cast<index_t>(assignment_trail_.size()),
        });
    }

    for (auto lit : lits) {
        for (auto it = bounds_.find(lit), ie = bounds_.end(); it != ie && it->first == lit; ++it) {
            auto const &[lit_a, bound_a] = *it;
            assert(lit == lit_a);
            if (!update_bound_(ass, ctl, bound_a)) {
                return false;
            }
        }
    }

    assert_extra(check_tableau_());
    assert_extra(check_basic_());
    assert_extra(check_non_basic_());

    // The configured selection rules can cycle, so we switch to Bland's rule
    // after a number of pivots.
    auto threshold = bland_threshold_();
    size_t pivots = 0;
    while (true) {
        switch (select_(i, j, v, pivots >= threshold)) {
            case State::Satisfiable: {
                if (options_.store_sat_assignment == StoreSATAssignments::Partial) {
                    store_sat_assignment();
                }
                return propagate_(ass, ctl);
            }
            case State::Unsatisfiable: {
                std::ignore = ctl.add_clause(conflict_clause_);
                return false;
            }
            case State::Unknown: {
                assert(v != nullptr);
                pivot_(level, i, j, *v); // NOLINT
                ++pivots;
            }
        }
    }
}

template <typename Value> auto Solver<Value>::propagate_(Clingo::Assignment ass, Clingo::PropagateControl ctl) -> bool {
    // In principle we could also propgate more bounds (see clingcon). This
    // would very likely be too expensive.
    //
    // Currently, the function propagates rows whose values have changed due to
    // a bound change (or all rows). In principal, the simplex algorithm sets
    // the value of a non-basic variable to a lower or upper bound and then
    // updates the basic variable associated with the row. This means that
    // already the current assignment could be used for bound propagation
    // avoiding any arithmetic operations. However, we spend a little more
    // effort here to compute more bounds.
    if (options_.propagate_mode == PropagateMode::None) {
        return true;
    }
    std::vector<Clingo::SolverLiteral> lower_clause;
    std::vector<Clingo::SolverLiteral> upper_clause;
    auto propagate_row = [&](index_t i) -> auto {
        variables_[i].propagate = false;
        // An upper bound on y_i can only falsify bound literals of y_i with
        // relation >= or = and a lower bound only ones with relation <= or =.
        // Hence, we only compute the bounds that might be used. (Checking the
        // assignment here would be more precise but is expensive.)
        auto &y_i = basic_(i);
        bool need_upper = false;
        bool need_lower = false;
        for (auto const *bound : y_i.bounds) {
            need_upper = need_upper || bound->rel != BoundRelation::LessEqual;
            need_lower = need_lower || bound->rel != BoundRelation::GreaterEqual;
            if (need_upper && need_lower) {
                break;
            }
        }
        if (!need_upper && !need_lower) {
            return true;
        }
        std::optional<Value> lower;
        std::optional<Value> upper;
        if (need_lower) {
            lower = Value{0};
        }
        if (need_upper) {
            upper = Value{0};
        }
        // check if a constraint provides a bound
        tableau_.update_row(i, [&, this](index_t j, Integer const &a_ij, Integer const &d_i) -> auto {
            auto &x_j = non_basic_(j);
            bool pos_a_ij = (a_ij > 0) == (d_i > 0);
            if (pos_a_ij ? !x_j.has_lower() : !x_j.has_upper()) {
                lower = std::nullopt;
            }
            if (pos_a_ij ? !x_j.has_upper() : !x_j.has_lower()) {
                upper = std::nullopt;
            }
        });
        if (!upper.has_value() && !lower.has_value()) {
            return true;
        }
        lower_clause.clear();
        upper_clause.clear();
        // Compute the bounds. All terms of a row share the denominator d_i, so
        // we sum a_ij * v_j for the bounds v_j and divide once. Integer bounds,
        // which are common, are summed using integer arithmetic.
        Integer lower_int{0};
        Integer upper_int{0};
        Value lower_rest;
        Value upper_rest;
        Integer const *den = nullptr;
        tableau_.update_row(i, [&, this](index_t j, Integer const &a_ij, Integer const &d_i) -> auto {
            den = &d_i;
            auto &x_j = non_basic_(j);
            auto add = [&](std::vector<Clingo::SolverLiteral> &clause, std::optional<Value> const &bound,
                           Integer &sum_int, Value &sum_rest, Bound const *x_bound) -> void {
                if (bound.has_value() && x_bound != nullptr) {
                    if (is_integer(x_bound->value)) {
                        sum_int.add_mul(a_ij, rational_part(x_bound->value).num());
                    } else {
                        sum_rest += x_bound->value * a_ij;
                    }
                    clause.emplace_back(-x_bound->lit);
                }
            };
            if ((a_ij > 0) == (d_i > 0)) {
                add(lower_clause, lower, lower_int, lower_rest, x_j.lower_bound);
                add(upper_clause, upper, upper_int, upper_rest, x_j.upper_bound);
            } else {
                add(lower_clause, lower, lower_int, lower_rest, x_j.upper_bound);
                add(upper_clause, upper, upper_int, upper_rest, x_j.lower_bound);
            }
        });
        auto finish = [&](std::optional<Value> &bound, Integer &sum_int, Value &sum_rest) -> void {
            if (bound.has_value() && den != nullptr) {
                sum_rest += Value{as_rational(std::move(sum_int))};
                *bound = mul_div(sum_rest, Integer{1}, *den);
            }
        };
        finish(lower, lower_int, lower_rest);
        finish(upper, upper_int, upper_rest);
        auto propagate = [&, this](std::vector<Clingo::SolverLiteral> &clause, Bound const &bound) -> auto {
            clause.emplace_back(-bound.lit);
            bool ret = ctl.add_clause(clause) && ctl.propagate();
            clause.pop_back();
            ++statistics_.propagated_bounds;
            return ret;
        };
        // propagate the bounds
        if (upper.has_value()) {
            // y_i <= upper
            for (auto &bound : y_i.bounds) {
                if (bound->rel != BoundRelation::LessEqual && bound->value > *upper && !ass.is_false(bound->lit)) {
                    if (!propagate(upper_clause, *bound)) {
                        return false;
                    }
                }
            }
        }
        if (lower.has_value()) {
            // y_i >= lower
            for (auto &bound : y_i.bounds) {
                if (bound->rel != BoundRelation::GreaterEqual && bound->value < *lower && !ass.is_false(bound->lit)) {
                    if (!propagate(lower_clause, *bound)) {
                        return false;
                    }
                }
            }
        }
        return true;
    };
    if (options_.propagate_mode == PropagateMode::Changed) {
        while (!propagate_queue_.empty()) {
            auto i = propagate_queue_.front();
            propagate_queue_.pop_front();
            if (!propagate_row(i)) {
                return false;
            }
        }
    } else {
        for (index_t i = 0; i < n_basic_; ++i) {
            if (!propagate_row(i)) {
                return false;
            }
        }
    }
    return true;
}

template <typename Value> void Solver<Value>::undo() {
    try {
        // this function restores the last satisfying assignment
        auto &offset = trail_offset_.back();

        // undo bound updates
        for (auto it = bound_trail_.begin() + offset.bound, ie = bound_trail_.end(); it != ie; ++it) {
            auto [var, rel, bound] = *it;
            switch (rel) {
                case BoundRelation::LessEqual: {
                    variables_[var].upper_bound = bound;
                    break;
                }
                case BoundRelation::GreaterEqual: {
                    variables_[var].lower_bound = bound;
                    break;
                }
                default: {
                    assert(false);
                }
            }
        }
        bound_trail_.resize(offset.bound);

        // undo assignments
        for (auto it = assignment_trail_.begin() + offset.assignment, ie = assignment_trail_.end(); it != ie; ++it) {
            auto &[level, index, number] = *it;
            variables_[index].level = level;
            variables_[index].value.swap(number);
        }
        assignment_trail_.resize(offset.assignment);

        // empty queue
        for (auto ii : conflicts_) {
            variables_[ii].queued = false;
        }
        conflicts_.clear();

        trail_offset_.pop_back();

        if (!cuts_.fresh.empty()) {
            refresh_cuts_();
        }

        assert_extra(check_solution_(false));
    } catch (...) {
        std::terminate();
    }
}

template <typename Value> auto Solver<Value>::statistics() const -> Statistics const & { return statistics_; }

template <typename Value> auto Solver<Value>::check_tableau_() -> bool {
    for (index_t i{0}; i < n_basic_; ++i) {
        Value v_i;
        tableau_.update_row(i, [&](index_t j, Integer const &a_ij, const Integer &d_i) -> auto {
            v_i += mul_div(non_basic_(j).value, a_ij, d_i);
        });
        if (v_i != basic_(i).value) {
            return false;
        }
    }
    return true;
}

template <typename Value> auto Solver<Value>::check_basic_() -> bool {
    for (index_t i = 0; i < n_basic_; ++i) {
        auto &xi = basic_(i);
        if (xi.has_lower() && xi.value < xi.lower() && !xi.queued) {
            return false;
        }
        if (xi.has_upper() && xi.value > xi.upper() && !xi.queued) {
            return false;
        }
    }
    return true;
}

template <typename Value> auto Solver<Value>::check_non_basic_() -> bool {
    for (index_t j = 0; j < n_non_basic_; ++j) {
        auto &xj = non_basic_(j);
        if (xj.has_lower() && xj.value < xj.lower()) {
            return false;
        }
        if (xj.has_upper() && xj.value > xj.upper()) {
            return false;
        }
    }
    return true;
}

template <typename Value> auto Solver<Value>::check_solution_(bool cuts) -> bool {
    auto n = cuts ? variables_.size() : cuts_.offset;
    for (size_t i = 0; i < n; ++i) {
        auto &x = variables_[i];
        if (x.has_lower() && x.lower() > x.value) {
            return false;
        }
        if (x.has_upper() && x.value > x.upper()) {
            return false;
        }
    }
    return check_tableau_() && check_basic_() && check_non_basic_();
}

template <typename Value> void Solver<Value>::update_(index_t level, index_t j, Value v) {
    auto &xj = non_basic_(j);
    auto delta = v - xj.value;
    tableau_.update_col(j, [&](index_t i, Integer const &a_ij, const Integer &d_i) -> auto {
        basic_(i).set_value(*this, level, mul_div(delta, a_ij, d_i), true);
        enqueue_(i);
    });
    xj.set_value(*this, level, std::move(v), false);
}

template <typename Value> void Solver<Value>::pivot_(index_t level, index_t i, index_t j, Value const &v) {
    Integer *a_ij = nullptr;
    Integer *d_i = nullptr;
    tableau_.unsafe_get(i, j, a_ij, d_i);
    assert(*a_ij != 0);

    auto &xi = basic_(i);
    auto &xj = non_basic_(j);

    // adjust assignment
    Value v_j = mul_div(v - xi.value, *d_i, *a_ij);

    xi.set_value(*this, level, v, false);
    xj.set_value(*this, level, v_j, true);
    tableau_.update_col(j, [&](index_t k, Integer const &a_kj, Integer const &d_k) -> auto {
        if (k != i) {
            basic_(k).set_value(*this, level, mul_div(v_j, a_kj, d_k), true);
            enqueue_(k);
        }
    });
    assert_extra(check_tableau_());

    // swap variables x_i and x_j
    std::swap(xi.reverse_index, xj.reverse_index);
    std::swap(variables_[i + n_non_basic_].index, variables_[j].index);
    enqueue_(i);

    // eliminate x_j from rows k != i
    tableau_.pivot(i, j, *a_ij, *d_i);

    ++statistics_.pivots;
    assert_extra(check_tableau_());
    assert_extra(check_basic_());
    assert_extra(check_non_basic_());
}

template <typename Value> auto Solver<Value>::bland_threshold_() const -> size_t {
    return std::max<size_t>(options_.bland_threshold, n_basic_);
}

template <typename Value> auto Solver<Value>::select_basic_(BasicRule rule) -> std::optional<index_t> {
    // The queue might contain variables that meanwhile became non-basic or are
    // no longer conflicting. The selected variable stays in the queue until it
    // left the basis.
    auto conflicting = [this](index_t ii) -> bool {
        auto &xi = variables_[ii];
        return xi.reverse_index >= n_non_basic_ && xi.has_conflict();
    };
    if (rule == BasicRule::Bland) {
        for (; !conflicts_.empty(); std::ranges::pop_heap(conflicts_, std::greater<>{}), conflicts_.pop_back()) {
            auto ii = conflicts_.front();
            variables_[ii].queued = false;
            if (conflicting(ii)) {
                return ii;
            }
        }
        return std::nullopt;
    }
    auto sel = Selector<Value, std::greater<>>{};
    std::erase_if(conflicts_, [&](index_t ii) -> bool {
        auto &xi = variables_[ii];
        if (!conflicting(ii)) {
            xi.queued = false;
            return true;
        }
        sel.offer(ii, xi.has_lower() && xi.value < xi.lower() ? xi.lower() - xi.value : xi.value - xi.upper());
        return false;
    });
    std::ranges::make_heap(conflicts_, std::greater<>{});
    if (sel.empty()) {
        return std::nullopt;
    }
    variables_[sel.index()].queued = false;
    return sel.index();
}

template <typename Value>
auto Solver<Value>::select_(index_t &ret_i, index_t &ret_j, Value const *&ret_v, bool bland) -> Solver<Value>::State {
    // First, a conflicting basic variable x_i is selected. Then, a non-basic
    // variable x_j is selected that can be changed to move x_i towards its
    // violated bound. The configured rules can cycle. If bland is set, the
    // variables with the smallest indices are selected instead (Bland's
    // rule), which guarantees termination.
    auto ii = select_basic_(bland ? BasicRule::Bland : options_.basic_rule);
    if (!ii.has_value()) {
        assert_extra(check_solution_());
        return State::Satisfiable;
    }
    auto &xi = variables_[*ii];
    assert(*ii == variables_[xi.reverse_index].index);
    bool lower = xi.has_lower() && xi.value < xi.lower();
    bool sparse = !bland && options_.non_basic_rule == NonBasicRule::Sparse;

    conflict_clause_.clear();
    conflict_clause_.emplace_back(lower ? -xi.lower_bound->lit : -xi.upper_bound->lit);
    auto sel = Selector<size_t>{};
    ret_i = xi.reverse_index - n_non_basic_;
    tableau_.update_row(ret_i, [&](index_t j, Integer const &a_ij, Integer const &d_i) -> void {
        auto &x_j = variables_[variables_[j].index];
        bool upper = lower == ((a_ij > 0) == (d_i > 0));
        // preemptively add bound to conflict clause if it can be increased no further
        if (upper ? x_j.has_upper() && x_j.value >= x_j.upper() : x_j.has_lower() && x_j.value <= x_j.lower()) {
            conflict_clause_.emplace_back(upper ? -x_j.upper_bound->lit : -x_j.lower_bound->lit);
        }
        // we can set x_i to one of its bounds to get rid of the conflict
        else if (sel.offer(variables_[j].index, sparse ? tableau_.col_size(j) : 0)) {
            ret_j = j;
        }
    });
    // the constraint determining x_i is tight - the conflict clause captures the reason
    if (sel.empty()) {
        return State::Unsatisfiable;
    }
    ret_v = lower ? &xi.lower() : &xi.upper();
    return State::Unknown;
}

template <typename Value> auto Solver<Value>::adjust(Clingo::SolverLiteral lit) const -> Clingo::SolverLiteral {
    if (options_.select == SelectionHeuristic::None) {
        return lit;
    }
    // Note that the tests could be strengthend further, we could check if
    // setting the value to its bound would cause a conflict or match.
    for (auto it = bounds_.find(lit), ie = bounds_.end(); it != ie && it->first == lit; ++it) {
        Bound const &bound = it->second;
        Value const &value = variables_[bound.variable].value;
        if (bound.compare(value) == (options_.select == SelectionHeuristic::Conflict)) {
            return -lit;
        }
    }
    for (auto it = bounds_.find(-lit), ie = bounds_.end(); it != ie && it->first == -lit; ++it) {
        Bound const &bound = it->second;
        Value const &value = variables_[bound.variable].value;
        if (bound.compare(value) == (options_.select == SelectionHeuristic::Match)) {
            return -lit;
        }
    }
    return lit;
}

template <typename Value> void Propagator<Value>::do_init(Clingo::Assignment ass, Clingo::PropagateInit init) {
    facts_offset_ = facts_.size();
    if (facts_offset_ > 0 || options_.global_objective.has_value()) {
        init.check_mode(Clingo::PropagatorCheckMode::both);
    }

    evaluate_theory(
        *lib_, init.base().theory(),
        [&](std::optional<Clingo::SolverLiteral> lit) -> auto { return init.solver_literal(lit); }, aux_map_, iqs_,
        objective_, integers_);
    n_vars_ = ass.size();

    auto add_var = [this](Clingo::Symbol const &var) -> void {
        if (var_map_.emplace(var, static_cast<index_t>(var_map_.size())).second) {
            var_vec_.emplace_back(var);
        }
    };
    for (auto const &term : objective_) {
        add_var(term.var);
    }
    for (auto const &var : integers_) {
        add_var(var);
    }
    for (auto &x : iqs_) {
        for (auto const &term : x.lhs) {
            add_var(term.var);
        }
    }

    auto integers = collect_integers_();
    auto is_integer = [&](Clingo::Symbol const &var) -> bool {
        return std::ranges::binary_search(integers, var_map_.at(var));
    };
    for (auto &x : iqs_) {
        if constexpr (std::is_same_v<Value, Rational>) {
            // strict inequalities over integer variables are turned into non-strict ones
            if ((x.rel == Relation::Less || x.rel == Relation::Greater) &&
                (x.lhs.empty() ||
                 !std::ranges::all_of(x.lhs, [&](Term const &term) -> bool { return is_integer(term.var); }))) {
                throw std::invalid_argument{"strict inequalities not enabled (try option --strict)"};
            }
        }
        init.add_watch(x.lit);
    }

    objective_state_.reset();

    slvs_.clear();
    slvs_.reserve(init.number_of_threads());
    for (size_t i = 0, e = init.number_of_threads(); i != e; ++i) {
        slvs_.emplace_back(std::piecewise_construct, std::forward_as_tuple(0), std::forward_as_tuple(options_));
        if (!slvs_.back().second.prepare(ass, init, var_map_, iqs_, objective_, integers, i == 0)) {
            return;
        }
    }
}

template <typename Value> auto Propagator<Value>::collect_integers_() const -> std::vector<index_t> {
    // Collect the indices of integer variables. Auxiliary variables for
    // conditional terms are integer if the term is integer.
    std::vector<index_t> integers;
    integers.reserve(integers_.size());
    for (auto const &var : integers_) {
        integers.emplace_back(var_map_.at(var));
    }
    auto sort_unique = [&integers]() -> void {
        std::ranges::sort(integers);
        integers.erase(std::ranges::unique(integers).begin(), integers.end());
    };
    sort_unique();
    if (integers.empty()) {
        return integers;
    }
    auto n = integers.size();
    for (auto const &[key, aux] : aux_map_) {
        auto it = var_map_.find(key.first);
        auto jt = var_map_.find(aux);
        if (jt != var_map_.end() &&
            (is_constant(key.first) ||
             (it != var_map_.end() &&
              std::ranges::binary_search(integers.begin(), integers.begin() + static_cast<std::ptrdiff_t>(n),
                                         it->second)))) {
            integers.emplace_back(jt->second);
        }
    }
    if (integers.size() != n) {
        sort_unique();
    }
    return integers;
}

template <typename Value> void Propagator<Value>::on_statistics(Clingo::Stats step, Clingo::Stats accu) {
    auto step_simplex = step.map().insert("Simplex", Clingo::StatsType::map);
    auto accu_simplex = accu.map().insert("Simplex", Clingo::StatsType::map);
    auto add = [&](char const *name, size_t Statistics::*member) -> void {
        double value = 0;
        for (auto const &[offset, slv] : slvs_) {
            value += static_cast<double>(slv.statistics().*member);
        }
        step_simplex.map().insert(name, Clingo::StatsType::value).value(value);
        auto accu_value = accu_simplex.map().insert(name, Clingo::StatsType::value);
        accu_value.value(accu_value.value() + value);
    };
    add("Pivots", &Statistics::pivots);
    add("Bounds propagated", &Statistics::propagated_bounds);
    if (!integers_.empty()) {
        add("Branches", &Statistics::branches);
        add("Cuts", &Statistics::cuts);
        add("GCD conflicts", &Statistics::gcd_conflicts);
        add("Fixed bounds", &Statistics::fixed_bounds);
    }
}

template <typename Value>
auto Propagator<Value>::do_decide(Clingo::Assignment ass, Clingo::SolverLiteral fallback) -> Clingo::SolverLiteral {
    // Literals introduced during solving keep their sign. The sign of order
    // literals is chosen when branching (see Solver::branch_), and the values
    // of integer variables are fractional, so they satisfy neither side of a
    // branch.
    auto const &slv = slvs_[ass.thread_id()].second;
    if (std::cmp_greater(std::abs(fallback), n_vars_)) {
        return slv.adjust_branch(fallback);
    }
    return slv.adjust(fallback);
}

template <typename Value> void Propagator<Value>::on_model(Clingo::Model const &model) {
    if (!options_.global_objective.has_value()) {
        return;
    }
    auto &slv = slvs_[model.thread_id()].second;
    auto objective = slv.get_objective();
    if (!objective.has_value()) {
        return;
    }
    objective_state_.update(*std::move(objective));
}

template <typename Value> void Propagator<Value>::do_check(Clingo::Assignment ass, Clingo::PropagateControl ctl) {
    auto &[offset, slv] = slvs_[ass.thread_id()];
    if (ass.decision_level() == 0 && offset < facts_offset_) {
        auto res = slv.solve(ass, ctl, Clingo::SolverLiteralSpan{facts_.data() + offset, facts_offset_}); // NOLINT
        offset = facts_offset_;
        // can happen in case of a top-level conflict
        if (!res) {
            return;
        }
    }
    // Integrate the current objective value into the solver (if it is better).
    if (!slv.integrate_objective(ctl, objective_state_)) {
        return;
    }
    if (ass.is_total()) {
        // Compute an optimal assignment.
        slv.optimize();
        // Ensure that integer variables have integer values.
        if (!slv.check_integers(ass, ctl)) {
            return;
        }
        // Store the current assignment in the hope that the next model can be
        // obtained from it with a small number of pivots.
        if (options_.store_sat_assignment >= StoreSATAssignments::Partial) {
            slv.store_sat_assignment();
        }
        // Discard bounded assignments if the objective value is unbounded.
        if (!slv.discard_bounded(ctl)) {
            return;
        }
    }
}

template <typename Value>
void Propagator<Value>::do_propagate(Clingo::Assignment ass, Clingo::PropagateControl ctl,
                                     Clingo::SolverLiteralSpan changes) {
    // Facts are collected by the first solver before the other solvers
    // start. Note that checking the root level instead of decision level 0
    // would also capture facts when solving under assumptions.
    if (ass.decision_level() == 0 && ass.thread_id() == 0) {
        // Literals introduced during solving are solver local and must not be shared.
        for (auto lit : changes) {
            if (std::cmp_less_equal(std::abs(lit), n_vars_)) {
                facts_.emplace_back(lit);
            }
        }
    }
    auto &[offset, slv] = slvs_[ass.thread_id()];
    static_cast<void>(slv.solve(ass, ctl, changes));
}

template <typename Value>
void Propagator<Value>::do_undo(Clingo::Assignment ass, [[maybe_unused]] Clingo::SolverLiteralSpan changes) {
    slvs_[ass.thread_id()].second.undo();
}

template <typename Value>
auto Propagator<Value>::lookup_symbol(const Clingo::Symbol &symbol) const -> std::optional<index_t> {
    if (auto it = var_map_.find(symbol); it != var_map_.end()) {
        return it->second;
    }
    return {};
}

template <typename Value> auto Propagator<Value>::get_symbol(index_t i) const -> Clingo::Symbol { return var_vec_[i]; }

template <typename Value> auto Propagator<Value>::has_value(index_t thread_id, index_t i) const -> bool {
    static_cast<void>(thread_id);
    return i < var_vec_.size();
}

template <typename Value> auto Propagator<Value>::get_value(index_t thread_id, index_t i) const -> Value {
    return slvs_[thread_id].second.get_value(i);
}

template <typename Value>
auto Propagator<Value>::get_objective(index_t thread_id) const -> std::optional<std::pair<Value, bool>> {
    return slvs_[thread_id].second.get_objective();
}

template <typename Value> auto Propagator<Value>::n_values(index_t thread_id) const -> index_t {
    static_cast<void>(thread_id);
    return static_cast<index_t>(var_vec_.size());
}

template class Solver<Rational>;
template class Solver<RationalQ>;
template class Propagator<Rational>;
template class Propagator<RationalQ>;

} // namespace ClingoLPX
