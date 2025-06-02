#include "solving.hh"
#include "parsing.hh"

#include <clingo.hh>

#include <climits>
#include <cmath>
#include <cstddef>
#include <exception>
#include <functional>
#include <memory>
#include <optional>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <tuple>
#include <unordered_set>
#include <utility>

namespace {

auto as_value(RationalQ const &a, RationalQ *b) -> RationalQ {
    static_cast<void>(b);
    return a;
}

auto as_value(RationalQ const &a, Rational *b) -> Rational {
    static_cast<void>(b);
    return a.as_rational();
}

} // namespace

template <typename Value> void ObjectiveState<Value>::reset() {
    value_ = Value{};
    generation_ = 0;
    bounded_ = true;
}

template <typename Value> void ObjectiveState<Value>::update(std::pair<Value, bool> value) {
#ifndef CLINGOLPX_NO_SHARED_MUTEX
    std::unique_lock<std::shared_mutex> lock{mutex_};
#else
    std::unique_lock<std::mutex> lock{mutex_};
#endif
    if (bounded_ && (!value.second || generation_ == 0 || value.first > value_)) {
        ++generation_;
        value_ = std::move(value.first);
        bounded_ = value.second;
    }
}

template <typename Value>
auto ObjectiveState<Value>::value(size_t &generation) -> std::optional<std::pair<Value, bool>> {
#ifndef CLINGOLPX_NO_SHARED_MUTEX
    std::shared_lock<std::shared_mutex> lock{mutex_};
#else
    std::unique_lock<std::mutex> lock{mutex_};
#endif
    if (generation != generation_) {
        generation = generation_;
        return std::make_pair(value_, bounded_);
    }
    return std::nullopt;
}

