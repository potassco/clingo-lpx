#pragma once

#include <numeric>
#include <stdexcept>
#include <unordered_map>
#include <vector>
#include <cstdint>
#include <cassert>
#include <iostream>
#include <memory>
#include <algorithm>
#include <limits>
#if defined(CLINGOLPX_USE_FLINT)
#include <flint/fmpq.h>
#elif defined(CLINGOLPX_USE_IMATH)
#include <imrat.h>
#else
#include <gmpxx.h>
#endif

#ifdef CLINGOLPX_CROSSCHECK
#   define assert_extra(X) assert(X) // NOLINT
#else
#   define assert_extra(X) // NOLINT
#endif

using index_t = uint32_t;

#if defined(CLINGOLPX_USE_FLINT)

constexpr int BASE = 10;

class Number {
public:
    Number() noexcept { // NOLINT
        fmpq_init(&num_);
    }
    Number(slong val)
    : Number() {
        fmpq_set_si(&num_, val, 1);
    }
    Number(char const *val, int radix)
    : Number() {
        fmpq_set_str(&num_, val, radix);
    }
    Number(std::string const &val, int radix)
    : Number(val.c_str(), radix) {
    }
    Number(Number const &a)
    : Number() {
        fmpq_set(&num_, &a.num_);
    }
    Number(Number &&a) noexcept
    : Number() {
        swap(a);
    }
    Number &operator=(Number const &a) {
        fmpq_set(&num_, &a.num_);
        return *this;
    }
    Number &operator=(Number &&a) noexcept {
        swap(a);
        return *this;
    }
    ~Number() noexcept {
        fmpq_clear(&num_);
    }

    void swap(Number &x) {
        fmpq_swap(&num_, &x.num_);
    }

    void canonicalize() {
        fmpq_canonicalise(&num_);
    }

    friend Number operator*(Number const &a, Number const &b) {
        Number c;
        fmpq_mul(&c.num_, &a.num_, &b.num_);
        return c;
    }
    friend Number &operator*=(Number &a, Number const &b) {
        fmpq_mul(&a.num_, &a.num_, &b.num_);
        return a;
    }

    friend Number operator/(Number const &a, Number const &b) {
        Number c;
        fmpq_div(&c.num_, &a.num_, &b.num_);
        return c;
    }
    friend Number &operator/=(Number &a, Number const &b) {
        fmpq_div(&a.num_, &a.num_, &b.num_);
        return a;
    }

    friend Number operator+(Number const &a, Number const &b) {
        Number c;
        fmpq_add(&c.num_, &a.num_, &b.num_);
        return c;
    }
    friend Number &operator+=(Number &a, Number const &b) {
        fmpq_add(&a.num_, &a.num_, &b.num_);
        return a;
    }

    friend Number operator-(Number const &a) {
        Number c;
        fmpq_neg(&c.num_, &a.num_);
        return c;
    }
    friend Number operator-(Number const &a, Number const &b) {
        Number c;
        fmpq_sub(&c.num_, &a.num_, &b.num_);
        return c;
    }
    friend Number &operator-=(Number &a, Number const &b) {
        fmpq_sub(&a.num_, &a.num_, &b.num_);
        return a;
    }

    friend bool operator<(Number const &a, Number const &b) {
        return compare(a, b) < 0;
    }
    friend bool operator<=(Number const &a, Number const &b) {
        return compare(a, b) <= 0;
    }
    friend bool operator>(Number const &a, Number const &b) {
        return compare(a, b) > 0;
    }
    friend bool operator>=(Number const &a, Number const &b) {
        return compare(a, b) >= 0;
    }
    friend bool operator==(Number const &a, Number const &b) {
        return fmpq_equal(&a.num_, &b.num_) != 0;
    }
    friend bool operator!=(Number const &a, Number const &b) {
        return fmpq_equal(&a.num_, &b.num_) == 0;
    }
    friend bool operator<(Number const &a, slong b) {
        return fmpq_cmp_si(&a.num_, b) < 0;
    }
    friend bool operator<=(Number const &a, slong b) {
        return fmpq_cmp_si(&a.num_, b) <= 0;
    }
    friend bool operator>(Number const &a, slong b) {
        return fmpq_cmp_si(&a.num_, b) > 0;
    }
    friend bool operator>=(Number const &a, slong b) {
        return fmpq_cmp_si(&a.num_, b) >= 0;
    }
    friend bool operator==(Number const &a, slong b) {
        return fmpq_cmp_si(&a.num_, b) == 0;
    }
    friend bool operator!=(Number const &a, slong b) {
        return fmpq_cmp_si(&a.num_, b) != 0;
    }

