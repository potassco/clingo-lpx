#pragma once

#include <ios>
#include <new>
#include <stdexcept>
#include <string>
#include <memory>
#include <iostream>

#if defined(CLINGOLPX_USE_FLINT)
#include <flint/fmpz.h>
#include <flint/fmpq.h>
#elif defined(CLINGOLPX_USE_IMATH)
#include <imrat.h>
#else
#include <gmpxx.h>
#endif

#if defined(CLINGOLPX_USE_FLINT)

constexpr int BASE = 10;

class Integer {
public:
    Integer() noexcept { // NOLINT
        fmpz_init(&num_);
    }
    Integer(slong val)
    : Integer() {
        fmpz_set_si(&num_, val);
    }
    Integer(char const *val, int radix)
    : Integer() {
        if (fmpz_set_str(&num_, val, radix) != 0) {
            throw std::runtime_error("could not parse number");
        }
    }
    Integer(std::string const &val, int radix)
    : Integer(val.c_str(), radix) {
    }
    Integer(Integer const &a)
    : Integer() {
        fmpz_set(&num_, &a.num_);
    }
    Integer(Integer &&a) noexcept
    : Integer() {
        swap(a);
    }
    Integer &operator=(Integer const &a) {
        fmpz_set(&num_, &a.num_);
        return *this;
    }
    Integer &operator=(Integer &&a) noexcept {
        swap(a);
        return *this;
    }
    ~Integer() noexcept {
        fmpz_clear(&num_);
    }

    void swap(Integer &x) {
        fmpz_swap(&num_, &x.num_);
    }

    Integer &add_mul(Integer const &a, Integer const &b) {
        fmpz_addmul(&num_, &a.num_, &b.num_);
        return *this;
    }

    Integer &neg() {
        fmpz_neg(&num_, &num_);
        return *this;
    }

    friend Integer operator*(Integer const &a, Integer const &b) {
        Integer c;
        fmpz_mul(&c.num_, &a.num_, &b.num_);
        return c;
    }
    friend Integer &operator*=(Integer &a, Integer const &b) {
        fmpz_mul(&a.num_, &a.num_, &b.num_);
        return a;
    }

    friend Integer operator/(Integer const &a, Integer const &b) {
        Integer c;
        fmpz_divexact(&c.num_, &a.num_, &b.num_);
        return c;
    }
    friend Integer &operator/=(Integer &a, Integer const &b) {
        fmpz_divexact(&a.num_, &a.num_, &b.num_);
        return a;
    }

    friend Integer operator+(Integer const &a, Integer const &b) {
        Integer c;
        fmpz_add(&c.num_, &a.num_, &b.num_);
        return c;
    }
    friend Integer &operator+=(Integer &a, Integer const &b) {
        fmpz_add(&a.num_, &a.num_, &b.num_);
        return a;
    }

    friend Integer operator-(Integer const &a) {
        Integer c;
        fmpz_neg(&c.num_, &a.num_);
        return c;
    }
    friend Integer operator-(Integer const &a, Integer const &b) {
        Integer c;
        fmpz_sub(&c.num_, &a.num_, &b.num_);
        return c;
    }
    friend Integer &operator-=(Integer &a, Integer const &b) {
        fmpz_sub(&a.num_, &a.num_, &b.num_);
        return a;
    }

    friend bool operator<(Integer const &a, Integer const &b) {
        return compare(a, b) < 0;
    }
    friend bool operator<=(Integer const &a, Integer const &b) {
        return compare(a, b) <= 0;
    }
    friend bool operator>(Integer const &a, Integer const &b) {
        return compare(a, b) > 0;
    }
    friend bool operator>=(Integer const &a, Integer const &b) {
        return compare(a, b) >= 0;
    }
    friend bool operator==(Integer const &a, Integer const &b) {
        return fmpz_equal(&a.num_, &b.num_) != 0;
    }
    friend bool operator!=(Integer const &a, Integer const &b) {
        return fmpz_equal(&a.num_, &b.num_) == 0;
    }
    friend bool operator<(Integer const &a, slong b) {
        return fmpz_cmp_si(&a.num_, b) < 0;
    }
    friend bool operator<=(Integer const &a, slong b) {
        return fmpz_cmp_si(&a.num_, b) <= 0;
    }
    friend bool operator>(Integer const &a, slong b) {
        return fmpz_cmp_si(&a.num_, b) > 0;
    }
    friend bool operator>=(Integer const &a, slong b) {
        return fmpz_cmp_si(&a.num_, b) >= 0;
    }
    friend bool operator==(Integer const &a, slong b) {
        return fmpz_equal_si(&a.num_, b) != 0;
    }
    friend bool operator!=(Integer const &a, slong b) {
        return fmpz_equal_si(&a.num_, b) == 0;
    }

