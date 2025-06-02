#pragma once

#include <algorithm>
#include <iterator>
#include <type_traits>
#include <vector>

#include "number.hh"

//! Type used for array indices.
using index_t = uint32_t;

//! A sparse tableau with efficient access to both rows and columns.
//!
//! Insertion into the tableau is linear in the number of rows/columns and
//! should be avoided. Runtime complexities are sometimes amortized without
//! further comments. Algorithms are generally faster the sparser the tableau.
//!
//! It would also be possible to additionally keep track of non-zero elements
//! using a hash table. Like this, logarithm factors could be removed at the
//! expense of additional storage requirements.
//!
//! In the documentation below, we use A to refer to a tableau with m rows and n
//! columns. Furthemore, we use the following common ways to work with the
//! tableau:
//! - A_i is the i-th row,
//! - A_ij is the element at row i and column j, and
//! - A^T is the transposed tableau.
class Tableau {
  public:
    //! Return a const reference to A_ij.
    //!
    //! Runs in O(log(n)).
    [[nodiscard]] auto get(index_t i, index_t j) const -> Rational;

    //! Return a mutable reference to A_ij assuming that A_ij != 0.
    //!
    //! Only non-zero values may be accessed and they must not be set to zero.
    //!
    //! Runs in O(log(n)).
    void unsafe_get(index_t i, index_t j, Integer *&num, Integer *&den);

    //! Set A_ij to value a.
    //!
    //! Setting an element to zero removes it from the tableau.
    //!
    //! Runs in O(m + n).
    void set(index_t i, index_t j, Rational const &a);

    //! Call f(j, a_ij) for each element a_ij != 0 in row A_i.
    //!
    //! Function f can change the value a_ij but must not set it to zero. While
    //! it would be possible to handle this case, this functionality is simply
    //! not required by a simplex algorithm.
    //!
    //! Runs in O(n).
    template <typename F> void update_row(index_t i, F &&f) {
        if (i < rows_.size()) {
            auto &row = rows_[i];
            for (auto &[col, val] : row.cells) {
                f(static_cast<index_t>(col), val, row.den);
            }
        }
    }

    //! Call f(i, a_ij) for each element a_ij != 0 in column A^T_j.
    //!
    //! The same remark as for update_row() applies.
    //!
    //! Runs in O(m*log(n)).
    template <typename F> void update_col(index_t j, F &&f) {
        if (j < cols_.size()) {
            auto &col = cols_[j];
            auto it = col.begin();
            auto ie = col.end();
            for (auto jt = it; jt != ie; ++jt) {
                auto i = *jt;
                auto &row = rows_[i];
                auto kt = std::lower_bound(row.cells.begin(), row.cells.end(), j);
                if (kt != row.cells.end() && kt->col == j) {
                    f(i, kt->val, row.den);
                    if (it != jt) {
                        std::iter_swap(it, jt);
                    }
                    ++it;
                }
            }
            col.erase(it, ie);
        }
    }

    //! This functions pivots row i and column j.
    //!
    //! The two integers passed in are a reference to the cell a_ij and
    //! denominator d_j. See the description of the class for a more detailed
    //! description.
    //!
    //! Runs in O(m*m).
    void pivot(index_t i, index_t j, Integer &a_ij, Integer &d_i);

    //! Get the number of non-zero elements in the tableau.
    //!
    //! Runs in O(m).
    [[nodiscard]] auto size() const -> size_t;

    //! Equivalent to `size() == 0`.
    //!
    //! Runs in O(m).
    [[nodiscard]] auto empty() const -> bool;

    //! Set all elements to zero.
    //!
    //! Runs in O(1).
    void clear();

    //! Print tableau to stderr for debugging purposes.
    void debug(char const *indent) const;

  private:
    //! Simplify the given row.
    void simplify_(index_t i);

    struct Cell {
        Cell(index_t col, Integer val) : col{col}, val{std::move(val)} {}

        friend auto operator==(Cell const &x, Cell const &y) -> bool { return x.col == y.col && x.val == y.val; }
        friend auto operator<(Cell const &x, Cell const &y) -> bool { return x.col < y.col; }
        friend auto operator<(Cell const &x, index_t col) -> bool { return x.col < col; }
        friend auto operator<(index_t col, Cell const &x) -> bool { return col < x.col; }

        index_t col;
        Integer val;
    };
    struct Row {
        Integer den = 1;
        std::vector<Cell> cells;
    };

    auto reserve_row_(index_t i) -> Row &;
    auto reserve_col_(index_t j) -> std::vector<index_t> &;
    static auto zero_() -> Rational const &;

    std::vector<Row> rows_;
    std::vector<std::vector<index_t>> cols_;
};