    friend int compare(Number const &a, Number const &b) {
        return fmpq_cmp(&a.num_, &b.num_);
    }

    friend std::ostream &operator<<(std::ostream &out, Number const &a) {
        // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
        std::unique_ptr<char, decltype(std::free) *> buf{fmpq_get_str(nullptr, 10, &a.num_), std::free};
        out << buf.get();
        return out;
    }

private:
    /*
    static void handle_error_(mp_result res) {
        if (res != MP_OK) {
            if (res == MP_MEMORY) {
                throw std::bad_alloc();
            }
            if (res == MP_RANGE || res == MP_TRUNC) {
                throw std::range_error(mp_error_string(res));
            }
            if (res == MP_UNDEF) {
                throw std::domain_error(mp_error_string(res));
            }
            if (res == MP_BADARG) {
                throw std::invalid_argument(mp_error_string(res));
            }
            throw std::logic_error(mp_error_string(res));
        }
    }
    */

    mutable fmpq num_;
};

#elif defined(CLINGOLPX_USE_IMATH)

class Number {
public:
    Number() { // NOLINT
        handle_error_(mp_rat_init(&num_));
    }
    Number(mp_small val)
    : Number() {
        handle_error_(mp_rat_set_value(&num_, val, 1));
    }
    Number(char const *val, mp_size radix)
    : Number() {
        handle_error_(mp_rat_read_string(&num_, radix, val));
    }
    Number(std::string const &val, mp_size radix)
    : Number(val.c_str(), radix) {
    }
    Number(Number const &a)
    : Number() {
        handle_error_(mp_rat_copy(&a.num_, &num_));
    }
    Number(Number &&a) noexcept
    : Number() {
        swap(a);
    }
    Number &operator=(Number const &a) {
        handle_error_(mp_rat_copy(&a.num_, &num_));
        return *this;
    }
    Number &operator=(Number &&a) noexcept {
        swap(a);
        return *this;
    }
    ~Number() {
        mp_rat_clear(&num_);
    }

    void swap(Number &x) {
        mp_int_swap(mp_rat_numer_ref(&num_), mp_rat_numer_ref(&x.num_));
        mp_int_swap(mp_rat_denom_ref(&num_), mp_rat_denom_ref(&x.num_));
    }

    void canonicalize() {
        handle_error_(mp_rat_reduce(&num_));
    }

    friend Number operator*(Number const &a, Number const &b) {
        Number c;
        handle_error_(mp_rat_mul(&a.num_, &b.num_, &c.num_));
        return c;
    }

    friend Number &operator*=(Number &a, Number const &b) {
        Number c;
        handle_error_(mp_rat_mul(&a.num_, &b.num_, &c.num_));
        c.swap(a);
        return a;
    }

    friend Number operator/(Number const &a, Number const &b) {
        Number c;
        handle_error_(mp_rat_div(&a.num_, &b.num_, &c.num_));
        return c;
    }

    friend Number &operator/=(Number &a, Number const &b) {
        handle_error_(mp_rat_div(&a.num_, &b.num_, &a.num_));
        return a;
    }