    friend int compare(Integer const &a, Integer const &b) {
        return fmpz_cmp(&a.num_, &b.num_);
    }

    friend std::ostream &operator<<(std::ostream &out, Integer const &a) {
        std::unique_ptr<char, decltype(std::free) *> buf{fmpz_get_str(nullptr, BASE, &a.num_), std::free};
        if (buf == nullptr) {
            throw std::bad_alloc();
        }
        out << buf.get();
        return out;
    }

    friend std::tuple<Integer, Integer, Integer> gcd(Integer const &a, Integer const &b) {
        Integer g;
        Integer ga;
        Integer gb;
        fmpz_gcd(&g.num_, &a.num_, &b.num_);
        fmpz_divexact(&ga.num_, &a.num_, &g.num_);
        fmpz_divexact(&gb.num_, &b.num_, &g.num_);
        return {g, ga, gb};
    }

    fmpz &impl() const {
        return num_;
    }

private:

    mutable fmpz num_;
};

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
#if __FLINT_RELEASE >= 20600
        if (fmpq_set_str(&num_, val, radix) != 0) {
            throw std::runtime_error("could not parse number");
        }
#else
        std::string buf = val;
        auto pos = buf.find('/');
        if (pos == std::string::npos) {
            if (fmpz_set_str(&num_.num, buf.c_str(), radix) != 0) {
                throw std::runtime_error("could not parse number");
            }
            fmpz_set_si(&num_.den, 1);
        }
        else {
            if (fmpz_set_str(&num_.num, buf.substr(0, pos).c_str(), radix) != 0) {
                throw std::runtime_error("could not parse number");
            }
            if (fmpz_set_str(&num_.den, buf.substr(pos+1).c_str(), radix) != 0) {
                throw std::runtime_error("could not parse number");
            }
            fmpq_canonicalise(&num_);
        }
