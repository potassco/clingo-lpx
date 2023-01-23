#pragma once

#include <iterator>
#include <type_traits>
#include <vector>
#include <algorithm>

#include "number.hh"

//! Type used for array indices.
using index_t = uint32_t;


//! A sparse matrix with efficient access to both rows and columns.
//!
//! Insertion into the matrix is linear in the number of rows/columns and
//! should be avoided. Runtime complexities are sometimes amortized without
//! further comments. Algorithms are generally faster the sparser the matrix.
//!
//! It would also be possible to additionally keep track of non-zero elements
//! using a hash table. Like this, logarithm factors could be removed at the
//! expense of additional storage requirements.
//!
//! In the documentation below, we use A to refer to a matrix with m rows and n
//! columns. Furthemore, we use the following common ways to work with the
//! matrix:
//! - A_i is the i-th row,
//! - A_ij is the element at row i and column j, and
//! - A^T is the transposed matrix.
class Matrix {
public:
    //! Return a const reference to A_ij.
    //!
    //! Runs in O(log(n)).
    [[nodiscard]] Number get(index_t i, index_t j) const;

    //! Return a mutable reference to A_ij assuming that A_ij != 0.
    //!
    //! Only non-zero values may be accessed and they must not be set to zero.
    //!
    //! Runs in O(log(n)).
    void unsafe_get(index_t i, index_t j, Integer *&num, Integer *&den);

    //! Set A_ij to value a.
    //!
    //! Setting an element to zero removes it from the matrix.
    //!
    //! Runs in O(m + n).
    void set(index_t i, index_t j, Number const &a);

    //! Call f(j, a_ij) for each element a_ij != 0 in row A_i.
    //!
    //! Function f can change the value a_ij but must not set it to zero. While
    //! it would be possible to handle this case, this functionality is simply
    //! not required by a simplex algorithm.
    //!
    //! Runs in O(n).
    template <typename F>
    void update_row(index_t i, F &&f) {
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
    template <typename F>
    void update_col(index_t j, F &&f) {
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

    //! This function eliminates elements a_kj for i != k and pivots i and j.
    //!
    //! This is the only function specific to the simplex algorithm. It is
    //! implemented here to offer better performance. It performs the following
    //! steps:
    //!
    //! 1. ignoring column j
    //!    1. replace row i by A_i/-A_ij
    //!    2. replace rows k != i by A_i*A_kj + A_k.
    //! 2. changing column j
    //!    1. devide rows k != i by -a_ij
    //!    2. replace row i by 1/a_ij
    //!
    //! Observe that if step 1. would not ignore column j, the column would be
    //! a unit vector.
    //!
    //! TODO: There is no need to store rational numbers in the matrix, we
    //! could also devide by a_ij when setting values for integer variables.
    //!
    //! Runs in O(m*m).
    void eliminate_and_pivot(index_t i, index_t j, Integer &a_ij);

    //! Get the number of non-zero elements in the matrix.
    //!
    //! The runtime of this function is linear in the size of the matrix.
    [[nodiscard]] size_t size() const;

    //! Equivalent to `size() == 0`.
    [[nodiscard]] bool empty() const;

    //! Set all elements to zero.
    void clear();

private:
    struct Cell {
        Cell(index_t col, Integer val)
        : col{col}
        , val{std::move(val)} { }

        friend bool operator==(Cell const &x, Cell const &y) {
            return x.col == y.col && x.val == y.val;
        }
        friend bool operator<(Cell const &x, Cell const &y) {
            return x.col < y.col;
        }
        friend bool operator<(Cell const &x, index_t col) {
            return x.col < col;
        }
        friend bool operator<(index_t col, Cell const &x) {
            return col < x.col;
        }

        index_t col;
        Integer val;
    };
    struct Row {
        Integer den = 1;
        std::vector<Cell> cells;
    };

    Row &reserve_row_(index_t i);
    std::vector<index_t> &reserve_col_(index_t j);
    static Number const &zero_();

    std::vector<Row> rows_;
    std::vector<std::vector<index_t>> cols_;
};