    friend Number operator+(Number const &a, Number const &b) {
        Number c;
        handle_error_(mp_rat_add(&a.num_, &b.num_, &c.num_));
        return c;
    }
    friend Number &operator+=(Number &a, Number const &b) {
        Number c;
        handle_error_(mp_rat_add(&a.num_, &b.num_, &c.num_));
        c.swap(a);
        return a;
    }

    friend Number operator-(Number const &a) {
        Number b;
        handle_error_(mp_rat_neg(&a.num_, &b.num_));
        return b;
    }
    friend Number operator-(Number const &a, Number const &b) {
        Number c;
        handle_error_(mp_rat_sub(&a.num_, &b.num_, &c.num_));
        return c;
    }
    friend Number &operator-=(Number &a, Number const &b) {
        Number c;
        handle_error_(mp_rat_sub(&a.num_, &b.num_, &c.num_));
        c.swap(a);
        return a;
    }

    friend bool operator<(Number const &a, Number const &b) {
        return mp_rat_compare(&a.num_, &b.num_) < 0;
    }
    friend bool operator<=(Number const &a, Number const &b) {
        return mp_rat_compare(&a.num_, &b.num_) <= 0;
    }
    friend bool operator>(Number const &a, Number const &b) {
        return mp_rat_compare(&a.num_, &b.num_) > 0;
    }
    friend bool operator>=(Number const &a, Number const &b) {
        return mp_rat_compare(&a.num_, &b.num_) >= 0;
    }
    friend bool operator==(Number const &a, Number const &b) {
        return mp_rat_compare(&a.num_, &b.num_) == 0;
    }
    friend bool operator!=(Number const &a, Number const &b) {
        return mp_rat_compare(&a.num_, &b.num_) != 0;
    }
    friend bool operator<(Number const &a, mp_small b) {
        return mp_rat_compare_value(&a.num_, b, 1) < 0;
    }
    friend bool operator<=(Number const &a, mp_small b) {
        return mp_rat_compare_value(&a.num_, b, 1) <= 0;
    }
    friend bool operator>(Number const &a, mp_small b) {
        return mp_rat_compare_value(&a.num_, b, 1) > 0;
    }
    friend bool operator>=(Number const &a, mp_small b) {
        return mp_rat_compare_value(&a.num_, b, 1) >= 0;
    }
    friend bool operator==(Number const &a, mp_small b) {
        return mp_rat_compare_value(&a.num_, b, 1) == 0;
    }
    friend bool operator!=(Number const &a, mp_small b) {
        return mp_rat_compare_value(&a.num_, b, 1) != 0;
    }

    friend int compare(Number const &a, Number const &b) {
        return mp_rat_compare(&a.num_, &b.num_);
    }

    friend std::ostream &operator<<(std::ostream &out, Number const &a) {
        constexpr int radix = 10;
        if (mp_int_compare_value(mp_rat_denom_ref(&a.num_), 1) == 0) {
            auto len = mp_int_string_len(mp_rat_numer_ref(&a.num_), radix);
            auto buf = std::make_unique<char[]>(len); // NOLINT
            handle_error_(mp_int_to_string(mp_rat_numer_ref(&a.num_), radix, buf.get(), len));
            out << buf.get();
        }
        else {
            auto len = mp_rat_string_len(&a.num_, radix);
            auto buf = std::make_unique<char[]>(len); // NOLINT
            handle_error_(mp_rat_to_string(&a.num_, radix, buf.get(), len));
            out << buf.get();
        }
        return out;
    }

private:
    static void handle_error_(mp_result res) {
        if (res != MP_OK) {
            if (res == MP_MEMORY) {
                throw std::bad_alloc();
            }
            if (res == MP_RANGE || res == MP_TRUNC) {
                throw std::range_error(mp_error_string(res));
            }
            if (res == MP_UNDEF) {
                throw std::domain_error(mp_error_string(res));
            }
            if (res == MP_BADARG) {
                throw std::invalid_argument(mp_error_string(res));
            }
            throw std::logic_error(mp_error_string(res));
        }
    }

