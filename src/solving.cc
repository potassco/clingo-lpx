#include <solving.hh>

#include <unordered_set>

struct Solver::Prepare {
    index_t add_non_basic(Solver &s, Clingo::Symbol var) {
        auto [jt, res] = s.indices_.emplace(var, n_vars);
        if (res) {
            s.variables_.emplace_back();
            // Note: that this makes it possible to use `Solver::non_basic_`
            // during initialization
            s.variables_[s.n_non_basic_].index = n_vars;
            s.variables_[n_vars].reserve_index = s.n_non_basic_;
            ++n_vars;
            ++s.n_non_basic_;
        }
        return s.variables_[jt->second].reserve_index;
    }

    index_t add_basic(Solver &s) {
        basic.emplace_back(n_vars);
        s.variables_.emplace_back();
        ++n_vars;
        return basic.size() - 1;
    }

    std::vector<std::pair<index_t, Number>> add_row(Solver &s, Inequality const &x) {
        std::vector<std::pair<index_t, Number>> row;
        std::vector<Clingo::Symbol> vars;
        std::unordered_map<Clingo::Symbol, Number> cos;

        // combine cofficients
        for (auto const &y : x.lhs) {
            if (y.co == 0) {
                continue;
            }
            if (auto [it, res] = cos.emplace(y.var, y.co); !res) {
                it->second += y.co;
                if (it->second == 0) {
                    cos.erase(it);
                }
            }
            else {
                vars.emplace_back(y.var);
            }
        }

        // add non-basic variables for the remaining non-zero coefficients
        for (auto &var : vars) {
            if (auto it = cos.find(var); it != cos.end()) {
                index_t j = add_non_basic(s, var);
                row.emplace_back(j, it->second);
            }
        }

        return row;
    }

    void finish(Solver &s) {
        s.n_basic_ = basic.size();
        int i = s.n_non_basic_;
        for (auto &index : basic) {
            s.variables_[index].reserve_index = i;
            s.variables_[i].index = index;
            ++i;
        }
    }

    index_t n_vars{0};
    std::vector<index_t> basic;
};

bool Solver::Variable::update_lower(Number const &v) {
    if (!lower || v > *lower) {
        lower = v;
    }
    return !upper || *lower <= *upper;
}

bool Solver::Variable::update_upper(Number const &v) {
    if (!upper || v < *upper) {
        upper = v;
    }
    return !lower || *lower <= *upper;
}

bool Solver::Variable::update(Relation rel, Number const &v) {
    switch (rel) {
        case Relation::LessEqual: {
            return update_upper(v);
        }
        case Relation::GreaterEqual: {
            return update_lower(v);
        }
        case Relation::Equal: {
            return update_lower(v) && update_upper(v);
        }
    }
    assert(false);
}

bool Solver::Variable::has_conflict() {
    return (lower && value < *lower) || (upper && value > *upper);
}

void Statistics::reset() {
    *this = {};
}

Solver::Solver(std::vector<Inequality> &&inequalities)
: inequalities_{std::move(inequalities)} { }

Solver::Variable &Solver::basic_(index_t i) {
    return variables_[variables_[i + n_non_basic_].index];
}

Solver::Variable &Solver::non_basic_(index_t j) {
    return variables_[variables_[j].index];
}

void Solver::enqueue_(index_t i) {
    auto ii = variables_[i + n_non_basic_].index;
    auto &xi = variables_[ii];
    if (!xi.queued && xi.has_conflict()) {
        conflicts_.emplace(ii);
        xi.queued = true;
    }
}

