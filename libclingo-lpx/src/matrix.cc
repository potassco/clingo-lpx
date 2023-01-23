#include "matrix.hh"

#include <numeric>

Number const &Matrix::get(index_t i, index_t j) const {
    if (i < rows_.size()) {
        auto const &row = rows_[i];
        auto it = std::lower_bound(row.begin(), row.end(), j);
        if (it != row.end() && it->col == j) {
            return it->val;
        }
    }
    return zero_();
}

Number &Matrix::unsafe_get(index_t i, index_t j) {
    return std::lower_bound(rows_[i].begin(), rows_[i].end(), j)->val;
}

void Matrix::set(index_t i, index_t j, Number const &a) {
    if (a == 0) {
        if (i < rows_.size()) {
            auto &row = rows_[i];
            auto it = std::lower_bound(row.begin(), row.end(), j);
            if (it != row.end() && it->col == j) {
                row.erase(it);
            }
        }
    }
    else {
        auto &row = reserve_row_(i);
        auto it = std::lower_bound(row.begin(), row.end(), j);
        if (it == row.end() || it->col != j) {
            row.emplace(it, j, a);
        }
        else {
            it->val = a;
        }
        auto &col = reserve_col_(j);
        auto jt = std::lower_bound(col.begin(), col.end(), i);
        if (jt == col.end() || *jt != i) {
            col.emplace(jt, i);
        }
    }
}

void Matrix::eliminate_and_pivot(index_t i, index_t j, Number &a_ij) {
    auto ib = rows_[i].begin();
    auto ie = rows_[i].end();
    std::vector<size_t> sizes;
    sizes.resize(rows_[i].size());
    std::vector<Cell> row;
    std::vector<index_t> col_buf;

    // step 1.1
    update_row(i, [&](index_t k, Number &a_ik) {
        if (k != j) {
            a_ik /= -a_ij;
        }
    });
    // step 2.2
    a_ij = 1 / a_ij;

    // Note that insertions into rows and columns do not invert iterators:
    // - row i is unaffected because k != i
    // - there are no insertions in column j because each a_kj != 0
    update_col(j, [&](index_t k, Number const &a_kj) {
        if (k != i) {
            for (auto it = ib, jt = rows_[k].begin(), je = rows_[k].end(); it != ie || jt != je; ) {
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
            std::swap(rows_[k], row);
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
                           [](size_t n, auto const &row) { return n + row.size(); });
}

bool Matrix::empty() const {
    return std::all_of(rows_.cbegin(), rows_.cend(),
                       [](auto const &row) { return row.empty(); });
}

void Matrix::clear() {
    rows_.clear();
    cols_.clear();
}

std::vector<Matrix::Cell> &Matrix::reserve_row_(index_t i) {
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
