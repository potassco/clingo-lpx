#include <solving.hh>

#include <unordered_set>

struct Solver::Prepare {
    index_t add_non_basic(Solver &s, Clingo::Symbol var) {
        auto [jt, res] = s.indices_.emplace(var, n_vars);
        if (res) {
            non_basic.emplace_back(n_vars);
            s.bounds_.emplace_back();
            s.assignment_.emplace_back();
            ++n_vars;
        }
        return std::distance(non_basic.begin(), std::lower_bound(non_basic.begin(), non_basic.end(), jt->second));
    }

    index_t add_basic(Solver &s) {
        basic.emplace_back(n_vars);
        s.bounds_.emplace_back();
        s.assignment_.emplace_back();
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
        s.n_non_basic_ = non_basic.size();
        for (auto &var : non_basic) {
            s.variables_.emplace_back(var);
        }
        s.n_basic_ = basic.size();
        for (auto &var : basic) {
            s.variables_.emplace_back(var);
        }
    }

    index_t n_vars{0};
    std::vector<index_t> basic;
    std::vector<index_t> non_basic;
};

bool Solver::Bounds::update_lower(Number const &v) {
    if (!lower || v > *lower) {
        lower = v;
    }
    return !upper || *lower <= *upper;
}

bool Solver::Bounds::update_upper(Number const &v) {
    if (!upper || v < *upper) {
        upper = v;
    }
    return !lower || *lower <= *upper;
}

bool Solver::Bounds::update(Relation rel, Number const &v) {
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

void Statistics::reset() {
    *this = {};
}

Solver::Solver(std::vector<Inequality> &&inequalities)
: inequalities_{std::move(inequalities)} { }

bool Solver::prepare() {
    assignment_.clear();
    bounds_.clear();
    tableau_.clear();
    variables_.clear();
    indices_.clear();
    statistics_.reset();

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
                        return false;
                    }
                    break;
                }
                case Relation::GreaterEqual: {
                    if (x.rhs < 0) {
                        return false;
                    }
                    break;
                }
                case Relation::Equal: {
                    if (x.rhs != 0) {
                        return false;
                    }
                    break;
                }
            }
        }
        // add a bound to a non-basic variable
        else if (row.size() == 1) {
            auto const &[j, v] = row.front();
            if (!bounds_[j].upper && !bounds_[j].lower) {
                update.emplace_back(j);
            }
            if (!bounds_[j].update(v < 0 ? invert(x.rel) : x.rel, x.rhs / v)) {
                return false;
            }
        }
        // add an inequality
        else {
            auto i = prep.add_basic(*this);
            if (!bounds_.back().update(x.rel, x.rhs)) {
                return false;
            }

            for (auto const &[j, v] : row) {
                tableau_.set(i, j, v);
            }
        }
    }

    prep.finish(*this);

    for (auto j : update) {
        if (bounds_[j].lower) {
            update_(j, *bounds_[j].lower);
        }
        else if (bounds_[j].upper) {
            update_(j, *bounds_[j].upper);
        }
    }

    assert_extra(check_tableau_());
    assert_extra(check_non_basic_());

    return true;
}