bool Solver::prepare() {
    tableau_.clear();
    variables_.clear();
    indices_.clear();
    statistics_.reset();
    n_basic_ = 0;
    n_non_basic_ = 0;

    // TODO: we need a datastructure for bounds here. It will probably be a map
    // from literals to variable, bound pairs. There will be the following
    // functions:
    //  - ensure_level:
    //    - opens a new decision level providing a trail of bounds for
    //      backtracking
    //  - propagate_literal:
    //    - adds all bounds associated with the given literal
    //      - only bound refinements are considered
    //      - adds the previous value of a bound to a trail
    //  - backtrack
    //    - must be called for each ensure_level call
    //    - restores the bounds on the trail
    //    - on backtracking bounds become weaker
    //    - if we make sure that the last conflict remains in the conflict
    //      queue, then we can keep the conflict queue around and solve can be
    //      used to restore a consistent state

    // TODO: Bounds associated with a variable form a propagation chain. We can
    // add binary clauses to propagate them. For example
    //
    //     `x >= u` implies not `x <= l` for all `l < u`.
    //
    // Care has to be taken because we cannot use
    //
    //     `x >= u` implies `x >= u'` for all u' >= u
    //
    // because I am going for a non-strict defined semantics.

    Prepare prep;
    std::vector<index_t> update;
    for (auto const &x : inequalities_) {
        // transform inequality into row suitable for tableau
        auto row = prep.add_row(*this, x);

        // check bound against 0
        if (row.empty()) {
            switch (x.rel) {
                case Relation::LessEqual: {
                    if (x.rhs > 0) {
                        // TODO!
                        // This makes the literal associated with the bound false.
                        return false;
                    }
                    break;
                }
                case Relation::GreaterEqual: {
                    if (x.rhs < 0) {
                        // TODO!
                        // This makes the literal associated with the bound false.
                        return false;
                    }
                    break;
                }
                case Relation::Equal: {
                    if (x.rhs != 0) {
                        // TODO!
                        // This makes the literal associated with the bound false.
                        return false;
                    }
                    break;
                }
            }
        }
        // add a bound to a non-basic variable
        else if (row.size() == 1) {
            auto const &[j, v] = row.front();
            auto &xj = non_basic_(j);
            // TODO!
            // Bounds associated with a true literal can be added like this here.
            // Bounds associated with a false literal can be ignored.
            // Bounds associated with a free literal must be kept for later.
            if (!xj.upper && !xj.lower) {
                update.emplace_back(j);
            }
            if (!xj.update(v < 0 ? invert(x.rel) : x.rel, x.rhs / v)) {
                return false;
            }
        }
        // add an inequality
        else {
            auto i = prep.add_basic(*this);
            // TODO!
            // This can use the same logic as above (even though the case is
            // simpler in principle because a slack variale is not conflicting
            // by construction).
            if (!variables_.back().update(x.rel, x.rhs)) {
                return false;
            }

            for (auto const &[j, v] : row) {
                tableau_.set(i, j, v);
            }
        }
    }

    prep.finish(*this);

    for (auto j : update) {
        auto &xj = non_basic_(j);
        if (xj.lower) {
            update_(j, *xj.lower);
        }
        else if (xj.upper) {
            update_(j, *xj.upper);
        }
    }

    for (size_t i = 0; i < n_basic_; ++i) {
        enqueue_(i);
    }

    assert_extra(check_tableau_());
    assert_extra(check_basic_());
    assert_extra(check_non_basic_());

    return true;
}

std::optional<std::vector<std::pair<Clingo::Symbol, Number>>> Solver::solve() {
    index_t i{0};
    index_t j{0};
    Number const *v{nullptr};

    while (true) {
        switch (select_(i, j, v)) {
            case State::Satisfiable: {
                std::vector<std::pair<Clingo::Symbol, Number>> ret;
                index_t k{0};
                for (auto var : vars_()) {
                    if (auto it = indices_.find(var); it != indices_.end()) {
                        ret.emplace_back(var, variables_[it->second].value);
                    }
                    else {
                        ret.emplace_back(var, 0);
                    }
                }
                return ret;
            }
            case State::Unsatisfiable: {
                return std::nullopt;
            }
            case State::Unknown: {
                assert(v != nullptr);
                pivot_(i, j, *v);
            }
        }
    }
}

Statistics const &Solver::statistics() const {
    return statistics_;
}

std::vector<Clingo::Symbol> Solver::vars_() {
    std::unordered_set<Clingo::Symbol> var_set;
    for (auto const &x : inequalities_) {
        for (auto const &y : x.lhs) {
            var_set.emplace(y.var);
        }
    }
    std::vector<Clingo::Symbol> var_vec{var_set.begin(), var_set.end()};
    std::sort(var_vec.begin(), var_vec.end());
    return var_vec;
};

bool Solver::check_tableau_() {
    for (index_t i{0}; i < n_basic_; ++i) {
        Number v_i{0};
        tableau_.update_row(i, [&](index_t j, Number const &a_ij){
            v_i += non_basic_(j).value * a_ij;
        });
        if (v_i != basic_(i).value) {
            return false;
        }
    }
    return true;
}

bool Solver::check_basic_() {
    for (index_t i = 0; i < n_basic_; ++i) {
        auto &xi = basic_(i);
        if (xi.lower && xi.value < *xi.lower && !xi.queued) {
            return false;
        }
        if (xi.upper && xi.value > *xi.upper && !xi.queued) {
            return false;
        }
    }
    return true;
}

