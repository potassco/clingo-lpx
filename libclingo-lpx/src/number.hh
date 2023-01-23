#pragma once

#include <string>
#include <memory>
#include <iostream>

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