    mutable mpq_t num_;
};

#else

using Number = mpq_class;

inline int compare(Number const &a, Number const &b) {
    return mpq_cmp(a.get_mpq_t(), b.get_mpq_t());
}

#endif

namespace detail {

template <int X>
using int_type = std::integral_constant<int, X>;
template <class T, class S>
inline void sc_check(S s, int_type<0> t) { // same sign
    static_cast<void>(t);
    if (!std::is_same<T, S>::value && (s < std::numeric_limits<T>::min() || s > std::numeric_limits<T>::max())) {
        throw std::overflow_error("safe cast failed");
    }
}
template <class T, class S>
inline void sc_check(S s, int_type<-1> t) { // Signed -> Unsigned
    static_cast<void>(t);
    if (s < 0 || static_cast<S>(static_cast<T>(s)) != s) {
        throw std::overflow_error("safe cast failed");
    }
}
template <class T, class S>
inline void sc_check(S s, int_type<1> t) { // Unsigned -> Signed
    static_cast<void>(t);
    if (s > static_cast<typename std::make_unsigned<T>::type>(std::numeric_limits<T>::max())) {
        throw std::overflow_error("safe cast failed");
    }
}

} // namespace detail

//! A safe numeric cast raising an exception if the target type cannot hold the value.
template <class T, class S>
inline T safe_cast(S s) {
    constexpr int sv = static_cast<int>(std::numeric_limits<T>::is_signed) - static_cast<int>(std::numeric_limits<S>::is_signed);
    detail::sc_check<T>(s, detail::int_type<sv>());
    return static_cast<T>(s);
}

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
class Tableau {
public:
    //! Return a const reference to A_ij.
    //!
    //! Runs in O(log(n)).
    [[nodiscard]] Number const &get(index_t i, index_t j) const {
        if (i < rows_.size()) {
            auto const &row = rows_[i];
            auto it = std::lower_bound(row.begin(), row.end(), j);
            if (it != row.end() && it->col == j) {
                return it->val;
            }
        }
        return zero_();
    }

    //! Return a mutable reference to A_ij assuming that A_ij != 0.
    //!
    //! Only non-zero values may be accessed and they must not be set to zero.
    //!
    //! Runs in O(log(n)).
    [[nodiscard]] Number &unsafe_get(index_t i, index_t j) {
        return std::lower_bound(rows_[i].begin(), rows_[i].end(), j)->val;
    }

