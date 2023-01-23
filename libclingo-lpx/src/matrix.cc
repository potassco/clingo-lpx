#include "matrix.hh"

#include <numeric>

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
    *num = std::lower_bound(r.cells.begin(), r.cells.end(), j)->val;
    *den = r.den;
}

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
        // TODO: It looks a bit like the gcd function could already return at
        // tuple with these three values to avoid having to add division to the
        // integer class. At the very least integer division should be renamed
        // as the flint library does.
        auto g = gcd(a.den(), r.den);
        auto rg = r.den / g;
        auto ag = a.den() / g;
        r.den *= ag;
        if (it == r.cells.end() || it->col != j) {
            it = r.cells.emplace(it, j, a.num() * rg);
        }
        else {
            it->val = a.num() * rg;
        }
        if (ag != 1) {
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

void Matrix::eliminate_and_pivot(index_t i, index_t j, Integer &a_ij) {
    auto ib = rows_[i].cells.begin();
    auto ie = rows_[i].cells.end();
    auto &d_i = rows_[i].den;
    std::vector<size_t> sizes;
    sizes.resize(rows_[i].cells.size());
    std::vector<Cell> row;
    std::vector<index_t> col_buf;

    // step 1.1
    update_row(i, [&](index_t k, Integer &a_ik, Integer const &d_i) {
        static_cast<void>(d_i);
        if (k != j) {
            //a_ik /= -a_ij;
            a_ik *= -1;
        }
    });
    // step 2.2
    // a_ij = 1 / a_ij;
    a_ij.swap(d_i);  // TODO: setting this might allow for simplifying the row
                     //       we can compute the gcd above
                     //       to see if the denominator can be made smaller
    d_i *= a_ij;

    // Note that insertions into rows and columns do not invert iterators:
    // - row i is unaffected because k != i
    // - there are no insertions in column j because each a_kj != 0
    update_col(j, [&](index_t k, Integer const &a_kj, Integer &d_k) {
        if (k != i) {
            // TODO: multiply the respective values below with gd_i and gd_k.
            auto g = gcd(d_i, d_k);
            auto gd_i = d_i / g;
            auto gd_k = d_k / g;
            d_k *= gd_i;
            for (auto it = ib, jt = rows_[k].cells.begin(), je = rows_[k].cells.end(); it != ie || jt != je; ) {
                // case A_ix != 0 and A_kx == 0 (step 1.2)
                if (jt == je || (it != ie && it->col < jt->col)) {
                    // add A_kj * A_ix for x != j
                    row.emplace_back(it->col, it->val * a_kj);
                    // Note that vectors will be sorted at the end.
                    cols_[it->col].emplace_back(k);
                    ++sizes[it - ib];
                    ++it;
                }
                // case A_ix == 0 and A_kx != 0 (step 1.2)
                else if (it == ie || jt->col < it->col) {
                    // add A_kx for x != j
                    row.emplace_back(std::move(*jt));
                    ++jt;
                }
                // case A_ix != 0 and A_kx != 0
                else {
                    // case x != j (step 1.2)
                    if (jt->col != j) {
                        // add A_kx + A_kj * A_ix for x != j
                        row.emplace_back(jt->col, std::move(jt->val));
                        row.back().val += it->val * a_kj;
                        if (row.back().val == 0) {
                            row.pop_back();
                        }
                    }
                    // case x == j (step 2.1)
                    else {
                        // pivot setting A_kj to A_kj/a_ij (step 2.1)
                        row.emplace_back(jt->col, jt->val * it->val);
                    }
                    ++it;
                    ++jt;
                }
            }
            std::swap(rows_[k].cells, row);
            row.clear();
        }
    });

    // Ensure that column vectors are sorted.
    //
    // Note that we cannot assume that elements are unique because of the
    // lazy deletion scheme. Hence, it is not possible to use
    // std::merge_inplace.
    auto jt = sizes.begin();
    for (auto it = ib; it != ie; ++it, ++jt) {
        if (*jt == 0) {
            continue;
        }
        auto &col = cols_[it->col];
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
