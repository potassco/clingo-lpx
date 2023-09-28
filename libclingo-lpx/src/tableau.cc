#include "tableau.hh"

#include <cassert>
#include <iostream>
#include <numeric>

auto Tableau::get(index_t i, index_t j) const -> Rational {
    if (i < rows_.size()) {
        auto const &r = rows_[i];
        auto it = std::lower_bound(r.cells.begin(), r.cells.end(), j);
        if (it != r.cells.end() && it->col == j) {
            return Rational{it->val, r.den};
        }
    }
    return zero_();
}

void Tableau::unsafe_get(index_t i, index_t j, Integer *&num, Integer *&den) {
    auto &r = rows_[i];
    num = &std::lower_bound(r.cells.begin(), r.cells.end(), j)->val;
    den = &r.den;
}

// NOLINTBEGIN(clang-analyzer-core.UndefinedBinaryOperatorResult)
void Tableau::set(index_t i, index_t j, Rational const &a) {
    // This implementation assumes that the set function is only called during
    // intialization and in the best case with already sorted elements that
    // have the same denominator.
    if (a == 0) {
        if (i < rows_.size()) {
            auto &r = rows_[i].cells;
            auto it = std::lower_bound(r.begin(), r.end(), j);
            if (it != r.end() && it->col == j) {
                r.erase(it);
            }
        }
    } else {
        auto &r = reserve_row_(i);
        auto it = std::lower_bound(r.cells.begin(), r.cells.end(), j);
        auto [g, ag, rg] = gcd_div(a.den(), r.den);
        if (it == r.cells.end() || it->col != j) {
            it = r.cells.emplace(it, j, a.num() * rg);
        } else {
            // Note: this case is only for completeness it is not going to be
            // used in practice.
            it->val = a.num() * rg;
        }
        if (ag != 1) {
            r.den *= ag;
            for (auto jt = r.cells.begin(); jt != r.cells.end(); ++jt) {
                if (jt != it) {
                    jt->val *= ag;
                }
            }
        }
        auto &col = reserve_col_(j);
        auto jt = std::lower_bound(col.begin(), col.end(), i);
        if (jt == col.end() || *jt != i) {
            col.emplace(jt, i);
        }
    }
}
// NOLINTEND(clang-analyzer-core.UndefinedBinaryOperatorResult)

void Tableau::simplify_(index_t i) {
    auto &row = rows_[i];
    if (row.den == 1) {
        return;
    }
    auto g = row.den;
    for (auto &[col, val] : row.cells) {
        g = gcd(val, g);
        if (g == 1) {
            return;
        }
    }
    for (auto &[col, val] : row.cells) {
        static_cast<void>(col);
        val.divide(g);
    }
    row.den.divide(g);
}