template <typename Value> auto bound_rel(Relation rel) -> typename Solver<Value>::BoundRelation {
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
        slv.n_non_basic_ = map.size();
        for (index_t i = 0; i != slv.n_non_basic_; ++i) {
            slv.variables_[i].index = i;
            slv.variables_[i].reverse_index = i;
        }
    }

    auto get_non_basic(Clingo::Symbol var) -> index_t {
        auto jt = map.find(var);
        assert(jt != map.end());
        return slv.variables_[jt->second].reverse_index;
    }

    auto add_basic() -> index_t {
        auto index = slv.variables_.size();
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
            if (upper_bound != &bound) {
                s.bound_trail_.emplace_back(bound.variable, BoundRelation::GreaterEqual, lower_bound);
            } else {
                // Note: this assumes that update_lower is called right after update_upper for the same bound
                std::get<1>(s.bound_trail_.back()) = BoundRelation::Equal;
            }
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
        s.assignment_trail_.emplace_back(level, this - s.variables_.data(), value);
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

template <typename Value> auto Solver<Value>::basic_(index_t i) -> typename Solver<Value>::Variable & {
    assert(i < n_basic_);
    return variables_[variables_[i + n_non_basic_].index];
}

template <typename Value> auto Solver<Value>::non_basic_(index_t j) -> typename Solver<Value>::Variable & {
    assert(j < n_non_basic_);
    return variables_[variables_[j].index];
}

template <typename Value> void Solver<Value>::enqueue_(index_t i) {
    assert(i < n_basic_);
    auto ii = variables_[i + n_non_basic_].index;
    auto &xi = variables_[ii];
    if (!xi.queued && xi.has_conflict()) {
        conflicts_.emplace(ii);
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
auto Solver<Value>::prepare(Clingo::PropagateInit &init, SymbolMap const &symbols,
                            std::vector<Inequality> const &inequalities, std::vector<Term> const &objective,
                            bool master) -> bool {
    auto ass = init.assignment();

    Prepare prep{*this, symbols};
    for (auto const &x : inequalities) {
        if (ass.is_false(x.lit)) {
            continue;
        }

        // transform inequality into row suitable for tableau
        auto row = prep.add_row(x.lhs);

        // check bound against 0
        if (row.empty()) {
            if (!master) {
                continue;
            }
            switch (x.rel) {
                case Relation::Less: {
                    if (x.rhs >= 0 && !init.add_clause({-x.lit})) {
                        return false;
                    }
                    break;
                }
                case Relation::LessEqual: {
                    if (x.rhs > 0 && !init.add_clause({-x.lit})) {
                        return false;
                    }
                    break;
                }
                case Relation::Greater: {
                    if (x.rhs <= 0 && !init.add_clause({-x.lit})) {
                        return false;
                    }
                    break;
                }
                case Relation::GreaterEqual: {
                    if (x.rhs < 0 && !init.add_clause({-x.lit})) {
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
            auto rel = v < 0 ? invert(x.rel) : x.rel;
            bounds_.emplace(x.lit,
                            Bound{bound_val<Value>(x.rhs / v, rel), variables_[j].index, x.lit, bound_rel<Value>(rel)});
        }
        // add an inequality
        else {
            auto i = prep.add_basic();
            bounds_.emplace(x.lit, Bound{bound_val<Value>(x.rhs, x.rel), static_cast<index_t>(variables_.size() - 1),
                                         x.lit, bound_rel<Value>(x.rel)});
            for (auto const &[j, v] : row) {
                tableau_.set(i, j, v);
            }
        }
    }

    // add objective function to tableau
    if (!objective.empty()) {
        objective_.active = true;
        auto row = prep.add_row(objective);
        auto add_row = [&, this]() {
            auto i = prep.add_basic();
            for (auto const &[j, v] : row) {
                tableau_.set(i, j, v);
            }
            return variables_.size() - 1;
        };
        if (options_.global_objective.has_value()) {
            objective_.bound_var = add_row();
        }
        objective_.var = add_row();
    }

    for (size_t i = 0; i < n_basic_; ++i) {
        enqueue_(i);
    }

    for (auto const &[lit, bound] : bounds_) {
        variables_[bound.variable].bounds.emplace_back(&bound);
    }
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
    std::cerr << "tableau:" << std::endl;
    tableau_.debug("  ");
    if (objective_) {
        auto z = variables_[objective_.var].reverse_index - n_non_basic_;
        std::cerr << "objective variable:\n  y_" << z << std::endl;
    }

    std::cerr << "basic assignment:" << std::endl;
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
        std::cerr << std::endl;
    }
    std::cerr << "non-basic assignment:" << std::endl;
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
        std::cerr << std::endl;
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
    //      violate x_e's lower bound (or make it tight).
    //        The row is unbounded.
    //      Case x_l can be set to its lower bound.
    //        We obtain an increase of x_e smaller than its upper bound.
    //
    //   We now consider which candidate to chose as a leaving variable.
    //   Case all rows for leaving variables candidates are unbounded.
    //     We can set x_e to its upper bound and adjust the corresponding
    //     values of the x_l without causing conflicts. At this point x_e is no
    //     entering variable anymore.
    //   Case there is an x_l that can be set to its lower bound to make a
    //   constraint tight.
    //     We select the leaving variable that causes the least change to x_e
    //     to guarantee that no bounds are violated. This is the standard
    //     pivoting case. Unlike in algorithm to find a basic feasible
    //     solution, this pivot cannot cause a conflict.
    // Case a_ze > 0:
    //   symmetric
    if (!objective_) {
        return;
    }

    assert_extra(check_solution_());
    while (true) {
        // the objective assigned to variable y_z
        auto z = variables_[objective_.var].reverse_index - n_non_basic_;

        // select entering variable x_e
        index_t ee = variables_.size();
        bool pos_a_ze = false;
        tableau_.update_row(z, [&, this](int j, Integer const &a_zj, Integer const &d_z) {
            auto jj = variables_[j].index;
            if (jj < ee) {
                auto &x_j = variables_[jj];
                bool pos_a_zj = (a_zj > 0) == (d_z > 0);
                if (pos_a_zj ? !x_j.has_upper() || x_j.value < x_j.upper()
                             : !x_j.has_lower() || x_j.value > x_j.lower()) {
                    ee = jj;
                    pos_a_ze = pos_a_zj;
                }
            }
        });

        // the solution is optimal if there is no exiting variable
        if (ee == variables_.size()) {
            assert_extra(check_solution_());
            objective_.bounded = true;
            return;
        }

        // select leaving variable y_l
        auto &x_e = variables_[ee];
        Value v_e;
        auto e = x_e.reverse_index;
        assert(ee == variables_[e].index);
        index_t ll = variables_.size();
        Value const *bound_l = nullptr;

        tableau_.update_col(e, [&, this](index_t i, Integer const &a_ie, Integer const &d_i) {
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
            Value v = x_e.value + (v_i - y_i.value) / a_ie * d_i;
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
            auto l = variables_[ll].reverse_index - n_non_basic_;
            pivot_(level, l, e, *bound_l);
        } else {
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
}

template <typename Value> auto Solver<Value>::update_bound_(Clingo::PropagateControl &ctl, Bound const &bound) -> bool {
    auto ass = ctl.assignment();
    auto &x = variables_[bound.variable];
    if (!x.update(*this, ass, bound)) {
        conflict_clause_.clear();
        conflict_clause_.emplace_back(-x.upper_bound->lit);
        conflict_clause_.emplace_back(-x.lower_bound->lit);
        ctl.add_clause(conflict_clause_);
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

template <typename Value> auto Solver<Value>::assert_bound_(Clingo::PropagateControl &ctl, Value value) -> bool {
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
auto Solver<Value>::integrate_objective(Clingo::PropagateControl &ctl, ObjectiveState<Value> &state) -> bool {
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

template <typename Value> auto Solver<Value>::discard_bounded(Clingo::PropagateControl &ctl) -> bool {
    // Here we discard bounded solutions by asserting that the objective
    // is greater than the current optimal objective.
    if (!objective_ || !options_.global_objective.has_value() || !objective_.bounded || !objective_.discard_bounded) {
        return true;
    }
    return assert_bound_(ctl, variables_[objective_.var].value + 1);
}

template <typename Value> auto Solver<Value>::solve(Clingo::PropagateControl &ctl, Clingo::LiteralSpan lits) -> bool {
    index_t i{0};
    index_t j{0};
    Value const *v{nullptr};

    auto ass = ctl.assignment();
    auto level = ass.decision_level();

    if (trail_offset_.empty() || trail_offset_.back().level < level) {
        trail_offset_.emplace_back(TrailOffset{level, static_cast<index_t>(bound_trail_.size()),
                                               static_cast<index_t>(assignment_trail_.size())});
    }

    for (auto lit : lits) {
        for (auto it = bounds_.find(lit), ie = bounds_.end(); it != ie && it->first == lit; ++it) {
            auto const &[lit_a, bound_a] = *it;
            assert(lit == lit_a);
            if (!update_bound_(ctl, bound_a)) {
                return false;
            }
        }
    }

    assert_extra(check_tableau_());
    assert_extra(check_basic_());
    assert_extra(check_non_basic_());

    while (true) {
        switch (select_(i, j, v)) {
            case State::Satisfiable: {
                if (options_.store_sat_assignment == StoreSATAssignments::Partial) {
                    store_sat_assignment();
                }
                return propagate_(ctl);
            }
            case State::Unsatisfiable: {
                ctl.add_clause(conflict_clause_);
                return false;
            }
            case State::Unknown: {
                assert(v != nullptr);
                pivot_(level, i, j, *v); // NOLINT
            }
        }
    }
}

template <typename Value> auto Solver<Value>::propagate_(Clingo::PropagateControl &ctl) -> bool {
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
    auto ass = ctl.assignment();
    std::vector<Clingo::literal_t> lower_clause;
    std::vector<Clingo::literal_t> upper_clause;
    auto propagate_row = [&](index_t i) {
        variables_[i].propagate = false;
        std::optional<Value> lower = Value{0};
        std::optional<Value> upper = Value{0};
        // check if a constraint provides a bound
        tableau_.update_row(i, [&, this](index_t j, Integer const &a_ij, Integer const &d_i) {
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
        // compute the bound
        tableau_.update_row(i, [&, this](index_t j, Integer const &a_ij, Integer const &d_i) {
            auto &x_j = non_basic_(j);
            auto update_lower = [&](std::vector<Clingo::literal_t> &clause, std::optional<Value> &bound) {
                if (bound.has_value() && x_j.has_lower()) {
                    *bound += x_j.lower() * a_ij / d_i;
                    clause.emplace_back(-x_j.lower_bound->lit);
                }
            };
            auto update_upper = [&](std::vector<Clingo::literal_t> &clause, std::optional<Value> &bound) {
                if (bound.has_value() && x_j.has_upper()) {
                    *bound += x_j.upper() * a_ij / d_i;
                    clause.emplace_back(-x_j.upper_bound->lit);
                }
            };
            if ((a_ij > 0) == (d_i > 0)) {
                update_lower(lower_clause, lower);
                update_upper(upper_clause, upper);
            } else {
                update_upper(lower_clause, lower);
                update_lower(upper_clause, upper);
            }
        });
        auto &y_i = basic_(i);
        auto propagate = [&, this](std::vector<Clingo::literal_t> &clause, Bound const &bound) {
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
                case BoundRelation::Equal: {
                    variables_[var].upper_bound = bound;
                    variables_[var].lower_bound = bound;
                    break;
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
        for (; !conflicts_.empty(); conflicts_.pop()) {
            variables_[conflicts_.top()].queued = false;
        }

        trail_offset_.pop_back();

        assert_extra(check_solution_());
    } catch (...) {
        std::terminate();
    }
}

template <typename Value> auto Solver<Value>::statistics() const -> Statistics const & { return statistics_; }

template <typename Value> auto Solver<Value>::check_tableau_() -> bool {
    for (index_t i{0}; i < n_basic_; ++i) {
        Value v_i;
        tableau_.update_row(
            i, [&](index_t j, Integer const &a_ij, Integer d_i) { v_i += non_basic_(j).value * a_ij / d_i; });
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

template <typename Value> auto Solver<Value>::check_solution_() -> bool {
    for (auto &x : variables_) {
        if (x.has_lower() && x.lower() > x.value) {
            return false;
        }
        if (x.has_upper() && x.value > x.upper()) {
            return false;
        }
    }
    return check_tableau_() && check_basic_();
}

template <typename Value> void Solver<Value>::update_(index_t level, index_t j, Value v) {
    auto &xj = non_basic_(j);
    tableau_.update_col(j, [&](index_t i, Integer const &a_ij, Integer d_i) {
        basic_(i).set_value(*this, level, (v - xj.value) * a_ij / d_i, true);
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
    Value v_j = (v - xi.value) / *a_ij * *d_i;

    xi.set_value(*this, level, v, false);
    xj.set_value(*this, level, v_j, true);
    tableau_.update_col(j, [&](index_t k, Integer const &a_kj, Integer const &d_k) {
        if (k != i) {
            basic_(k).set_value(*this, level, v_j * a_kj / d_k, true);
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

template <typename Value>
auto Solver<Value>::select_(index_t &ret_i, index_t &ret_j, Value const *&ret_v) -> typename Solver<Value>::State {
    // This implements Bland's rule selecting the variables with the smallest
    // indices for pivoting.

    for (; !conflicts_.empty(); conflicts_.pop()) {
        auto ii = conflicts_.top();
        auto &xi = variables_[ii];
        auto i = xi.reverse_index;
        assert(ii == variables_[i].index);
        xi.queued = false;
        // the queue might contain variables that meanwhile became basic
        if (i < n_non_basic_) {
            continue;
        }
        i -= n_non_basic_;

        bool lower = xi.has_lower() && xi.value < xi.lower();
        if (lower || (xi.has_upper() && xi.value > xi.upper())) {
            conflict_clause_.clear();
            conflict_clause_.emplace_back(lower ? -xi.lower_bound->lit : -xi.upper_bound->lit);
            index_t kk = variables_.size();
            tableau_.update_row(i, [&](index_t j, Integer const &a_ij, Integer const &d_i) {
                auto jj = variables_[j].index;
                // skip over the variable if we already have a better one
                // according to blands rule
                if (jj > kk) {
                    return;
                }
                auto &x_j = variables_[jj];
                bool upper = lower == ((a_ij > 0) == (d_i > 0));
                // preemptively add bound to conflict clause if it can be increased no further
                if (upper ? x_j.has_upper() && x_j.value >= x_j.upper() : x_j.has_lower() && x_j.value <= x_j.lower()) {
                    conflict_clause_.emplace_back(upper ? -x_j.upper_bound->lit : -x_j.lower_bound->lit);
                }
                // we can set x_i to one of its bounds to get rid of the conflict
                else {
                    kk = jj;
                    ret_i = i;
                    ret_j = j;
                    ret_v = lower ? &xi.lower() : &xi.upper();
                }
            });
            // the constraint determining x_i is tight - the conflict clause captures the reason
            if (kk == variables_.size()) {
                return State::Unsatisfiable;
            }
            return State::Unknown;
        }
    }

    assert_extra(check_solution_());

    return State::Satisfiable;
}

template <typename Value>
auto Solver<Value>::adjust(Clingo::Assignment const &assign, Clingo::literal_t lit) const -> Clingo::literal_t {
    static_cast<void>(assign);
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

template <typename Value> void Propagator<Value>::init(Clingo::PropagateInit &init) {
    facts_offset_ = facts_.size();
    if (facts_offset_ > 0 || options_.global_objective.has_value()) {
        init.set_check_mode(Clingo::PropagatorCheckMode::Both);
    }

    evaluate_theory(
        init.theory_atoms(), [&](Clingo::literal_t lit) { return init.solver_literal(lit); }, aux_map_, iqs_,
        objective_);

    auto gather_vars = [this](std::vector<Term> const &terms) {
        for (auto const &term : terms) {
            if (var_map_.emplace(term.var, var_map_.size()).second) {
                var_vec_.emplace_back(term.var);
            }
        }
    };
    gather_vars(objective_);
    for (auto &x : iqs_) {
        gather_vars(x.lhs);
        init.add_watch(x.lit);
    }

    objective_state_.reset();

    slvs_.clear();
    slvs_.reserve(init.number_of_threads());
    for (size_t i = 0, e = init.number_of_threads(); i != e; ++i) {
        slvs_.emplace_back(std::piecewise_construct, std::forward_as_tuple(0), std::forward_as_tuple(options_));
        if (!slvs_.back().second.prepare(init, var_map_, iqs_, objective_, i == 0)) {
            return;
        }
    }
}

template <typename Value> void Propagator<Value>::register_control(Clingo::Control &ctl) {
    ctl.register_propagator(*this);
    if constexpr (std::is_same_v<Value, RationalQ>) {
        ctl.add("base", {}, THEORY_Q);
    } else {
        ctl.add("base", {}, THEORY);
    }
}

template <typename Value>
void Propagator<Value>::on_statistics(Clingo::UserStatistics step, Clingo::UserStatistics accu) {
    auto step_simplex = step.add_subkey("Simplex", Clingo::StatisticsType::Map);
    auto step_pivots = step_simplex.add_subkey("Pivots", Clingo::StatisticsType::Value);
    auto step_propagated_bounds = step_simplex.add_subkey("Bounds propagated", Clingo::StatisticsType::Value);
    auto accu_simplex = accu.add_subkey("Simplex", Clingo::StatisticsType::Map);
    auto accu_pivots = accu_simplex.add_subkey("Pivots", Clingo::StatisticsType::Value);
    auto accu_propagated_bounds = accu_simplex.add_subkey("Bounds propagated", Clingo::StatisticsType::Value);
    for (auto const &[offset, slv] : slvs_) {
        step_pivots.set_value(slv.statistics().pivots);
        accu_pivots.set_value(accu_pivots.value() + slv.statistics().pivots);
        step_propagated_bounds.set_value(slv.statistics().propagated_bounds);
        accu_propagated_bounds.set_value(accu_propagated_bounds.value() + slv.statistics().propagated_bounds);
    }
}

template <typename Value>
auto Propagator<Value>::decide(Clingo::id_t thread_id, Clingo::Assignment const &assign, Clingo::literal_t fallback)
    -> Clingo::literal_t {
    return slvs_[thread_id].second.adjust(assign, fallback);
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

template <typename Value> void Propagator<Value>::check(Clingo::PropagateControl &ctl) {
    auto ass = ctl.assignment();
    auto &[offset, slv] = slvs_[ctl.thread_id()];
    if (ass.decision_level() == 0 && offset < facts_offset_) {
        auto res = slv.solve(ctl, Clingo::LiteralSpan{facts_.data() + offset, facts_offset_}); // NOLINT
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
void Propagator<Value>::propagate(Clingo::PropagateControl &ctl, Clingo::LiteralSpan changes) {
    auto ass = ctl.assignment();
    if (ass.decision_level() == 0 && ctl.thread_id() == 0) {
        // Note to self: auxiliary variables introduced during solving cannot become facts
        facts_.insert(facts_.end(), changes.begin(), changes.end());
    }
    auto &[offset, slv] = slvs_[ctl.thread_id()];
    static_cast<void>(slv.solve(ctl, changes));
}

template <typename Value>
void Propagator<Value>::undo(Clingo::PropagateControl const &ctl, Clingo::LiteralSpan changes) noexcept {
    static_cast<void>(changes);
    slvs_[ctl.thread_id()].second.undo();
}

template <typename Value> auto Propagator<Value>::lookup_symbol(Clingo::Symbol symbol) const -> std::optional<index_t> {
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
    return var_vec_.size();
}

template class Solver<Rational>;
template class Solver<RationalQ>;
template class Propagator<Rational>;
template class Propagator<RationalQ>;
