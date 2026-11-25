// Support for integer variables: branching, order literals, the GCD test,
// Gomory cuts, and reduced cost fixing.

#include <clingo-lpx/integer.hh>
#include <clingo-lpx/solving.hh>
#include <clingo-lpx/util.hh>

#include <algorithm>
#include <cassert>
#include <optional>
#include <tuple>
#include <utility>

namespace ClingoLPX {

// {{{1 OrderLiterals

void OrderLiterals::initialize(Lits uppers, Lits lowers) {
    uppers_ = std::move(uppers);
    lowers_ = std::move(lowers);
    std::ranges::sort(uppers_);
    std::ranges::sort(lowers_);
    init_ = true;
}

auto OrderLiterals::find(Integer const &k) const -> std::optional<Clingo::SolverLiteral> {
    auto it = std::ranges::lower_bound(branches_, k, {}, &Lits::value_type::first);
    if (it != branches_.end() && it->first == k) {
        return it->second;
    }
    return std::nullopt;
}

void OrderLiterals::insert(Integer const &k, Clingo::SolverLiteral lit, bool link,
                           std::vector<Implication> &implications) {
    auto by_value = &Lits::value_type::first;
    auto it = std::ranges::lower_bound(branches_, k, {}, by_value);
    assert(it == branches_.end() || it->first != k);
    auto pos = it - branches_.begin();
    branches_.emplace(it, k, lit);
    if (!link) {
        return;
    }
    auto const *prev = pos > 0 ? &branches_[pos - 1] : nullptr;
    auto const *next = static_cast<size_t>(pos + 1) < branches_.size() ? &branches_[pos + 1] : nullptr;
    // program literals p with p -> x <= a for a in (k', k]
    auto ub = prev != nullptr ? std::ranges::upper_bound(uppers_, prev->first, {}, by_value) : uppers_.begin();
    for (auto ue = std::ranges::upper_bound(uppers_, k, {}, by_value); ub != ue; ++ub) {
        implications.emplace_back(ub->second, lit);
    }
    // program literals q with q -> x >= b for b in (k, k'']
    auto lb = std::ranges::upper_bound(lowers_, k, {}, by_value);
    auto le = next != nullptr ? std::ranges::upper_bound(lowers_, next->first, {}, by_value) : lowers_.end();
    for (; lb != le; ++lb) {
        implications.emplace_back(lb->second, -lit);
    }
    // neighboring order literals
    if (prev != nullptr) {
        implications.emplace_back(prev->second, lit);
    }
    if (next != nullptr) {
        implications.emplace_back(lit, next->second);
    }
}

// {{{1 Cut helpers

auto gomory_coefficient(Rational const &c, Rational const &f_0, bool integer) -> Rational {
    auto f_1 = Rational{1} - f_0;
    if (integer) {
        auto f = c - as_rational(floor(c));
        return f <= f_0 ? f / f_0 : (Rational{1} - f) / f_1;
    }
    return c >= 0 ? c / f_0 : -c / f_1;
}

void round_up(Rational &g, fixed_int den) {
    if (g.den() > den) {
        g = Rational{ceil(g * den), Integer{den}};
    }
}

auto finish_cut(std::vector<CutTerm> &terms, Rational const &margin, Cut &cut) -> bool {
    // drop terms with small contributions starting with the smallest one
    auto weight = [](CutTerm const &term) -> std::optional<Rational> {
        return term.range.has_value() ? std::optional{term.g * *term.range} : std::nullopt;
    };
    std::ranges::sort(terms, [&](CutTerm const &a, CutTerm const &b) -> bool {
        auto wa = weight(a);
        auto wb = weight(b);
        return wa.has_value() && (!wb.has_value() || *wa < *wb);
    });
    cut.rhs = Rational{1};
    cut.clause.clear();
    cut.row.clear();
    auto it = terms.begin();
    for (auto ie = terms.end(); it != ie; ++it) {
        auto w = weight(*it);
        if (!w.has_value() || cut.rhs - *w < margin) {
            break;
        }
        cut.rhs -= *w;
        cut.clause.emplace_back(-it->lit);
        cut.clause.emplace_back(-it->other);
    }
    // substitute back the remaining terms
    for (auto ie = terms.end(); it != ie; ++it) {
        cut.clause.emplace_back(-it->lit);
        if (it->lower) {
            cut.rhs += it->g * it->v;
        } else {
            cut.rhs -= it->g * it->v;
            it->g.neg();
        }
        if (!is_small(it->g)) {
            return false;
        }
        cut.row.emplace_back(it->col, std::move(it->g));
    }
    if (!is_small(cut.rhs)) {
        return false;
    }
    std::ranges::sort(cut.row, {}, &std::pair<index_t, Rational>::first);
    return true;
}

// {{{1 Solver: entry point

template <typename Value>
auto Solver<Value>::check_integers(Clingo::Assignment ass, Clingo::PropagateControl ctl) -> bool {
    if (!ints_.active) {
        return true;
    }
    return fix_reduced_costs_(ctl) && check_integrality_(ass, ctl);
}

template <typename Value>
auto Solver<Value>::check_integrality_(Clingo::Assignment ass, Clingo::PropagateControl ctl) -> bool {
    // We select the integer variable with a fractional value and the smallest
    // index. Before branching, the GCD test is applied. Every n-th time, we
    // try to cut off the current assignment with a Gomory cut and only branch
    // if no cut can be derived.
    //
    // It suffices to check the structural variables, which are stored first.
    // Integer slack variables are integer combinations of them.
    auto ib = variables_.begin();
    auto ie = ib + n_non_basic_;
    auto it = std::find_if(ib, ie, [](Variable const &x) -> bool { return x.integer && !is_integer(x.value); });
    if (it == ie) {
        return true;
    }
    ++ints_.fractional;
    if (options_.gcd_test && !gcd_test_(ctl)) {
        return false;
    }
    if (options_.cut_period > 0 && ints_.fractional % options_.cut_period == 0 && !cut_(ass, ctl)) {
        return false;
    }
    branch_(ctl, static_cast<index_t>(it - ib));
    return false;
}

// {{{1 Solver: GCD test

template <typename Value> auto Solver<Value>::gcd_test_(Clingo::PropagateControl ctl) -> bool {
    // Consider a row d*y = sum_j a_j x_j where all variables are integer and
    // the coefficients are integers. Moving fixed variables, whose lower
    // and upper bound coincide, to the right-hand side gives
    //
    //   sum_j a_j x_j - d*y = C
    //
    // where the left-hand side only contains variables that are not fixed.
    // If the gcd g of the remaining coefficients does not divide C, there is
    // no integer solution and the bounds of the fixed variables conflict.
    auto fixed = [](Variable const &x) -> bool { return x.has_lower() && x.has_upper() && x.lower() == x.upper(); };
    // add the term a*x to C or g and the reasons for fixed variables
    auto add = [&](Variable const &x, Integer const &a, Integer &g, Integer &c) -> bool {
        if (!x.integer) {
            return false;
        }
        if (!fixed(x)) {
            g = gcd(g, a);
            return true;
        }
        if (!is_integer(x.lower())) {
            return false;
        }
        c.add_mul(a, rational_part(x.lower()).num());
        conflict_clause_.emplace_back(-x.lower_bound->lit);
        if (x.upper_bound->lit != x.lower_bound->lit) {
            conflict_clause_.emplace_back(-x.upper_bound->lit);
        }
        return true;
    };
    for (index_t i = 0; i < n_basic_; ++i) {
        auto &y = basic_(i);
        if (!y.integer) {
            continue;
        }
        bool applicable = true;
        Integer g{0};
        Integer c{0};
        Integer const *d = nullptr;
        conflict_clause_.clear();
        tableau_.update_row(i, [&](index_t j, Integer const &a_ij, Integer const &d_i) -> void {
            d = &d_i;
            // moving a_j x_j to the right-hand side
            applicable = applicable && add(non_basic_(j), -a_ij, g, c);
        });
        // moving -d*y to the right-hand side
        if (!applicable || d == nullptr || !add(y, *d, g, c) || g == 0) {
            continue;
        }
        if (c != floor_div(c, g) * g) {
            ++statistics_.gcd_conflicts;
            std::ignore = ctl.add_clause(conflict_clause_);
            return false;
        }
    }
    return true;
}

// {{{1 Solver: branching and order literals

template <typename Value> void Solver<Value>::branch_(Clingo::PropagateControl ctl, index_t ii) {
    // We branch on x <= k or x >= k + 1 for k = floor(v) and the current
    // value v of variable x using an order literal. The solver assigns new
    // literals false by default, so we choose the sign of the literal such
    // that the configured side is tried first.
    auto const &v = variables_[ii].value;
    auto k = floor(v);
    bool prefer_upper = false;
    switch (options_.branch_polarity) {
        case BranchPolarity::Nearest:
        case BranchPolarity::Farthest: {
            bool nearer_upper = v - Value{as_rational(k)} < Value{Rational{Integer{1}, Integer{2}}};
            prefer_upper = nearer_upper == (options_.branch_polarity == BranchPolarity::Nearest);
            break;
        }
        case BranchPolarity::Up: {
            break;
        }
        case BranchPolarity::Down: {
            prefer_upper = true;
            break;
        }
    }
    ++statistics_.branches;
    std::ignore = order_lit_(ctl, ii, std::move(k), prefer_upper);
}

template <typename Value> auto Solver<Value>::orders_(index_t ii) -> OrderLiterals & {
    if (ints_.orders.size() <= ii) {
        ints_.orders.resize(n_non_basic_);
    }
    auto &order = ints_.orders[ii];
    // collect the integer bounds of the program on variable x_ii
    if (options_.order_branches && !order.initialized()) {
        OrderLiterals::Lits uppers;
        OrderLiterals::Lits lowers;
        for (auto const *bound : variables_[ii].bounds) {
            if (!is_integer(bound->value)) {
                continue;
            }
            auto k = floor(bound->value);
            if (bound->rel != BoundRelation::GreaterEqual) {
                uppers.emplace_back(k, bound->lit);
            }
            if (bound->rel != BoundRelation::LessEqual) {
                lowers.emplace_back(std::move(k), bound->lit);
            }
        }
        order.initialize(std::move(uppers), std::move(lowers));
    }
    return order;
}

template <typename Value>
auto Solver<Value>::order_lit_(Clingo::PropagateControl ctl, index_t ii, Integer k, bool prefer_upper)
    -> std::optional<Clingo::SolverLiteral> {
    // We use literals l with l -> x <= k and -l -> x >= k + 1 for integer
    // variables x. Existing literals are reused. A new literal is created
    // such that its default value false selects x <= k if prefer_upper is
    // set and x >= k + 1 otherwise. It is connected to the existing literals
    // via the implications computed by OrderLiterals::insert.
    //
    // The implications hold independently of the current assignment and
    // might only propagate the new literal.
    auto &order = orders_(ii);
    if (auto lit = order.find(k); lit.has_value()) {
        return lit;
    }
    auto fresh = ctl.add_literal();
    auto lit = prefer_upper ? -fresh : fresh;
    ctl.add_watch(lit);
    ctl.add_watch(-lit);
    auto value = as_rational(k);
    auto &upper = bounds_.emplace(lit, Bound{Value{value}, ii, lit, BoundRelation::LessEqual})->second;
    auto &lower = bounds_.emplace(-lit, Bound{Value{value + 1}, ii, -lit, BoundRelation::GreaterEqual})->second;
    variables_[ii].bounds.emplace_back(&upper);
    variables_[ii].bounds.emplace_back(&lower);
    ints_.implications.clear();
    order.insert(k, lit, options_.order_branches, ints_.implications);
    for (auto const &[a, b] : ints_.implications) {
        if (!ctl.add_clause({-a, b}, Clingo::ClauseFlags::lock)) {
            return std::nullopt;
        }
    }
    return lit;
}

// {{{1 Solver: Gomory cuts

template <typename Value> auto Solver<Value>::cut_(Clingo::Assignment ass, Clingo::PropagateControl ctl) -> bool {
    // We derive a cut from the first row that admits one (see derive_cut_)
    // and add it as a new row (see add_cut_).
    for (index_t i = 0; i < n_basic_; ++i) {
        if (derive_cut_(i)) {
            return add_cut_(ass, ctl);
        }
    }
    return true;
}

template <typename Value> auto Solver<Value>::derive_cut_(index_t i) -> bool {
    // Let y be a basic integer variable with fractional value b and row
    //
    //   y = sum_j a_j x_j.
    //
    // If all non-basic variables x_j are at one of their bounds, we can
    // substitute x_j = l_j + t_j for lower bounds and x_j = u_j - t_j for
    // upper bounds with t_j >= 0 to obtain
    //
    //   y + sum_j c_j t_j = b
    //
    // where c_j = -a_j for lower and c_j = a_j for upper bounds. The Gomory
    // mixed integer cut is sum_j g_j t_j >= 1 with coefficients g_j computed
    // by gomory_coefficient. The cut is implied by the bounds of the
    // non-basic variables and violated by the current assignment.
    //
    // Since all t_j are zero in the current assignment, any weakening of the
    // cut is still violated. Dense cuts with large coefficients make exact
    // arithmetic slow, so we weaken cuts in two ways:
    //
    // - Coefficients g_j with denominators larger than cut_denominator are
    //   rounded up to multiples of 1/cut_denominator.
    // - Terms of variables bounded on both sides are dropped (see
    //   finish_cut).
    //
    // Cuts whose coefficients do not fit into machine integers are discarded.
    static constexpr fixed_int cut_denominator = 1024;
    static auto const cut_margin = Rational{Integer{1}, Integer{2}};
    auto &y = basic_(i);
    if (!y.integer || is_integer(y.value) || !is_rational(y.value)) {
        return false;
    }
    auto const &b = rational_part(y.value);
    auto f_0 = b - as_rational(floor(b));
    bool applicable = true;
    auto &terms = cuts_.terms;
    terms.clear();
    tableau_.update_row(i, [&](index_t j, Integer const &a_ij, Integer const &d_i) -> void {
        if (!applicable) {
            return;
        }
        auto &x_j = non_basic_(j);
        bool lower = x_j.has_lower() && x_j.value == x_j.lower();
        bool upper = !lower && x_j.has_upper() && x_j.value == x_j.upper();
        if ((!lower && !upper) || !is_rational(x_j.value)) {
            applicable = false;
            return;
        }
        auto const &bound = *(lower ? x_j.lower_bound : x_j.upper_bound);
        auto const &v_j = rational_part(bound.value);
        auto c_j = Rational{a_ij, d_i};
        if (lower) {
            c_j.neg();
        }
        auto g_j = gomory_coefficient(c_j, f_0, x_j.integer && is_integer(v_j));
        if (g_j == 0) {
            return;
        }
        round_up(g_j, cut_denominator);
        auto const *other = lower ? x_j.upper_bound : x_j.lower_bound;
        std::optional<Rational> range;
        Clingo::SolverLiteral other_lit = 0;
        if (other != nullptr && is_rational(other->value)) {
            range = rational_part(other->value) - v_j;
            if (!lower) {
                range->neg();
            }
            other_lit = other->lit;
        }
        terms.emplace_back(j, std::move(g_j), v_j, std::move(range), bound.lit, other_lit, lower);
    });
    return applicable && finish_cut(terms, cut_margin, cuts_.cut);
}

template <typename Value> auto Solver<Value>::add_cut_(Clingo::Assignment ass, Clingo::PropagateControl ctl) -> bool {
    auto &[row, rhs, clause] = cuts_.cut;
    // the bounds alone are conflicting
    if (row.empty()) {
        std::ignore = ctl.add_clause(clause);
        return false;
    }
    auto slot = cut_slot_(ass);
    if (!slot.has_value()) {
        return true;
    }
    auto &cut = cuts_.rows[*slot];
    auto &s = variables_[cut.var];
    auto k = s.reverse_index - n_non_basic_;
    // add the row and compute the definition of the cut
    Value value;
    auto &def = cuts_.def;
    def.clear();
    for (auto const &[j, g_j] : row) {
        tableau_.set(k, j, g_j);
        auto jj = variables_[j].index;
        value += mul_div(variables_[jj].value, g_j.num(), g_j.den());
        if (jj >= cuts_.offset) {
            for (auto const &[ll, a_l] : cuts_.rows[jj - cuts_.offset].def) {
                def.emplace_back(ll, a_l * g_j);
            }
        } else {
            def.emplace_back(jj, g_j);
        }
    }
    // combine terms with the same variable
    std::ranges::sort(def, {}, &std::pair<index_t, Rational>::first);
    cut.def.clear();
    for (auto it = def.begin(), ie = def.end(); it != ie;) {
        auto [jj, a_j] = std::move(*it);
        for (++it; it != ie && it->first == jj; ++it) {
            a_j += it->second;
        }
        if (a_j != 0) {
            cut.def.emplace_back(jj, std::move(a_j));
        }
    }
    s.value = std::move(value);
    s.level = 0;
    if (!assignment_trail_.empty()) {
        std::erase(cuts_.fresh, *slot);
        cuts_.fresh.emplace_back(*slot);
    }
    // add the bound
    cut.lit = ctl.add_literal();
    ctl.add_watch(cut.lit);
    auto &bound = bounds_.emplace(cut.lit, Bound{Value{rhs}, cut.var, cut.lit, BoundRelation::GreaterEqual})->second;
    s.bounds.emplace_back(&bound);
    clause.emplace_back(cut.lit);
    ++statistics_.cuts;
    assert_extra(check_tableau_());
    if (!ctl.add_clause(clause)) {
        return false;
    }
    // Make the literal equivalent to the conjunction of the bounds. Otherwise,
    // the solver would have to decide it if the bounds do not hold, which
    // leads to needless search and repeated models.
    for (auto it = clause.begin(), ie = clause.end() - 1; it != ie; ++it) {
        if (!ctl.add_clause({-cut.lit, -*it})) {
            return false;
        }
    }
    std::ignore = ctl.propagate();
    return false;
}

template <typename Value> auto Solver<Value>::cut_slot_(Clingo::Assignment ass) -> std::optional<size_t> {
    auto &rows = cuts_.rows;
    // add a new row
    if (rows.size() < options_.cut_limit) {
        auto ii = static_cast<index_t>(variables_.size());
        assert(ii == cuts_.offset + rows.size());
        auto &s = variables_.emplace_back();
        s.index = ii;
        s.reverse_index = ii;
        ++n_basic_;
        rows.emplace_back(ii, 0, std::vector<std::pair<index_t, Rational>>{});
        return rows.size() - 1;
    }
    // Replace the oldest cut whose slack variable is basic and whose bound is
    // not active. The basic variable does not occur in other rows and its
    // bound is not referenced on the trail.
    auto &next = cuts_.next;
    for (size_t n = 0; n < rows.size(); ++n, next = (next + 1) % rows.size()) {
        auto &cut = rows[next];
        auto &s = variables_[cut.var];
        if (s.reverse_index < n_non_basic_ || ass.is_true(cut.lit)) {
            continue;
        }
        assert(s.lower_bound == nullptr && s.upper_bound == nullptr);
        for (auto it = bounds_.find(cut.lit), ie = bounds_.end(); it != ie && it->first == cut.lit; ++it) {
            if (it->second.variable == cut.var) {
                bounds_.erase(it);
                break;
            }
        }
        s.bounds.clear();
        tableau_.clear_row(s.reverse_index - n_non_basic_);
        auto ret = next;
        next = (next + 1) % rows.size();
        return ret;
    }
    return std::nullopt;
}

template <typename Value> void Solver<Value>::refresh_cuts_() {
    auto level = trail_offset_.empty() ? 0 : trail_offset_.back().level;
    // The values of variables are restored to the last satisfying
    // assignment. Cuts added afterward have to be computed from their
    // definition because the trail does not store their values.
    for (auto k : cuts_.fresh) {
        auto &cut = cuts_.rows[k];
        Value value;
        for (auto const &[jj, a_j] : cut.def) {
            value += mul_div(variables_[jj].value, a_j.num(), a_j.den());
        }
        auto &s = variables_[cut.var];
        s.value = std::move(value);
        s.level = 0;
        if (s.reverse_index >= n_non_basic_) {
            enqueue_(s.reverse_index - n_non_basic_);
        } else if (s.has_lower() && s.value < s.lower()) {
            update_(level, s.reverse_index, s.lower());
        }
    }
    if (assignment_trail_.empty()) {
        cuts_.fresh.clear();
    }
}

// {{{1 Solver: reduced cost fixing

template <typename Value> auto Solver<Value>::fix_reduced_costs_(Clingo::PropagateControl ctl) -> bool {
    // At an optimal assignment, the objective row reads z = sum_j c_j x_j
    // where each non-basic x_j with c_j > 0 is at its upper and each one with
    // c_j < 0 at its lower bound. This identity holds for all solutions, so
    // for z >= L and v_j the current value of x_j we get
    //
    //   c_j (x_j - v_j) >= L - z* - sum_{i != j} c_i (x_i - v_i) >= L - z*
    //
    // where z* is the current objective value and the last step uses the
    // bounds of the x_i. Hence, x_j <= v_j + (z* - L)/|c_j| if c_j < 0 and
    // x_j >= v_j - (z* - L)/c_j if c_j > 0. For integer x_j, the bounds are
    // rounded and added via order literals implied by the bound on z and the
    // bounds of the x_i.
    if (!options_.fix_reduced_costs || !objective_ || !objective_.bounded || !options_.global_objective.has_value()) {
        return true;
    }
    auto const &zb = variables_[objective_.bound_var];
    auto const &zv = variables_[objective_.var];
    if (!zb.has_lower() || zv.reverse_index < n_non_basic_) {
        return true;
    }
    auto gap = zv.value - zb.lower();
    auto z_lit = zb.lower_bound->lit;
    auto z = zv.reverse_index - n_non_basic_;
    // collect the bound literals of the objective row
    bool optimal = true;
    Integer d_z;
    auto &reasons = ints_.fix_reasons;
    reasons.clear();
    tableau_.update_row(z, [&](index_t j, Integer const &a_zj, Integer const &d) -> void {
        auto &x_j = non_basic_(j);
        auto const *bound = (a_zj > 0) == (d > 0) ? x_j.upper_bound : x_j.lower_bound;
        if (bound == nullptr || x_j.value != bound->value) {
            optimal = false;
            return;
        }
        d_z = d;
        reasons.emplace_back(j, bound->lit, a_zj);
    });
    if (!optimal) {
        return true;
    }
    // The loop does not propagate, so the tableau and the assignment do not
    // change.
    bool added = false;
    for (size_t n = 0; n < reasons.size(); ++n) {
        auto const &[j, j_lit, a_zj] = reasons[n];
        auto jj = variables_[j].index;
        if (jj >= n_non_basic_ || !variables_[jj].integer) {
            continue;
        }
        auto const &x_j = variables_[jj];
        auto delta = mul_div(gap, magnitude(d_z), magnitude(a_zj));
        std::optional<Clingo::SolverLiteral> lit;
        if ((a_zj > 0) == (d_z > 0)) {
            // x_j >= k + 1 with k + 1 = ceil(v_j - delta)
            auto k = -floor(-(x_j.value - delta)) - 1;
            if (x_j.has_lower() && Value{as_rational(k + 1)} <= x_j.lower()) {
                continue;
            }
            lit = order_lit_(ctl, jj, std::move(k), false);
            if (lit.has_value()) {
                lit = -*lit;
            }
        } else {
            // x_j <= k with k = floor(v_j + delta)
            auto k = floor(x_j.value + delta);
            if (x_j.has_upper() && Value{as_rational(k)} >= x_j.upper()) {
                continue;
            }
            lit = order_lit_(ctl, jj, std::move(k), true);
        }
        if (!lit.has_value()) {
            return false;
        }
        conflict_clause_.clear();
        conflict_clause_.emplace_back(-z_lit);
        for (size_t m = 0; m < reasons.size(); ++m) {
            if (m != n) {
                conflict_clause_.emplace_back(-std::get<1>(reasons[m]));
            }
        }
        conflict_clause_.emplace_back(*lit);
        ++statistics_.fixed_bounds;
        added = true;
        if (!ctl.add_clause(conflict_clause_)) {
            return false;
        }
    }
    if (added) {
        std::ignore = ctl.propagate();
    }
    return !added;
}

// {{{1 Solver: decisions on branch literals

template <typename Value> auto Solver<Value>::adjust_branch(Clingo::SolverLiteral lit) const -> Clingo::SolverLiteral {
    // After backtracking, the value of a variable often lies on one side of
    // an earlier branch x <= k or x >= k + 1. Selecting this side keeps the
    // current assignment, which usually avoids pivots. If the value is
    // fractional, the nearer side is selected like in Solver::branch_.
    if (options_.branch_phase == BranchPhase::Saved) {
        return lit;
    }
    Bound const *upper = nullptr;
    for (auto l : {lit, -lit}) {
        for (auto it = bounds_.find(l), ie = bounds_.end(); it != ie && it->first == l; ++it) {
            auto const &bound = it->second;
            if (bound.compare(variables_[bound.variable].value)) {
                return l;
            }
            if (bound.rel == BoundRelation::LessEqual) {
                upper = &bound;
            }
        }
    }
    // literals of cuts only have a bound for the positive literal
    if (upper == nullptr) {
        return lit;
    }
    auto const &v = variables_[upper->variable].value;
    bool nearer_upper = v - upper->value < Value{Rational{Integer{1}, Integer{2}}};
    return nearer_upper == (upper->lit == lit) ? lit : -lit;
}

// }}}1

// The solver is instantiated in solving.cc. The functions here are only
// called via the following entry points.
template auto Solver<Rational>::check_integers(Clingo::Assignment ass, Clingo::PropagateControl ctl) -> bool;
template auto Solver<RationalQ>::check_integers(Clingo::Assignment ass, Clingo::PropagateControl ctl) -> bool;
template void Solver<Rational>::refresh_cuts_();
template auto Solver<Rational>::adjust_branch(Clingo::SolverLiteral lit) const -> Clingo::SolverLiteral;
template auto Solver<RationalQ>::adjust_branch(Clingo::SolverLiteral lit) const -> Clingo::SolverLiteral;
template void Solver<RationalQ>::refresh_cuts_();

} // namespace ClingoLPX
