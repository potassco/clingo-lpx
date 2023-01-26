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

    Integer &add_mul(Integer const &a, Integer const &b) & {
        fmpz_addmul(&num_, &a.num_, &b.num_);
        return *this;
    }

    Integer add_mul(Integer const &a, Integer const &b) && {
        return std::move(this->add_mul(a, b));
    }

    Integer &neg() {
        fmpz_neg(&num_, &num_);
        return *this;
    }

    friend Integer operator*(Integer &&a, Integer const &b) {
        return std::move(a *= b);
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

class Rational {
public:
    Rational() noexcept { // NOLINT
        fmpq_init(&num_);
    }
    Rational(slong val)
    : Rational() {
        fmpq_set_si(&num_, val, 1);
    }
    Rational(char const *val, int radix)
    : Rational() {
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
    Rational(std::string const &val, int radix)
    : Rational(val.c_str(), radix) {
    }
    Rational(Rational const &a)
    : Rational() {
        fmpq_set(&num_, &a.num_);
    }
    Rational(Integer const &num, Integer const &den)
    : Rational() {
        fmpz_set(&num_.num, &num.impl());
        fmpz_set(&num_.den, &den.impl());
        fmpq_canonicalise(&num_);
    }
    Rational(Rational &&a) noexcept
    : Rational() {
        swap(a);
    }
    Rational &operator=(Rational const &a) {
        fmpq_set(&num_, &a.num_);
        return *this;
    }
    Rational &operator=(Rational &&a) noexcept {
        swap(a);
        return *this;
    }
    ~Rational() noexcept {
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

    Rational &neg() {
        fmpq_neg(&num_, &num_);
        return *this;
    }

    void swap(Rational &x) {
        fmpq_swap(&num_, &x.num_);
    }

    void canonicalize() {
        fmpq_canonicalise(&num_);
    }

    friend Rational operator*(Rational const &a, Rational const &b) {
        Rational c;
        fmpq_mul(&c.num_, &a.num_, &b.num_);
        return c;
    }
    friend Rational &operator*=(Rational &a, Rational const &b) {
        fmpq_mul(&a.num_, &a.num_, &b.num_);
        return a;
    }

    friend Rational operator/(Rational const &a, Rational const &b) {
        Rational c;
        fmpq_div(&c.num_, &a.num_, &b.num_);
        return c;
    }
    friend Rational &operator/=(Rational &a, Rational const &b) {
        fmpq_div(&a.num_, &a.num_, &b.num_);
        return a;
    }

    friend Rational operator+(Rational const &a, Rational const &b) {
        Rational c;
        fmpq_add(&c.num_, &a.num_, &b.num_);
        return c;
    }
    friend Rational &operator+=(Rational &a, Rational const &b) {
        fmpq_add(&a.num_, &a.num_, &b.num_);
        return a;
    }

    friend Rational operator-(Rational const &a) {
        Rational c;
        fmpq_neg(&c.num_, &a.num_);
        return c;
    }
    friend Rational operator-(Rational const &a, Rational const &b) {
        Rational c;
        fmpq_sub(&c.num_, &a.num_, &b.num_);
        return c;
    }
    friend Rational &operator-=(Rational &a, Rational const &b) {
        fmpq_sub(&a.num_, &a.num_, &b.num_);
        return a;
    }

    friend Rational operator*(Rational &&a, Integer const &b) {
        return std::move(a /= b);
    }
    friend Rational operator*(Rational const &a, Integer const &b) {
        Rational c;
        fmpq_mul_fmpz(&c.num_, &a.num_, &b.impl());
        return c;
    }
    friend Rational &operator*=(Rational &a, Integer const &b) {
        fmpq_mul_fmpz(&a.num_, &a.num_, &b.impl());
        return a;
    }

    friend Rational operator/(Rational const &a, Integer const &b) {
        Rational c;
        fmpq_div_fmpz(&c.num_, &a.num_, &b.impl());
        return c;
    }
    friend Rational operator/(Rational &&a, Integer const &b) {
        return std::move(a /= b);
    }
    friend Rational &operator/=(Rational &a, Integer const &b) {
        fmpq_div_fmpz(&a.num_, &a.num_, &b.impl());
        return a;
    }

    friend Rational operator+(Rational const &a, Integer const &b) {
        Rational c;
        fmpq_add_fmpz(&c.num_, &a.num_, &b.impl());
        return c;
    }
    friend Rational &operator+=(Rational &a, Integer const &b) {
        fmpq_add_fmpz(&a.num_, &a.num_, &b.impl());
        return a;
    }

    friend Rational operator-(Rational const &a, Integer const &b) {
        Rational c;
        fmpq_sub_fmpz(&c.num_, &a.num_, &b.impl());
        return c;
    }
    friend Rational &operator-=(Rational &a, Integer const &b) {
        fmpq_sub_fmpz(&a.num_, &a.num_, &b.impl());
        return a;
    }

    friend bool operator<(Rational const &a, Rational const &b) {
        return compare(a, b) < 0;
    }
    friend bool operator<=(Rational const &a, Rational const &b) {
        return compare(a, b) <= 0;
    }
    friend bool operator>(Rational const &a, Rational const &b) {
        return compare(a, b) > 0;
    }
    friend bool operator>=(Rational const &a, Rational const &b) {
        return compare(a, b) >= 0;
    }
    friend bool operator==(Rational const &a, Rational const &b) {
        return fmpq_equal(&a.num_, &b.num_) != 0;
    }
    friend bool operator!=(Rational const &a, Rational const &b) {
        return !(a == b);
    }
    friend bool operator<(Rational const &a, slong b) {
        return compare(a, b) < 0;
    }
    friend bool operator<=(Rational const &a, slong b) {
        return compare(a, b) <= 0;
    }
    friend bool operator>(Rational const &a, slong b) {
        return compare(a, b) > 0;
    }
    friend bool operator>=(Rational const &a, slong b) {
        return compare(a, b) >= 0;
    }
    friend bool operator==(Rational const &a, slong b) {
#if __FLINT_RELEASE >= 20600
        return fmpq_equal_si(&a.num_, b) != 0;
#else
        return a == Rational{b};
#endif
    }
    friend bool operator!=(Rational const &a, slong b) {
        return !(a == b);
    }

    friend int compare(Rational const &a, Rational const &b) {
        return fmpq_cmp(&a.num_, &b.num_);
    }

    friend int compare(Rational const &a, slong b) {
#if __FLINT_RELEASE >= 20600
        return fmpq_cmp_si(&a.num_, b);
#else
        return compare(a, Rational{b});
#endif
    }

    friend std::ostream &operator<<(std::ostream &out, Rational const &a) {
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

class Rational {
public:
    Rational() { // NOLINT
        handle_error_(mp_rat_init(&num_));
    }
    Rational(mp_small val)
    : Rational() {
        handle_error_(mp_rat_set_value(&num_, val, 1));
    }
    Rational(char const *val, mp_size radix)
    : Rational() {
        handle_error_(mp_rat_read_string(&num_, radix, val));
    }
    Rational(std::string const &val, mp_size radix)
    : Rational(val.c_str(), radix) {
    }
    Rational(Rational const &a)
    : Rational() {
        handle_error_(mp_rat_copy(&a.num_, &num_));
    }
    Rational(Rational &&a) noexcept
    : Rational() {
        swap(a);
    }
    Rational &operator=(Rational const &a) {
        handle_error_(mp_rat_copy(&a.num_, &num_));
        return *this;
    }
    Rational &operator=(Rational &&a) noexcept {
        swap(a);
        return *this;
    }
    ~Rational() {
        mp_rat_clear(&num_);
    }

    void swap(Rational &x) {
        mp_int_swap(mp_rat_numer_ref(&num_), mp_rat_numer_ref(&x.num_));
        mp_int_swap(mp_rat_denom_ref(&num_), mp_rat_denom_ref(&x.num_));
    }

    void canonicalize() {
        handle_error_(mp_rat_reduce(&num_));
    }

    friend Rational operator*(Rational const &a, Rational const &b) {
        Rational c;
        handle_error_(mp_rat_mul(&a.num_, &b.num_, &c.num_));
        return c;
    }

    friend Rational &operator*=(Rational &a, Rational const &b) {
        Rational c;
        handle_error_(mp_rat_mul(&a.num_, &b.num_, &c.num_));
        c.swap(a);
        return a;
    }

    friend Rational operator/(Rational const &a, Rational const &b) {
        Rational c;
        handle_error_(mp_rat_div(&a.num_, &b.num_, &c.num_));
        return c;
    }

    friend Rational &operator/=(Rational &a, Rational const &b) {
        handle_error_(mp_rat_div(&a.num_, &b.num_, &a.num_));
        return a;
    }

    friend Rational operator+(Rational const &a, Rational const &b) {
        Rational c;
        handle_error_(mp_rat_add(&a.num_, &b.num_, &c.num_));
        return c;
    }
    friend Rational &operator+=(Rational &a, Rational const &b) {
        Rational c;
        handle_error_(mp_rat_add(&a.num_, &b.num_, &c.num_));
        c.swap(a);
        return a;
    }

    friend Rational operator-(Rational const &a) {
        Rational b;
        handle_error_(mp_rat_neg(&a.num_, &b.num_));
        return b;
    }
    friend Rational operator-(Rational const &a, Rational const &b) {
        Rational c;
        handle_error_(mp_rat_sub(&a.num_, &b.num_, &c.num_));
        return c;
    }
    friend Rational &operator-=(Rational &a, Rational const &b) {
        Rational c;
        handle_error_(mp_rat_sub(&a.num_, &b.num_, &c.num_));
        c.swap(a);
        return a;
    }

    friend bool operator<(Rational const &a, Rational const &b) {
        return mp_rat_compare(&a.num_, &b.num_) < 0;
    }
    friend bool operator<=(Rational const &a, Rational const &b) {
        return mp_rat_compare(&a.num_, &b.num_) <= 0;
    }
    friend bool operator>(Rational const &a, Rational const &b) {
        return mp_rat_compare(&a.num_, &b.num_) > 0;
    }
    friend bool operator>=(Rational const &a, Rational const &b) {
        return mp_rat_compare(&a.num_, &b.num_) >= 0;
    }
    friend bool operator==(Rational const &a, Rational const &b) {
        return mp_rat_compare(&a.num_, &b.num_) == 0;
    }
    friend bool operator!=(Rational const &a, Rational const &b) {
        return mp_rat_compare(&a.num_, &b.num_) != 0;
    }
    friend bool operator<(Rational const &a, mp_small b) {
        return mp_rat_compare_value(&a.num_, b, 1) < 0;
    }
    friend bool operator<=(Rational const &a, mp_small b) {
        return mp_rat_compare_value(&a.num_, b, 1) <= 0;
    }
    friend bool operator>(Rational const &a, mp_small b) {
        return mp_rat_compare_value(&a.num_, b, 1) > 0;
    }
    friend bool operator>=(Rational const &a, mp_small b) {
        return mp_rat_compare_value(&a.num_, b, 1) >= 0;
    }
    friend bool operator==(Rational const &a, mp_small b) {
        return mp_rat_compare_value(&a.num_, b, 1) == 0;
    }
    friend bool operator!=(Rational const &a, mp_small b) {
        return mp_rat_compare_value(&a.num_, b, 1) != 0;
    }

    friend int compare(Rational const &a, Rational const &b) {
        return mp_rat_compare(&a.num_, &b.num_);
    }

    friend std::ostream &operator<<(std::ostream &out, Rational const &a) {
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

using Rational = mpq_class;

inline int compare(Rational const &a, Rational const &b) {
    return mpq_cmp(a.get_mpq_t(), b.get_mpq_t());
}

#endif

class RationalQ {
private:
    friend RationalQ operator+(RationalQ const &a, Integer const &b);
    friend RationalQ operator+(RationalQ const &a, Rational  const &b);
    friend RationalQ operator+(RationalQ const &a, RationalQ const &b);

    friend RationalQ operator-(RationalQ const &a, Integer const &b);
    friend RationalQ operator-(RationalQ const &a, Rational  const &b);
    friend RationalQ operator-(RationalQ const &a, RationalQ const &b);

    friend RationalQ operator*(RationalQ &&a, Integer const &b);
    friend RationalQ operator*(RationalQ const &a, Integer const &b);
    friend RationalQ operator*(RationalQ const &a, Rational  const &b);

    friend RationalQ operator/(RationalQ &&a, Integer const &b);
    friend RationalQ operator/(RationalQ const &a, Integer const &b);
    friend RationalQ operator/(RationalQ const &a, Rational  const &b);

    friend std::ostream &operator<<(std::ostream &out, RationalQ const &q);

public:
    explicit RationalQ(Rational c = Rational{}, Rational k = Rational{})
    : c_{std::move(c)}
    , k_{std::move(k)} { }
    RationalQ(RationalQ const &) = default;
    RationalQ(RationalQ &&) = default;
    RationalQ &operator=(RationalQ const &) = default;
    RationalQ &operator=(RationalQ &&) = default;
    ~RationalQ() = default;

    void swap(RationalQ &q) {
        c_.swap(q.c_);
        k_.swap(q.k_);
    }

    // addition
    RationalQ &operator+=(Integer const &c) {
        c_ += c;
        return *this;
    }

    RationalQ &operator+=(Rational const &c) {
        c_ += c;
        return *this;
    }

    RationalQ &operator+=(RationalQ const &q) {
        c_ += q.c_;
        k_ += q.k_;
        return *this;
    }

    RationalQ &operator-=(Integer const &c) {
        c_ -= c;
        return *this;
    }

    RationalQ &operator-=(Rational const &c) {
        c_ -= c;
        return *this;
    }

    RationalQ &operator-=(RationalQ const &q) {
        c_ -= q.c_;
        k_ -= q.k_;
        return *this;
    }

    RationalQ &operator*=(Integer const &c) {
        c_ *= c;
        k_ *= c;
        return *this;
    }

    RationalQ &operator*=(Rational const &c) {
        c_ *= c;
        k_ *= c;
        return *this;
    }

    RationalQ &operator/=(Integer const &c) {
        c_ /= c;
        k_ /= c;
        return *this;
    }

    RationalQ &operator/=(Rational const &c) {
        c_ /= c;
        k_ /= c;
        return *this;
    }

    [[nodiscard]] bool operator<(Rational const &c) const {
        return cmp_(c) < 0;
    }

    [[nodiscard]] bool operator<(RationalQ const &q) const {
        return cmp_(q) < 0;
    }

    [[nodiscard]] bool operator<=(Rational const &c) const {
        return cmp_(c) <= 0;
    }

    [[nodiscard]] bool operator<=(RationalQ const &q) const {
        return cmp_(q) <= 0;
    }

    [[nodiscard]] bool operator>(Rational const &c) const {
        return cmp_(c) > 0;
    }

    [[nodiscard]] bool operator>(RationalQ const &q) const {
        return cmp_(q) > 0;
    }

    [[nodiscard]] bool operator>=(Rational const &c) const {
        return cmp_(c) >= 0;
    }

    [[nodiscard]] bool operator>=(RationalQ const &q) const {
        return cmp_(q) >= 0;
    }

    [[nodiscard]] bool operator==(RationalQ const &q) const {
        return c_ == q.c_ && k_ == q.k_;
    }

    [[nodiscard]] bool operator==(Rational const &c) const {
        return c_ == c && k_ == 0;
    }

    [[nodiscard]] bool operator!=(RationalQ const &q) const {
        return c_ != q.c_ || k_ != q.k_;
    }

    [[nodiscard]] bool operator!=(Rational const &c) const {
        return c_ != c || k_ != 0;
    }

private:
    [[nodiscard]] int cmp_(RationalQ const &q) const {
        auto ret = compare(c_, q.c_);
        if (ret != 0) {
            return ret;
        }
        return compare(k_, q.k_);
    }
    [[nodiscard]] int cmp_(Rational const &c) const {
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

    Rational c_;
    Rational k_;
};

// addition

[[nodiscard]] inline RationalQ operator+(RationalQ const &a, Integer const &b) {
    return RationalQ{a.c_ + b, a.k_};
}

[[nodiscard]] inline RationalQ operator+(RationalQ const &a, Rational const &b) {
    return RationalQ{a.c_ + b, a.k_};
}

[[nodiscard]] inline RationalQ operator+(RationalQ const &a, RationalQ const &b) {
    return RationalQ{a.c_ + b.c_, a.k_ + b.k_};
}

// subtraction

[[nodiscard]] inline RationalQ operator-(RationalQ const &a, Integer const &b) {
    return RationalQ{a.c_ - b, a.k_};
}

[[nodiscard]] inline RationalQ operator-(RationalQ const &a, Rational const &b) {
    return RationalQ{a.c_ - b, a.k_};
}

[[nodiscard]] inline RationalQ operator-(RationalQ const &a, RationalQ const &b) {
    return RationalQ{a.c_ - b.c_, a.k_ - b.k_};
}

// multiplication

[[nodiscard]] inline RationalQ operator*(RationalQ &&a, Integer const &b) {
    return std::move(a *= b);
}

[[nodiscard]] inline RationalQ operator*(RationalQ const &a, Integer const &b) {
    return RationalQ{a.c_ * b, a.k_ * b};
}

[[nodiscard]] inline RationalQ operator*(RationalQ const &a, Rational const &b) {
    return RationalQ{a.c_ * b, a.k_ * b};
}

// division

[[nodiscard]] inline RationalQ operator/(RationalQ &&a, Integer const &b) {
    return std::move(a /= b);
}

[[nodiscard]] inline RationalQ operator/(RationalQ const &a, Integer const &b) {
    return RationalQ{a.c_ / b, a.k_ / b};
}

[[nodiscard]] inline RationalQ operator/(RationalQ const &a, Rational const &b) {
    return RationalQ{a.c_ / b, a.k_ / b};
}

inline std::ostream &operator<<(std::ostream &out, RationalQ const &q) {
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
