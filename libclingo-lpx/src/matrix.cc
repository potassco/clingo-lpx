#include "matrix.hh"

#include <numeric>
#include <cassert>

Number Matrix::get(index_t i, index_t j) const {
    if (i < rows_.size()) {
        auto const &r = rows_[i];
        auto it = std::lower_bound(r.cells.begin(), r.cells.end(), j);
        if (it != r.cells.end() && it->col == j) {
            return Number{it->val, r.den};
        }
    }
    return zero_();
}

void Matrix::unsafe_get(index_t i, index_t j, Integer *&num, Integer *&den) {
    auto &r = rows_[i];
    num = &std::lower_bound(r.cells.begin(), r.cells.end(), j)->val;
    den = &r.den;
}

// NOLINTBEGIN(clang-analyzer-core.UndefinedBinaryOperatorResult)
void Matrix::set(index_t i, index_t j, Number const &a) {
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
    }
    else {
        auto &r = reserve_row_(i);
        auto it = std::lower_bound(r.cells.begin(), r.cells.end(), j);
        auto [g, ag, rg] = gcd(a.den(), r.den);
        if (it == r.cells.end() || it->col != j) {
            it = r.cells.emplace(it, j, a.num() * rg);
        }
        else {
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

void Matrix::pivot(index_t i, index_t j, Integer &a_ij, Integer &d_i) {
    // Let the tableau have form
    //
    //   (A|D)
    //
    // where A is a matrix of integer coefficients of size m*n and D is a
    // diagonal matrix of size m*m.
    //
    // We begin by eliminating elements A_kj from rows A_k with k != i in the
    // standard way. We below list all the cells that change:
    //
    //   A'_k  = A_kj *  A_i - A_ij * A_k
    //   D'_kk =           0 - A_ij * D_kk
    //   D'_ik = A_kj * D_ii -    0
    //
    // We obtain the matrix (A'|D') which is no longer in tableau form because
    // D' is no longer a diagonal matrix. We can restore this property by
    // swapping columns A'^T_j and D'^T_i.
    //
    // Note that we can devide rows A_k by the greatest common divisior of A_kj
    // and A_ij to keep numbers smaller. In the algorithm, we can initialize
    // two variables for A_kj/g and A_ij/g right from the start to keep integer
    // divisions to a minimum:
    //
    //   A'_k  = (A_kj / g) * A_i  - (A_ij / g) * A_k
    //   D'_kk = (A_kj / g) * 0    - (A_ij / g) * D_kk
    //   D'_ki = (A_kj / g) * D_ii - (A_ij / g) * 0

    auto A_i0 = rows_[i].cells.begin();
    auto A_in = rows_[i].cells.end();
    std::vector<size_t> sizes;
    sizes.resize(rows_[i].cells.size());
    std::vector<Cell> row;
    std::vector<index_t> col_buf;

    // Note that insertions into rows and columns do not invert iterators:
    // - row i is unaffected because k != i
    // - there are no insertions in column j because each a_kj != 0
    update_col(j, [&](index_t k, Integer const &a_kj, Integer &d_k) {
        if (k != i) {
            auto [g, ga_ij, ga_kj] = gcd(a_ij, a_kj);
            ga_ij.neg();
            size_t pivot_index = 0;
            for (auto A_il = A_i0, A_kl = rows_[k].cells.begin(), A_kn = rows_[k].cells.end(); A_il != A_in || A_kl != A_kn; ) {
                // case A_il != 0 and A_kl == 0
                if (A_kl == A_kn || (A_il != A_in && A_il->col < A_kl->col)) {
                    assert(A_il->col != j);
                    // add A_kj * A_ix for x != j
                    row.emplace_back(A_il->col, ga_kj * A_il->val);
                    // Note that vectors will be sorted at the end.
                    cols_[A_il->col].emplace_back(k);
                    ++sizes[A_il - A_i0];
                    ++A_il;
                }
                // case A_il == 0 and A_kl != 0
                else if (A_il == A_in || A_kl->col < A_il->col) {
                    assert(A_kl->col != j);
                    // add A_kx for x != j
                    row.emplace_back(A_kl->col, std::move(A_kl->val *= ga_ij));
                    ++A_kl;
                }
                // case A_il != 0 and A_kl != 0
                else {
                    // case l != j
                    if (A_kl->col != j) {
                        // add A_kx + A_kj * A_ix for x != j
                        row.emplace_back(A_kl->col, std::move((A_kl->val *= ga_ij).add_mul(ga_kj, A_il->val)));
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
            row[pivot_index].val = std::move(ga_kj *= d_i);
            std::swap(rows_[k].cells, row);
            row.clear();
        }
    });

    // pivot element in row i
    a_ij.swap(d_i);

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

size_t Matrix::size() const {
    return std::accumulate(rows_.begin(), rows_.end(), static_cast<size_t>(0),
                           [](size_t n, auto const &r) { return n + r.cells.size(); });
}

bool Matrix::empty() const {
    return std::all_of(rows_.cbegin(), rows_.cend(),
                       [](auto const &r) { return r.cells.empty(); });
}

void Matrix::clear() {
    rows_.clear();
    cols_.clear();
}

Matrix::Row &Matrix::reserve_row_(index_t i) {
    if (rows_.size() <= i) {
        rows_.resize(i + 1);
    }
    return rows_[i];
}
std::vector<index_t> &Matrix::reserve_col_(index_t j) {
    if (cols_.size() <= j) {
        cols_.resize(j + 1);
    }
    return cols_[j];
}

Number const &Matrix::zero_() {
    static Number zero{0};
    return zero;
}