bool Solver::check_non_basic_() {
    for (index_t j = 0; j < n_non_basic_; ++j) {
        auto &xj = non_basic_(j);
        if (xj.lower && xj.value < *xj.lower) {
            return false;
        }
        if (xj.upper && xj.value > *xj.upper) {
            return false;
        }
    }
    return true;
}

bool Solver::check_solution_() {
    for (auto &x : variables_) {
        if (x.lower && *x.lower > x.value) {
            return false;
        }
        if (x.upper && x.value > *x.upper) {
            return false;
        }
    }
    return check_tableau_() && check_basic_();
}

void Solver::update_(index_t j, Number v) {
    auto &xj = non_basic_(j);
    tableau_.update_col(j, [&](index_t i, Number const &a_ij) {
        basic_(i).value += a_ij * (v - xj.value);
    });
    xj.value = v;
}

void Solver::pivot_(index_t i, index_t j, Number const &v) {
    auto &a_ij = tableau_.unsafe_get(i, j);
    assert(a_ij != 0);

    auto &xi = basic_(i);
    auto &xj = non_basic_(j);

    // adjust assignment
    Number dj = (v - xi.value) / a_ij;
    xi.value = v;
    xj.value += dj;
    tableau_.update_col(j, [&](index_t k, Number const &a_kj) {
        if (k != i) {
            basic_(k).value += a_kj * dj;
            enqueue_(k);
        }
    });
    assert_extra(check_tableau_());

    // swap variables x_i and x_j
    std::swap(xi.reserve_index, xj.reserve_index);
    std::swap(variables_[i + n_non_basic_].index, variables_[j].index);
    enqueue_(i);

    // invert row i
    tableau_.update_row(i, [&](index_t k, Number &a_ik) {
        if (k != j) {
            a_ik /= -a_ij;
        }
    });
    a_ij = 1 / a_ij;

    // eliminate x_j from rows k != i
    tableau_.update_col(j, [&](index_t k, Number &a_kj) {
        if (k != i) {
            // Note that this call does not invalidate active iterators:
            // - row i is unaffected because k != i
            // - there are no insertions in column j because each a_kj != 0
            tableau_.update_row(i, [&](index_t l, Number const &a_il) {
                if (l != j) {
                    tableau_.update(k, l, [&](Number &a_kl) { a_kl += a_il * a_kj; });
                }
            });
            // Note that a_ij was inverted above.
            a_kj *= a_ij;
        }
    });

    ++statistics_.pivots_;
    assert_extra(check_tableau_());
    assert_extra(check_basic_());
    assert_extra(check_non_basic_());
}

Solver::State Solver::select_(index_t &ret_i, index_t &ret_j, Number const *&ret_v) {
    // This implements Bland's rule selecting the variables with the smallest
    // indices for pivoting.

    // TODO:
    // This function must be extended with conflict generation. A conflict
    // consists of all literals associated with bounds that prevented a bound
    // update.

    while (!conflicts_.empty()) {
        auto &xi = variables_[conflicts_.top()];
        auto i = xi.reserve_index;
        assert(conflicts_.top() == variables_[i].index);
        xi.queued = false;
        conflicts_.pop();
        // the queue might contain variables that meanwhile became basic
        if (i < n_non_basic_) {
            continue;
        }
        i -= n_non_basic_;

        if (xi.lower && xi.value < *xi.lower) {
            index_t kk = variables_.size();
            tableau_.update_row(i, [&](index_t j, Number const &a_ij) {
                auto jj = variables_[j].index;
                if (jj < kk) {
                    auto &xj = variables_[jj];
                    if ((a_ij > 0 && (!xj.upper || xj.value < *xj.upper)) ||
                        (a_ij < 0 && (!xj.lower || xj.value > *xj.lower))) {
                        kk = jj;
                        ret_i = i;
                        ret_j = j;
                        ret_v = &*xi.lower;
                    }
                }
            });
            return kk == variables_.size() ? State::Unsatisfiable : State::Unknown;
        }

        if (xi.upper && xi.value > *xi.upper) {
            index_t kk = variables_.size();
            tableau_.update_row(i, [&](index_t j, Number const &a_ij) {
                auto jj = variables_[j].index;
                if (jj < kk) {
                    auto &xj = variables_[jj];
                    if ((a_ij < 0 && (!xj.upper || xj.value < *xj.upper)) ||
                        (a_ij > 0 && (!xj.lower || xj.value > *xj.lower))) {
                        kk = jj;
                        ret_i = i;
                        ret_j = j;
                        ret_v = &*xi.upper;
                    }
                }
            });
            return kk == variables_.size() ? State::Unsatisfiable : State::Unknown;
        }
    }

    assert(check_solution_());

    return State::Satisfiable;
}