std::optional<std::vector<std::pair<Clingo::Symbol, Number>>> Solver::solve() {
    index_t i{0};
    index_t j{0};
    Number v{0};

    while (true) {
        switch (select_(i, j, v)) {
            case State::Satisfiable: {
                std::vector<std::pair<Clingo::Symbol, Number>> ret;
                index_t k{0};
                for (auto var : vars_()) {
                    if (auto it = indices_.find(var); it != indices_.end()) {
                        ret.emplace_back(var, assignment_[it->second]);
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
                pivot_(i, j, v);
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
            v_i += assignment_[variables_[j]] * a_ij;
        });
        if (v_i != assignment_[variables_[n_non_basic_ + i]]) {
            return false;
        }
    }
    return true;
}

bool Solver::check_non_basic_() {
    for (index_t j = 0; j < n_non_basic_; ++j) {
        auto xj = variables_[j];
        auto const &[lower, upper] = bounds_[xj];
        if (lower && assignment_[xj] < *lower) {
            return false;
        }
        if (upper && assignment_[xj] > *upper) {
            return false;
        }
    }
    return true;
}

void Solver::update_(index_t j, Number v) {
    auto const &a_xj = assignment_[variables_[j]];
    tableau_.update_col(j, [&](index_t i, Number const &a_ij) {
        assignment_[variables_[n_non_basic_ + i]] += a_ij * (v - a_xj);
    });
    assignment_[variables_[j]] = v;
}

void Solver::pivot_(index_t i, index_t j, Number const &v) {
    auto a_ij = tableau_.get(i, j);
    assert(a_ij != 0);

    // adjust assignment
    auto ii = i + n_non_basic_;
    Number dj = (v - assignment_[variables_[ii]]) / a_ij;
    assignment_[variables_[ii]] = v;
    assignment_[variables_[j]] += dj;
    tableau_.update_col(j, [&](index_t k, Number const &a_kj) {
        if (k != i) {
            // Note that a bound can become conflicting here
            assignment_[variables_[n_non_basic_ + k]] += a_kj * dj;
        }
    });
    assert_extra(check_tableau_());

    // swap variables x_i and x_j
    std::swap(variables_[ii], variables_[j]);

    // invert row i
    tableau_.update_row(i, [&](index_t k, Number &a_ik) {
        if (k == j) {
            a_ik = 1 / a_ij;
        }
        else {
            a_ik /= -a_ij;
        }
    });

    // eliminate x_j from rows k != i
    tableau_.update_col(j, [&](index_t k, Number const &a_kj) {
        if (k != i) {
            tableau_.update_row(i, [&](index_t l, Number const &a_il) {
                Number a_kl;
                if (l == j) {
                    a_kl = a_kj / a_ij;
                }
                else {
                    a_kl = tableau_.get(k, l) + a_il * a_kj;
                }
                // Note that this call does not invalidate active iterators:
                // - row i is unaffected because k != i
                // - there are no insertions in column j because each a_kj != 0
                //   (values in the column can change though)
                tableau_.set(k, l, a_kl);
            });
        }
    });

    ++statistics_.pivots_;
    assert_extra(check_tableau_());
    assert_extra(check_non_basic_());
}

Solver::State Solver::select_(index_t &ret_i, index_t &ret_j, Number &ret_v) {
    // TODO: This can be done while pivoting as well!
    std::vector<std::pair<index_t, index_t>> basic;
    std::vector<std::pair<index_t, index_t>> non_basic;
    for (index_t i = 0; i < n_basic_; ++i) {
        basic.emplace_back(i, variables_[i + n_non_basic_]);
    }
        // Note that a bound can become conflicting here
    std::sort(basic.begin(), basic.end(), [](auto const &a, auto const &b){ return a.second < b.second; });
    for (index_t j = 0; j < n_non_basic_; ++j) {
        non_basic.emplace_back(j, variables_[j]);
    }
    std::sort(non_basic.begin(), non_basic.end(), [](auto const &a, auto const &b){ return a.second < b.second; });

    for (auto [i, xi] : basic) {
        auto const &axi = assignment_[xi];

        if (auto const &li = bounds_[xi].lower; li && axi < *li) {
            for (auto [j, xj] : non_basic) {
                auto const &a_ij = tableau_.get(i, j);
                auto const &v_xj = assignment_[xj];
                if ((a_ij > 0 && (xj < n_non_basic_ || !bounds_[xj].upper || v_xj < *bounds_[xj].upper)) ||
                    (a_ij < 0 && (xj < n_non_basic_ || !bounds_[xj].lower || v_xj > *bounds_[xj].lower))) {
                    ret_i = i;
                    ret_j = j;
                    ret_v = *li;
                    return State::Unknown;
                }
            }
            return State::Unsatisfiable;
        }

        if (auto const &ui = bounds_[xi].upper; ui && axi > *ui) {
            for (auto [j, xj] : non_basic) {
                auto const &a_ij = tableau_.get(i, j);
                auto const &v_xj = assignment_[xj];
                if ((a_ij < 0 && (xj < n_non_basic_ || !bounds_[xj].upper || v_xj < *bounds_[xj].upper)) ||
                    (a_ij > 0 && (xj < n_non_basic_ || !bounds_[xj].lower || v_xj > *bounds_[xj].lower))) {
                    ret_i = i;
                    ret_j = j;
                    ret_v = *ui;
                    return State::Unknown;
                }
            }
            return State::Unsatisfiable;
        }
    }

    return State::Satisfiable;
}
