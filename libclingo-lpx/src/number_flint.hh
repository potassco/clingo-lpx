#pragma once

#ifdef CLINGOLPX_USE_FLINT

#include <flint/fmpz.h>
#include <flint/fmpq.h>

#include <ios>
#include <new>
#include <stdexcept>
#include <string>
#include <memory>
#include <iostream>

constexpr int BASE = 10;

using fixed_int = slong;

class Integer {
public:
    Integer() noexcept { // NOLINT
        fmpz_init(&num_);
    }
    Integer(fixed_int val)
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
    friend bool operator<(Integer const &a, fixed_int b) {
        return fmpz_cmp_si(&a.num_, b) < 0;
    }
    friend bool operator<=(Integer const &a, fixed_int b) {
        return fmpz_cmp_si(&a.num_, b) <= 0;
    }
    friend bool operator>(Integer const &a, fixed_int b) {
        return fmpz_cmp_si(&a.num_, b) > 0;
    }
    friend bool operator>=(Integer const &a, fixed_int b) {
        return fmpz_cmp_si(&a.num_, b) >= 0;
    }
    friend bool operator==(Integer const &a, fixed_int b) {
        return fmpz_equal_si(&a.num_, b) != 0;
    }
    friend bool operator!=(Integer const &a, fixed_int b) {
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
    Rational(fixed_int val)
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
    friend bool operator<(Rational const &a, fixed_int b) {
        return compare(a, b) < 0;
    }
    friend bool operator<=(Rational const &a, fixed_int b) {
        return compare(a, b) <= 0;
    }
    friend bool operator>(Rational const &a, fixed_int b) {
        return compare(a, b) > 0;
    }
    friend bool operator>=(Rational const &a, fixed_int b) {
        return compare(a, b) >= 0;
    }
    friend bool operator==(Rational const &a, fixed_int b) {
#if __FLINT_RELEASE >= 20600
        return fmpq_equal_si(&a.num_, b) != 0;
#else
        return a == Rational{b};
#endif
    }
    friend bool operator!=(Rational const &a, fixed_int b) {
        return !(a == b);
    }
    friend bool operator==(Rational const &a, Integer const &b) {
#if __FLINT_RELEASE >= 20600
        return fmpq_equal_fmpz(&a.num_, b) != 0;
#else
        return a == Rational{b, Integer{1}};
#endif
    }

    friend int compare(Rational const &a, fixed_int b) {
#if __FLINT_RELEASE >= 20600
        return fmpq_cmp_si(&a.num_, b);
#else
        return compare(a, Rational{b});
#endif
    }

    friend int compare(Rational const &a, Integer const &b) {
#if __FLINT_RELEASE >= 20600
        return fmpq_cmp_fmpz(&a.num_, &b.impl());
#else
        return compare(a, Rational{b, 1});
#endif
    }

    friend int compare(Rational const &a, Rational const &b) {
        return fmpq_cmp(&a.num_, &b.num_);
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

#endif