    //! Set A_ij to value a.
    //!
    //! Setting an element to zero removes it from the matrix.
    //!
    //! Runs in O(m + n).
    void set(index_t i, index_t j, Number const &a) {
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
            for (auto &[col, val] : rows_[i]) {
                f(static_cast<index_t>(col), val);
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
                auto kt = std::lower_bound(row.begin(), row.end(), j);
                if (kt != row.end() && kt->col == j) {
                    f(i, kt->val);
                    if (it != jt) {
                        std::iter_swap(it, jt);
                    }
                    ++it;
                }
            }
            col.erase(it, ie);
        }
    }

    //! Replace all rows A_k with k != i by A_kj * A_i - A_ij * A_k.
    //!
    //! This is the only function specific to the simplex algorithm. It is
    //! implemented here to offer better performance.
    //!
    //! Runs in O(m*m*n). The hope is that in practice this is rather
    //! O(m*log(m)*n). A tighter bound could be guaranteed by delaying
    //! insertions into columns and merging them at the end using
    //! std::inplace_merge.
    void eliminate(index_t i, index_t j) {
        auto ib = rows_[i].begin();
        auto ie = rows_[i].end();
        std::vector<Cell> row;
        update_col(j, [&](index_t k, Number const &a_kj) {
            if (k != i) {
                // Note that this call does not invalidate active iterators:
                // - row i is unaffected because k != i
                // - there are no insertions in column j because each a_kj != 0
                for (auto it = ib, jt = rows_[k].begin(), je = rows_[k].end(); it != ie || jt != je; ) {
                    if (jt == je || (it != ie && it->col < jt->col)) {
                        row.emplace_back(it->col, it->val * a_kj);
                        auto &col = cols_[it->col];
                        auto kt = std::lower_bound(col.begin(), col.end(), k);
                        if (kt == col.end() || *kt != k) {
                            col.emplace(kt, k);
                        }
                        ++it;
                    }
                    else if (it == ie || jt->col < it->col) {
                        row.emplace_back(std::move(*jt));
                        ++jt;
                    }
                    else {
                        if (jt->col != j) {
                            row.emplace_back(jt->col, std::move(jt->val));
                            row.back().val += it->val * a_kj;
                            if (row.back().val == 0) {
                                row.pop_back();
                            }
                        }
                        else {
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
    }

    //! Get the number of non-zero elements in the matrix.
    //!
    //! The runtime of this function is linear in the size of the matrix.
    [[nodiscard]] size_t size() const {
        return std::accumulate(rows_.begin(), rows_.end(), static_cast<size_t>(0),
                               [](size_t n, auto const &row) { return n + row.size(); });
    }

    //! Equivalent to `size() == 0`.
    [[nodiscard]] bool empty() const {
        return std::all_of(rows_.cbegin(), rows_.cend(),
                           [](auto const &row) { return row.empty(); });
    }

    //! Set all elements to zero.
    void clear() {
        rows_.clear();
        cols_.clear();
    }

private:
    struct Cell {
        Cell(index_t col, Number val)
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
        Number val;
    };

    std::vector<Cell> &reserve_row_(index_t i) {
        if (rows_.size() <= i) {
            rows_.resize(i + 1);
        }
        return rows_[i];
    }
    std::vector<index_t> &reserve_col_(index_t j) {
        if (cols_.size() <= j) {
            cols_.resize(j + 1);
        }
        return cols_[j];
    }
    static Number const &zero_() {
        static Number zero{0};
        return zero;
    }

    std::vector<std::vector<Cell>> rows_;
    std::vector<std::vector<index_t>> cols_;
};

class NumberQ {
private:
    friend NumberQ operator+(NumberQ const &q, Number const &c);
    friend NumberQ operator+(Number  const &c, NumberQ const &q);
    friend NumberQ operator+(NumberQ const &p, NumberQ const &q);
    friend NumberQ operator-(NumberQ const &q, Number const &c);
    friend NumberQ operator-(Number  const &c, NumberQ const &q);
    friend NumberQ operator-(NumberQ const &p, NumberQ const &q);
    friend NumberQ operator*(NumberQ const &q, Number const &c);
    friend NumberQ operator*(Number  const &c, NumberQ const &q);
    friend NumberQ operator/(NumberQ const &q, Number const &c);
    friend std::ostream &operator<<(std::ostream &out, NumberQ const &q);

public:
    explicit NumberQ(Number c = Number{}, Number k = Number{})
    : c_{std::move(c)}
    , k_{std::move(k)} { }
    NumberQ(NumberQ const &) = default;
    NumberQ(NumberQ &&) = default;
    NumberQ &operator=(NumberQ const &) = default;
    NumberQ &operator=(NumberQ &&) = default;
    ~NumberQ() = default;

    void swap(NumberQ &q) {
        c_.swap(q.c_);
        k_.swap(q.k_);
    }

    // addition
    NumberQ &operator+=(Number const &c) {
        c_ += c;
        return *this;
    }

    NumberQ &operator+=(NumberQ const &q) {
        c_ += q.c_;
        k_ += q.k_;
        return *this;
    }

    NumberQ &operator-=(Number const &c) {
        c_ -= c;
        return *this;
    }

    NumberQ &operator-=(NumberQ const &q) {
        c_ -= q.c_;
        k_ -= q.k_;
        return *this;
    }

    NumberQ &operator*=(Number const &c) {
        c_ *= c;
        k_ *= c;
        return *this;
    }

    NumberQ &operator/=(Number const &c) {
        c_ /= c;
        k_ /= c;
        return *this;
    }

    [[nodiscard]] bool operator<(Number const &c) const {
        return cmp_(c) < 0;
    }

    [[nodiscard]] bool operator<(NumberQ const &q) const {
        return cmp_(q) < 0;
    }

    [[nodiscard]] bool operator<=(Number const &c) const {
        return cmp_(c) <= 0;
    }

    [[nodiscard]] bool operator<=(NumberQ const &q) const {
        return cmp_(q) <= 0;
    }

    [[nodiscard]] bool operator>(Number const &c) const {
        return cmp_(c) > 0;
    }

    [[nodiscard]] bool operator>(NumberQ const &q) const {
        return cmp_(q) > 0;
    }

    [[nodiscard]] bool operator>=(Number const &c) const {
        return cmp_(c) >= 0;
    }

    [[nodiscard]] bool operator>=(NumberQ const &q) const {
        return cmp_(q) >= 0;
    }

    [[nodiscard]] bool operator==(NumberQ const &q) const {
        return c_ == q.c_ && k_ == q.k_;
    }

    [[nodiscard]] bool operator==(Number const &c) const {
        return c_ == c && k_ == 0;
    }

    [[nodiscard]] bool operator!=(NumberQ const &q) const {
        return c_ != q.c_ || k_ != q.k_;
    }

    [[nodiscard]] bool operator!=(Number const &c) const {
        return c_ != c || k_ != 0;
    }

private:
    [[nodiscard]] int cmp_(NumberQ const &q) const {
        auto ret = compare(c_, q.c_);
        if (ret != 0) {
            return ret;
        }
        return compare(k_, q.k_);
    }
    [[nodiscard]] int cmp_(Number const &c) const {
        auto ret = compare(c_, c);
        if (ret != 0) {
            return ret;
        }
        if (k_ < 0) {
            return -1;
        }
        if (k_ > 0) {
            return 1;
        }
        return 0;
    }

    Number c_;
    Number k_;
};

// addition

[[nodiscard]] inline NumberQ operator+(NumberQ const &q, Number const &c) {
    return NumberQ{q.c_ + c, q.k_};
}

[[nodiscard]] inline NumberQ operator+(Number  const &c, NumberQ const &q) {
    return NumberQ{c + q.c_, q.k_};
}

[[nodiscard]] inline NumberQ operator+(NumberQ const &p, NumberQ const &q) {
    return NumberQ{p.c_ + q.c_, p.k_ + q.k_};
}

// subtraction

[[nodiscard]] inline NumberQ operator-(NumberQ const &q, Number const &c) {
    return NumberQ{q.c_ - c, q.k_};
}

[[nodiscard]] inline NumberQ operator-(Number  const &c, NumberQ const &q) {
    return NumberQ{c - q.c_, -q.k_};
}

[[nodiscard]] inline NumberQ operator-(NumberQ const &p, NumberQ const &q) {
    return NumberQ{p.c_ - q.c_, p.k_ - q.k_};
}

// multiplication

[[nodiscard]] inline NumberQ operator*(NumberQ const &q, Number const &c) {
    return NumberQ{q.c_ * c, q.k_ * c};
}

[[nodiscard]] inline NumberQ operator*(Number const &c, NumberQ const &q) {
    return NumberQ{c * q.c_, c * q.k_};
}

// division

[[nodiscard]] inline NumberQ operator/(NumberQ const &q, Number const &c) {
    return NumberQ{q.c_ / c, q.k_ / c};
}

inline std::ostream &operator<<(std::ostream &out, NumberQ const &q) {
    if (q.c_ != 0 || q.k_ == 0) {
        out << q.c_;
    }
    if (q.k_ != 0) {
        if (q.c_ != 0) {
            out << "+";
        }
        if (q.k_ != 1) {
            out << q.k_ << "*";
        }
        out << "e";
    }
    return out;
}
