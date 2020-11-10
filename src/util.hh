#pragma once

#include <unordered_map>
#include <vector>
#include <cstdint>
#include <cassert>
#include <gmpxx.h>

#define CLINGOLP_EXTRA_DEBUG
#ifdef CLINGOLP_EXTRA_DEBUG
#   define assert_extra(X) assert(X)
#else
#   define assert_extra(X)
#endif

using Number = mpq_class;
using index_t = uint32_t;

//! A sparse matrix with efficient access to both rows and columns.
//!
//! Values set to zero are deleted lazily from adjacency lists when traversing
//! rows and columns.
//!
//! Note: that this deletion scheme has not been tested in practice at all. I
//! could imagine that an additional cleanup step called at strategic points is
//! much better than the current scheme because this would totally eliminate
//! the necessity to tag the indexes.
class Tableau {
private:
    static constexpr index_t mask_get = 0x7fffffff;
    static constexpr index_t mask_set = 0x80000000;
    static constexpr index_t width = 32;
    class Index {
    public:
        Index(index_t i, index_t j)
        : row_{i}
        , col_{j} {
            if ((i & mask_set) != 0 || (j & mask_set) != 0) {
                std::abort();
            }
        }
        [[nodiscard]] index_t row() const {
            return row_ & mask_get;
        }
        [[nodiscard]] index_t col() const {
            return col_ & mask_get;
        }
        [[nodiscard]] bool in_row() const {
            return (row_ & mask_set) != 0;
        }
        bool in_row(bool x) const {
            auto ret = in_row();
            row_ = x ? (row_ | mask_set) : row();
            return ret;
        }
        [[nodiscard]] bool in_col() const {
            return (col_ & mask_set) != 0;
        }
        bool in_col(bool x) const {
            auto ret = in_col();
            col_ = x ? (col_ | mask_set) : col();
            return ret;
        }
    private:
        mutable index_t row_;
        mutable index_t col_;
    };
    struct IndexPred {
        [[nodiscard]] bool operator()(Index const &a, Index const &b) const {
            return a.row() == b.row() && a.col() == b.col();
        }
    };
    struct IndexHash {
        [[nodiscard]] size_t operator()(Index const &a) const {
            return std::hash<uint64_t>{}(a.row() + (static_cast<uint64_t>(a.col()) << width));
        }
    };
    void reserve_row_(index_t i) {
        if (rows_.size() <= i) {
            rows_.resize(i + 1);
        }
    }
    void reserve_col_(index_t j) {
        if (cols_.size() <= j) {
            cols_.resize(j + 1);
        }
    }
    static Number const &zero_() {
        static Number zero{0};
        return zero;
    }

public:
    //! Get value at row `i` and column `j`.
    [[nodiscard]] Number const &get(index_t i, index_t j) const {
        auto it = vals_.find({i, j});
        if (it != vals_.end()) {
            return it->second;
        }
        return zero_();
    }

    //! Set value `a` at row `i` and column `j`.
    void set(index_t i, index_t j, Number const &a) {
        if (a == 0) {
            auto it = vals_.find({i, j});
            if (it != vals_.end()) {
                it->second = 0;
            }
        }
        else {
            auto it = vals_.insert_or_assign(Index{i, j}, a).first;
            if (!it->first.in_row(true)) {
                reserve_row_(i);
                rows_[i].push_back(j);
            }
            if (!it->first.in_col(true)) {
                reserve_col_(j);
                cols_[j].push_back(i);
            }
        }
    }

    template <typename F>
    void update(index_t i, index_t j, F &&f) {
        auto [it, res] = vals_.try_emplace({i, j});
        f(it->second);
        if (it->second == 0) {
            if (res) {
                vals_.erase(it);
            }
        }
        else {
            if (!it->first.in_row(true)) {
                reserve_row_(i);
                rows_[i].push_back(j);
            }
            if (!it->first.in_col(true)) {
                reserve_col_(j);
                cols_[j].push_back(i);
            }
        }
    }
    //! Traverse non-zero elements in a row and possibly update them.
    template <typename F>
    void update_row(index_t i, F &&f) {
        if (i < rows_.size()) {
            auto &row = rows_[i];
            auto it = row.begin();
            auto ie = row.end();
            for (auto jt = it; jt != ie; ++jt) {
                auto kt = vals_.find({i, *jt});
                if (kt->second != 0) {
                    f(*jt, kt->second);
                }
                if (kt->second == 0) {
                    assert(kt->first.in_row());
                    kt->first.in_row(false);
                    if (!kt->first.in_col()) {
                        vals_.erase(kt);
                    }
                }
                else {
                    if (it != jt) {
                        std::iter_swap(it, jt);
                    }
                    ++it;
                }
            }
            row.erase(it, ie);
        }
    }

    //! Traverse non-zero elements in a row and possibly update them.
    template <typename F>
    void update_col(index_t j, F &&f) {
        if (j < cols_.size()) {
            auto &col = cols_[j];
            auto it = col.begin();
            auto ie = col.end();
            for (auto jt = it; jt != ie; ++jt) {
                auto kt = vals_.find({*jt, j});
                if (kt->second != 0) {
                    f(*jt, kt->second);
                }
                if (kt->second == 0) {
                    assert(kt->first.in_col());
                    kt->first.in_col(false);
                    if (!kt->first.in_row()) {
                        vals_.erase(kt);
                    }
                }
                else {
                    if (it != jt) {
                        std::iter_swap(it, jt);
                    }
                    ++it;
                }
            }
            col.erase(it, ie);
        }
    }

    //! Get the number of values in the matrix including zero values that have
    //! not yet been deleted.
    [[nodiscard]] size_t size() const {
        return vals_.size();
    }

    //! Equivalent to `size() == 0`.
    [[nodiscard]] bool empty() const {
        return vals_.empty();
    }

    //! Clear the tableau.
    void clear() {
        rows_.clear();
        cols_.clear();
        vals_.clear();
    }

private:
    std::unordered_map<Index, Number, IndexHash, IndexPred> vals_;
    std::vector<std::vector<index_t>> rows_;
    std::vector<std::vector<index_t>> cols_;
};