#endif
    }
    Number(std::string const &val, int radix)
    : Number(val.c_str(), radix) {
    }
    Number(Number const &a)
    : Number() {
        fmpq_set(&num_, &a.num_);
    }
    Number(Integer const &num, Integer const &den)
    : Number() {
        fmpz_set(&num_.num, &num.impl());
        fmpz_set(&num_.den, &den.impl());
        fmpq_canonicalise(&num_);
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

    Integer &num() {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        return reinterpret_cast<Integer&>(num_.num);
    }

    Integer const &num() const {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        return reinterpret_cast<Integer const&>(num_.num);
    }

    Integer &den() {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        return reinterpret_cast<Integer&>(num_.den);
    }

    Integer const &den() const {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        return reinterpret_cast<Integer const&>(num_.den);
    }

    Number &neg() {
        fmpq_neg(&num_, &num_);
        return *this;
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

    friend Number operator*(Number const &a, Integer const &b) {
        Number c;
        fmpq_mul_fmpz(&c.num_, &a.num_, &b.impl());
        return c;
    }
    friend Number &operator*=(Number &a, Integer const &b) {
        fmpq_mul_fmpz(&a.num_, &a.num_, &b.impl());
        return a;
    }

    friend Number operator/(Number const &a, Integer const &b) {
        Number c;
        fmpq_div_fmpz(&c.num_, &a.num_, &b.impl());
        return c;
    }
    friend Number &operator/=(Number &a, Integer const &b) {
        fmpq_div_fmpz(&a.num_, &a.num_, &b.impl());
        return a;
    }

    friend Number operator+(Number const &a, Integer const &b) {
        Number c;
        fmpq_add_fmpz(&c.num_, &a.num_, &b.impl());
        return c;
    }
    friend Number &operator+=(Number &a, Integer const &b) {
        fmpq_add_fmpz(&a.num_, &a.num_, &b.impl());
        return a;
    }

    friend Number operator-(Number const &a, Integer const &b) {
        Number c;
        fmpq_sub_fmpz(&c.num_, &a.num_, &b.impl());
        return c;
    }
    friend Number &operator-=(Number &a, Integer const &b) {
        fmpq_sub_fmpz(&a.num_, &a.num_, &b.impl());
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
        return !(a == b);
    }
    friend bool operator<(Number const &a, slong b) {
        return compare(a, b) < 0;
    }
    friend bool operator<=(Number const &a, slong b) {
        return compare(a, b) <= 0;
    }
    friend bool operator>(Number const &a, slong b) {
        return compare(a, b) > 0;
    }
    friend bool operator>=(Number const &a, slong b) {
        return compare(a, b) >= 0;
    }
    friend bool operator==(Number const &a, slong b) {
#if __FLINT_RELEASE >= 20600
        return fmpq_equal_si(&a.num_, b) != 0;
#else
        return a == Number{b};
#endif
    }
    friend bool operator!=(Number const &a, slong b) {
        return !(a == b);
    }

    friend int compare(Number const &a, Number const &b) {
        return fmpq_cmp(&a.num_, &b.num_);
    }

    friend int compare(Number const &a, slong b) {
#if __FLINT_RELEASE >= 20600
        return fmpq_cmp_si(&a.num_, b);
#else
        return compare(a, Number{b});
#endif
    }

    friend std::ostream &operator<<(std::ostream &out, Number const &a) {
        std::unique_ptr<char, decltype(std::free) *> buf{fmpq_get_str(nullptr, BASE, &a.num_), std::free};
        if (buf == nullptr) {
            throw std::bad_alloc();
        }
        out << buf.get();
        return out;
    }

private:
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
    friend NumberQ operator+(NumberQ const &a, Integer const &b);
    friend NumberQ operator+(NumberQ const &a, Number  const &b);
    friend NumberQ operator+(NumberQ const &a, NumberQ const &b);

    friend NumberQ operator-(NumberQ const &a, Integer const &b);
    friend NumberQ operator-(NumberQ const &a, Number  const &b);
    friend NumberQ operator-(NumberQ const &a, NumberQ const &b);

    friend NumberQ operator*(NumberQ const &a, Integer const &b);
    friend NumberQ operator*(NumberQ const &a, Number  const &b);

    friend NumberQ operator/(NumberQ const &a, Integer const &b);
    friend NumberQ operator/(NumberQ const &a, Number  const &b);

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

[[nodiscard]] inline NumberQ operator+(NumberQ const &a, Integer const &b) {
    return NumberQ{a.c_ + b, a.k_};
}

[[nodiscard]] inline NumberQ operator+(NumberQ const &a, Number const &b) {
    return NumberQ{a.c_ + b, a.k_};
}

[[nodiscard]] inline NumberQ operator+(NumberQ const &a, NumberQ const &b) {
    return NumberQ{a.c_ + b.c_, a.k_ + b.k_};
}

// subtraction

[[nodiscard]] inline NumberQ operator-(NumberQ const &a, Integer const &b) {
    return NumberQ{a.c_ - b, a.k_};
}

[[nodiscard]] inline NumberQ operator-(NumberQ const &a, Number const &b) {
    return NumberQ{a.c_ - b, a.k_};
}

[[nodiscard]] inline NumberQ operator-(NumberQ const &a, NumberQ const &b) {
    return NumberQ{a.c_ - b.c_, a.k_ - b.k_};
}

// multiplication

[[nodiscard]] inline NumberQ operator*(NumberQ const &a, Integer const &b) {
    return NumberQ{a.c_ * b, a.k_ * b};
}

[[nodiscard]] inline NumberQ operator*(NumberQ const &a, Number const &b) {
    return NumberQ{a.c_ * b, a.k_ * b};
}

// division

[[nodiscard]] inline NumberQ operator/(NumberQ const &a, Integer const &b) {
    return NumberQ{a.c_ / b, a.k_ / b};
}

[[nodiscard]] inline NumberQ operator/(NumberQ const &a, Number const &b) {
    return NumberQ{a.c_ / b, a.k_ / b};
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
