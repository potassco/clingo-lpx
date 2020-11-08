#include <solving.hh>

#include <unordered_set>

void Statistics::reset() {
    *this = {};
}

Solver::Solver(std::vector<Equation> &&equations)
: equations_{std::move(equations)} { }

void Solver::prepare() {
    std::unordered_map<Clingo::Symbol, index_t> indices;
    index_t i{0};
    for (auto var : vars_()) {
        indices.emplace(var, i++);
    }

    n_non_basic_ = indices.size();
    n_basic_ = equations_.size();

    assignment_.clear();
    assignment_.resize(n_basic_ + n_non_basic_, 0);

    tableau_.clear();
    bounds_.clear();
    i = 0;
    for (auto const &x : equations_) {
        for (auto const &y : x.lhs) {
            tableau_.set(i, indices[y.var], tableau_.get(i, indices[y.var]) + y.co);
        }

        bounds_.emplace_back();
        switch (x.op) {
            case Operator::LessEqual: {
                bounds_.back().upper = x.rhs;
                break;
            }
            case Operator::GreaterEqual: {
                bounds_.back().lower = x.rhs;
                break;
            }
            case Operator::Equal: {
                bounds_.back().lower = x.rhs;
                bounds_.back().upper = x.rhs;
                break;
            }

        }
        ++i;
    }

    variables_.clear();
    for (index_t i = 0; i < n_non_basic_ + n_basic_; ++i) {
        variables_.emplace_back(i);
    }

    statistics_.reset();
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
                    ret.emplace_back(var, assignment_[variables_[k++]]);
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
    for (auto const &x : equations_) {
        for (auto const &y : x.lhs) {
            var_set.emplace(y.var);
        }
    }
    std::vector<Clingo::Symbol> var_vec{var_set.begin(), var_set.end()};
    std::sort(var_vec.begin(), var_vec.end());
    return var_vec;
};

bool Solver::check_() {
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
            assignment_[variables_[n_non_basic_ + k]] += (a_kj * dj);
        }
    });
    assert(check_());

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
    assert(check_());
}

Solver::State Solver::select_(index_t &ret_i, index_t &ret_j, Number &ret_v) {
    // TODO: This can be done while pivoting as well!
    std::vector<std::pair<index_t, index_t>> basic;
    std::vector<std::pair<index_t, index_t>> non_basic;
    for (index_t i = 0; i < n_basic_; ++i) {
        basic.emplace_back(i, variables_[i + n_non_basic_]);
    }
    std::sort(basic.begin(), basic.end(), [](auto const &a, auto const &b){ return a.second < b.second; });
    for (index_t j = 0; j < n_non_basic_; ++j) {
        non_basic.emplace_back(j, variables_[j]);
    }
    std::sort(non_basic.begin(), non_basic.end(), [](auto const &a, auto const &b){ return a.second < b.second; });

    for (auto [i, xi] : basic) {
        auto const &axi = assignment_[xi];

        if (xi < n_non_basic_) {
            continue;
        }

        if (auto const &li = bounds_[xi - n_non_basic_].lower; li && axi < *li) {
            for (auto [j, xj] : non_basic) {
                auto const &a_ij = tableau_.get(i, j);
                auto const &v_xj = assignment_[xj];
                if ((a_ij > 0 && (xj < n_non_basic_ || !bounds_[xj - n_non_basic_].upper || v_xj < *bounds_[xj - n_non_basic_].upper)) ||
                    (a_ij < 0 && (xj < n_non_basic_ || !bounds_[xj - n_non_basic_].lower || v_xj > *bounds_[xj - n_non_basic_].lower))) {
                    ret_i = i;
                    ret_j = j;
                    ret_v = *li;
                    return State::Unknown;
                }
            }
            return State::Unsatisfiable;
        }

        if (auto const &ui = bounds_[xi - n_non_basic_].upper; ui && axi > *ui) {
            for (auto [j, xj] : non_basic) {
                auto const &a_ij = tableau_.get(i, j);
                auto const &v_xj = assignment_[xj];
                if ((a_ij < 0 && (xj < n_non_basic_ || !bounds_[xj - n_non_basic_].upper || v_xj < *bounds_[xj - n_non_basic_].upper)) ||
                    (a_ij > 0 && (xj < n_non_basic_ || !bounds_[xj - n_non_basic_].lower || v_xj > *bounds_[xj - n_non_basic_].lower))) {
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