void Tableau::pivot(index_t i, index_t j, Integer &a_ij, Integer &d_i) {
    // Detailed notes how this algorithm works can be found in doc/pivot.lyx.

    auto A_i0 = rows_[i].cells.begin();
    auto A_in = rows_[i].cells.end();
    std::vector<size_t> sizes;
    sizes.resize(rows_[i].cells.size());
    std::vector<Cell> row;
    std::vector<index_t> col_buf;

    update_row(i, [&](index_t k, Integer &a_il, Integer &d_i) {
        static_cast<void>(d_i);
        if (k != j) {
            a_il.neg();
        }
    });
    // Note that insertions into rows and columns do not invert iterators:
    // - row i is unaffected because k != i
    // - there are no insertions in column j because each a_kj != 0
    update_col(j, [&](index_t k, Integer const &a_kj, Integer &d_k) {
        if (k != i) {
            auto [g, ga_ij, ga_kj] = gcd_div(a_ij, a_kj);
            size_t pivot_index = 0;
            for (auto A_il = A_i0, A_kl = rows_[k].cells.begin(), A_kn = rows_[k].cells.end();
                 A_il != A_in || A_kl != A_kn;) {
                // case A_il != 0 and A_kl == 0
                if (A_kl == A_kn || (A_il != A_in && A_il->col < A_kl->col)) {
                    assert(A_il->col != j);
                    row.emplace_back(A_il->col, ga_kj * A_il->val);
                    // Note that vectors will be sorted at the end.
                    cols_[A_il->col].emplace_back(k);
                    ++sizes[A_il - A_i0];
                    ++A_il;
                }
                // case A_il == 0 and A_kl != 0
                else if (A_il == A_in || A_kl->col < A_il->col) {
                    assert(A_kl->col != j);
                    row.emplace_back(A_kl->col, std::move(A_kl->val) * ga_ij);
                    ++A_kl;
                }
                // case A_il != 0 and A_kl != 0
                else {
                    // case l != j
                    if (A_kl->col != j) {
                        row.emplace_back(A_kl->col, (std::move(A_kl->val) * ga_ij).add_mul(ga_kj, A_il->val));
                        if (row.back().val == 0) {
                            row.pop_back();
                        }
                    }
                    // case l == j
                    else {
                        // pivot
                        pivot_index = row.size();
                        row.emplace_back(A_kl->col, 0);
                        d_k *= ga_ij;
                    }
                    ++A_il;
                    ++A_kl;
                }
            }
            // finish pivoting to benefit from another move
            row[pivot_index].val = std::move(ga_kj) * d_i;
            std::swap(rows_[k].cells, row);
            row.clear();
            simplify_(k);
        }
    });
    // pivot element in row i
    a_ij.swap(d_i);
    simplify_(i);

    // Ensure that column vectors are sorted.
    //
    // Note that we cannot assume that elements are unique because of the
    // lazy deletion scheme. Hence, A_il is not possible to use
    // std::merge_inplace.
    auto jt = sizes.begin();
    for (auto A_il = A_i0; A_il != A_in; ++A_il, ++jt) {
        if (*jt == 0) {
            continue;
        }
        auto &col = cols_[A_il->col];
        // Optimize for the case that only one element is inserted.
        if (*jt == 1) {
            auto j = col.back();
            col.pop_back();
            auto kt = std::lower_bound(col.begin(), col.end(), j);
            if (kt == col.end() || *kt != j) {
                col.emplace(kt, j);
            }
        }
        // Use set_union for the general case.
        else {
            auto im = col.end() - static_cast<std::make_signed_t<std::size_t>>(*jt);
            std::set_union(col.begin(), im, im, col.end(), std::back_inserter(col_buf));
            std::swap(col_buf, col);
            col_buf.clear();
        }
    }
}

void Tableau::debug(char const *indent) const {
    size_t m = rows_.size();
    size_t n = cols_.size();
    for (size_t i = 0; i < m; ++i) {
        std::cerr << indent;
        std::cerr << "y_" << i << " = ";
        for (size_t j = 0; j < n; ++j) {
            if (j > 0) {
                std::cerr << " + ";
            }
            std::cerr << get(i, j) << "*x_" << j;
        }
        std::cerr << "\n";
    }
}

auto Tableau::size() const -> size_t {
    return std::accumulate(rows_.begin(), rows_.end(), static_cast<size_t>(0),
                           [](size_t n, auto const &r) { return n + r.cells.size(); });
}

auto Tableau::empty() const -> bool {
    return std::all_of(rows_.cbegin(), rows_.cend(), [](auto const &r) { return r.cells.empty(); });
}

void Tableau::clear() {
    rows_.clear();
    cols_.clear();
}

auto Tableau::reserve_row_(index_t i) -> Tableau::Row & {
    if (rows_.size() <= i) {
        rows_.resize(i + 1);
    }
    return rows_[i];
}
auto Tableau::reserve_col_(index_t j) -> std::vector<index_t> & {
    if (cols_.size() <= j) {
        cols_.resize(j + 1);
    }
    return cols_[j];
}

auto Tableau::zero_() -> Rational const & {
    static Rational zero{0};
    return zero;
}
