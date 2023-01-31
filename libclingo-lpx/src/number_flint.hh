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
    friend Integer operator+(Integer const &a, fixed_int b);
    friend Integer operator+(Integer const &a, Integer const &b);
    friend Integer operator+(Integer &&a, fixed_int b);
    friend Integer operator+(Integer &&a, Integer const &b);
    friend Integer &operator+=(Integer &a, fixed_int b);
    friend Integer &operator+=(Integer &a, Integer const &b);

    friend Integer operator-(Integer a);

    friend Integer operator-(Integer const &a, fixed_int b);
    friend Integer operator-(Integer const &a, Integer const &b);
    friend Integer operator-(Integer &&a, fixed_int b);
    friend Integer operator-(Integer &&a, Integer const &b);
    friend Integer &operator-=(Integer &a, fixed_int b);
    friend Integer &operator-=(Integer &a, Integer const &b);

    friend Integer operator*(Integer const &a, fixed_int b);
    friend Integer operator*(Integer const &a, Integer const &b);
    friend Integer operator*(Integer &&a, fixed_int b);
    friend Integer operator*(Integer &&a, Integer const &b);
    friend Integer &operator*=(Integer &a, fixed_int b);
    friend Integer &operator*=(Integer &a, Integer const &b);

    friend bool operator<(Integer const &a, fixed_int b);
    friend bool operator<(Integer const &a, Integer const &b);

    friend bool operator<=(Integer const &a, fixed_int b);
    friend bool operator<=(Integer const &a, Integer const &b);

    friend bool operator>(Integer const &a, fixed_int b);
    friend bool operator>(Integer const &a, Integer const &b);

    friend bool operator>=(Integer const &a, fixed_int b);
    friend bool operator>=(Integer const &a, Integer const &b);

    friend bool operator==(Integer const &a, fixed_int b);
    friend bool operator==(Integer const &a, Integer const &b);

    friend bool operator!=(Integer const &a, fixed_int b);
    friend bool operator!=(Integer const &a, Integer const &b);

    friend std::ostream &operator<<(std::ostream &out, Integer const &a);

    friend int compare(Integer const &a, fixed_int b);
    friend int compare(Integer const &a, Integer const &b);

    friend std::tuple<Integer, Integer, Integer> gcd(Integer const &a, Integer const &b);

public:
    Integer() noexcept;
    Integer(fixed_int val);
    Integer(char const *val, int radix);
    Integer(std::string const &val, int radix);
    Integer(Integer const &a);
    Integer(Integer &&a) noexcept;
    Integer &operator=(Integer const &a);
    Integer &operator=(Integer &&a) noexcept;
    ~Integer() noexcept;

    void swap(Integer &x) noexcept;
    Integer &add_mul(Integer const &a, Integer const &b) &;
    Integer add_mul(Integer const &a, Integer const &b) &&;
    Integer &neg();
    [[nodiscard]] fmpz &impl() const;

private:
    mutable fmpz num_;
};

class Rational {
private:
    friend Rational operator+(Rational const &a, fixed_int b);
    friend Rational operator+(Rational const &a, Integer const &b);
    friend Rational operator+(Rational const &a, Rational const &b);
    friend Rational operator+(Rational &&a, fixed_int b);
    friend Rational operator+(Rational &&a, Integer const &b);
    friend Rational operator+(Rational &&a, Rational const &b);
    friend Rational &operator+=(Rational &a, fixed_int b);
    friend Rational &operator+=(Rational &a, Integer const &b);
    friend Rational &operator+=(Rational &a, Rational const &b);

    friend Rational operator-(Rational a);

    friend Rational operator-(Rational const &a, fixed_int b);
    friend Rational operator-(Rational const &a, Integer const &b);
    friend Rational operator-(Rational const &a, Rational const &b);
    friend Rational operator-(Rational &&a, fixed_int b);
    friend Rational operator-(Rational &&a, Integer const &b);
    friend Rational operator-(Rational &&a, Rational const &b);
    friend Rational &operator-=(Rational &a, fixed_int b);
    friend Rational &operator-=(Rational &a, Integer const &b);
    friend Rational &operator-=(Rational &a, Rational const &b);

    friend Rational operator*(Rational const &a, fixed_int b);
    friend Rational operator*(Rational const &a, Integer const &b);
    friend Rational operator*(Rational const &a, Rational const &b);
    friend Rational operator*(Rational &&a, fixed_int b);
    friend Rational operator*(Rational &&a, Integer const &b);
    friend Rational operator*(Rational &&a, Rational const &b);
    friend Rational &operator*=(Rational &a, fixed_int b);
    friend Rational &operator*=(Rational &a, Integer const &b);
    friend Rational &operator*=(Rational &a, Rational const &b);

    friend Rational operator/(Rational const &a, fixed_int b);
    friend Rational operator/(Rational const &a, Integer const &b);
    friend Rational operator/(Rational const &a, Rational const &b);
    friend Rational operator/(Rational &&a, fixed_int b);
    friend Rational operator/(Rational &&a, Integer const &b);
    friend Rational operator/(Rational &&a, Rational const &b);
    friend Rational &operator/=(Rational &a, fixed_int b);
    friend Rational &operator/=(Rational &a, Integer const &b);
    friend Rational &operator/=(Rational &a, Rational const &b);

    friend bool operator<(Rational const &a, fixed_int b);
    friend bool operator<(Rational const &a, Integer const &b);
    friend bool operator<(Rational const &a, Rational const &b);

    friend bool operator<=(Rational const &a, fixed_int b);
    friend bool operator<=(Rational const &a, Integer const &b);
    friend bool operator<=(Rational const &a, Rational const &b);

    friend bool operator>(Rational const &a, fixed_int b);
    friend bool operator>(Rational const &a, Integer const &b);
    friend bool operator>(Rational const &a, Rational const &b);

    friend bool operator>=(Rational const &a, fixed_int b);
    friend bool operator>=(Rational const &a, Integer const &b);
    friend bool operator>=(Rational const &a, Rational const &b);

    friend bool operator==(Rational const &a, fixed_int b);
    friend bool operator==(Rational const &a, Integer const &b);
    friend bool operator==(Rational const &a, Rational const &b);

    friend bool operator!=(Rational const &a, fixed_int b);
    friend bool operator!=(Rational const &a, Integer const &b);
    friend bool operator!=(Rational const &a, Rational const &b);

    friend std::ostream &operator<<(std::ostream &out, Rational const &a);

    friend int compare(Rational const &a, fixed_int b);
    friend int compare(Rational const &a, Integer const &b);
    friend int compare(Rational const &a, Rational const &b);

public:
    Rational() noexcept;
    Rational(fixed_int val);
    Rational(Integer num, Integer den);
    Rational(char const *val, int radix);
    Rational(std::string const &val, int radix);
    Rational(Rational const &a);
    Rational(Rational &&a) noexcept;
    Rational &operator=(Rational const &a);
    Rational &operator=(Rational &&a) noexcept;
    ~Rational() noexcept;

    [[nodiscard]] Integer &num();
    [[nodiscard]] Integer const &num() const;
    [[nodiscard]] Integer &den();
    [[nodiscard]] Integer const &den() const;
    Rational &neg();
    void swap(Rational &x) noexcept;
    void canonicalize();

private:
    mutable fmpq num_;
};

// Integer

inline Integer::Integer() noexcept { // NOLINT
    fmpz_init(&num_);
}

inline Integer::Integer(fixed_int val)
: Integer() {
    fmpz_set_si(&num_, val);
}

inline Integer::Integer(char const *val, int radix)
: Integer() {
    if (fmpz_set_str(&num_, val, radix) != 0) {
        throw std::runtime_error("could not parse number");
    }
}

inline Integer::Integer(std::string const &val, int radix)
: Integer(val.c_str(), radix) { }

inline Integer::Integer(Integer const &a)
: Integer() {
    fmpz_set(&num_, &a.num_);
}

inline Integer::Integer(Integer &&a) noexcept
: Integer() {
    swap(a);
}

inline Integer &Integer::operator=(Integer const &a) {
    fmpz_set(&num_, &a.num_);
    return *this;
}

inline Integer &Integer::operator=(Integer &&a) noexcept {
    swap(a);
    return *this;
}

inline Integer::~Integer() noexcept {
    fmpz_clear(&num_);
}

inline void Integer::swap(Integer &x) noexcept {
    fmpz_swap(&num_, &x.num_);
}

inline Integer &Integer::add_mul(Integer const &a, Integer const &b) & {
    fmpz_addmul(&num_, &a.num_, &b.num_);
    return *this;
}

inline Integer Integer::add_mul(Integer const &a, Integer const &b) && {
    return std::move(this->add_mul(a, b));
}

inline Integer &Integer::neg() {
    fmpz_neg(&num_, &num_);
    return *this;
}

inline fmpz &Integer::impl() const {
    return num_;
}

// addition

[[nodiscard]] inline Integer operator+(Integer const &a, fixed_int b) {
#if __FLINT_RELEASE >= 20600
    Integer c;
    fmpz_add_si(&c.num_, &a.num_, b);
    return c;
#else
    return a + Integer{b};
#endif
}

[[nodiscard]] inline Integer operator+(Integer const &a, Integer const &b) {
    Integer c;
    fmpz_add(&c.num_, &a.num_, &b.num_);
    return c;
}

[[nodiscard]] inline Integer operator+(Integer &&a, fixed_int b) {
    return std::move(a += b);
}

[[nodiscard]] inline Integer operator+(Integer &&a, Integer const &b) {
    return std::move(a += b);
}

inline Integer &operator+=(Integer &a, fixed_int b) {
#if __FLINT_RELEASE >= 20600
    fmpz_add_si(&a.num_, &a.num_, b);
    return a;
#else
    return a += Integer{b};
#endif
}

inline Integer &operator+=(Integer &a, Integer const &b) {
    fmpz_add(&a.num_, &a.num_, &b.num_);
    return a;
}

// subtraction

[[nodiscard]] inline Integer operator-(Integer a) {
    a.neg();
    return a;
}

[[nodiscard]] inline Integer operator-(Integer const &a, fixed_int b) {
#if __FLINT_RELEASE >= 20600
    Integer c;
    fmpz_sub_si(&c.num_, &a.num_, b);
    return c;
#else
    return a - Integer{b};
#endif
}

[[nodiscard]] inline Integer operator-(Integer const &a, Integer const &b) {
    Integer c;
    fmpz_sub(&c.num_, &a.num_, &b.num_);
    return c;
}

[[nodiscard]] inline Integer operator-(Integer &&a, fixed_int b) {
    return std::move(a -= b);
}

[[nodiscard]] inline Integer operator-(Integer &&a, Integer const &b) {
    return std::move(a -= b);
}

inline Integer &operator-=(Integer &a, fixed_int b) {
#if __FLINT_RELEASE >= 20600
    fmpz_sub_si(&a.num_, &a.num_, b);
    return a;
#else
    return a -= Integer{b};
#endif
}

inline Integer &operator-=(Integer &a, Integer const &b) {
    fmpz_sub(&a.num_, &a.num_, &b.num_);
    return a;
}

// multiplication

[[nodiscard]] inline Integer operator*(Integer const &a, fixed_int b) {
    Integer c;
    fmpz_mul_si(&c.num_, &a.num_, b);
    return c;
}

[[nodiscard]] inline Integer operator*(Integer const &a, Integer const &b) {
    Integer c;
    fmpz_mul(&c.num_, &a.num_, &b.num_);
    return c;
}

[[nodiscard]] inline Integer operator*(Integer &&a, fixed_int b) {
    return std::move(a *= b);
}

[[nodiscard]] inline Integer operator*(Integer &&a, Integer const &b) {
    return std::move(a *= b);
}

inline Integer &operator*=(Integer &a, fixed_int b) {
    fmpz_mul_si(&a.num_, &a.num_, b);
    return a;
}

inline Integer &operator*=(Integer &a, Integer const &b) {
    fmpz_mul(&a.num_, &a.num_, &b.num_);
    return a;
}

// less than

[[nodiscard]] inline bool operator<(Integer const &a, fixed_int b) {
    return compare(a, b) < 0;
}

[[nodiscard]] inline bool operator<(Integer const &a, Integer const &b) {
    return compare(a, b) < 0;
}

// less than or equal to

[[nodiscard]] inline bool operator<=(Integer const &a, fixed_int b) {
    return compare(a, b) <= 0;
}

[[nodiscard]] inline bool operator<=(Integer const &a, Integer const &b) {
    return compare(a, b) <= 0;
}

// greater than

[[nodiscard]] inline bool operator>(Integer const &a, fixed_int b) {
    return compare(a, b) > 0;
}

[[nodiscard]] inline bool operator>(Integer const &a, Integer const &b) {
    return compare(a, b) > 0;
}

// greater than or equal to

[[nodiscard]] inline bool operator>=(Integer const &a, fixed_int b) {
    return compare(a, b) >= 0;
}

[[nodiscard]] inline bool operator>=(Integer const &a, Integer const &b) {
    return compare(a, b) >= 0;
}

// greater equal to


[[nodiscard]] inline bool operator==(Integer const &a, fixed_int b) {
    return fmpz_equal_si(&a.num_, b) != 0;
}

[[nodiscard]] inline bool operator==(Integer const &a, Integer const &b) {
    return fmpz_equal(&a.num_, &b.num_) != 0;
}

// not equal to

[[nodiscard]] inline bool operator!=(Integer const &a, fixed_int b) {
    return !(a == b);
}

[[nodiscard]] inline bool operator!=(Integer const &a, Integer const &b) {
    return !(a == b);
}

// printing

inline std::ostream &operator<<(std::ostream &out, Integer const &a) {
    std::unique_ptr<char, decltype(flint_free) *> buf{fmpz_get_str(nullptr, BASE, &a.num_), flint_free};
    if (buf == nullptr) {
        throw std::bad_alloc();
    }
    out << buf.get();
    return out;
}

// comparison

[[nodiscard]] inline int compare(Integer const &a, fixed_int b) {
    return fmpz_cmp_si(&a.num_, b);
}

[[nodiscard]] inline int compare(Integer const &a, Integer const &b) {
    return fmpz_cmp_si(&a.num_, b.num_);
}

// gcd

[[nodiscard]] inline std::tuple<Integer, Integer, Integer> gcd(Integer const &a, Integer const &b) {
    Integer g;
    Integer ga;
    Integer gb;
    fmpz_gcd(&g.num_, &a.num_, &b.num_);
    fmpz_divexact(&ga.num_, &a.num_, &g.num_);
    fmpz_divexact(&gb.num_, &b.num_, &g.num_);
    return {g, ga, gb};
}

// Rational

inline Rational::Rational() noexcept { // NOLINT
    fmpq_init(&num_);
}

inline Rational::Rational(fixed_int val)
: Rational() {
    fmpq_set_si(&num_, val, 1);
}

inline Rational::Rational(char const *val, int radix)
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

inline Rational::Rational(std::string const &val, int radix)
: Rational(val.c_str(), radix) {
}

inline Rational::Rational(Rational const &a)
: Rational() {
    fmpq_set(&num_, &a.num_);
}

inline Rational::Rational(Integer num, Integer den)
: Rational() {
    this->num() = std::move(num);
    this->den() = std::move(den);
    canonicalize();
}

inline Rational::Rational(Rational &&a) noexcept
: Rational() {
    swap(a);
}

inline Rational &Rational::operator=(Rational const &a) {
    fmpq_set(&num_, &a.num_);
    return *this;
}

inline Rational &Rational::operator=(Rational &&a) noexcept {
    swap(a);
    return *this;
}

inline Rational::~Rational() noexcept {
    fmpq_clear(&num_);
}

inline Integer &Rational::num() {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    return reinterpret_cast<Integer&>(num_.num);
}

inline Integer const &Rational::num() const {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    return reinterpret_cast<Integer const&>(num_.num);
}

inline Integer &Rational::den() {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    return reinterpret_cast<Integer&>(num_.den);
}

inline Integer const &Rational::den() const {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    return reinterpret_cast<Integer const&>(num_.den);
}

inline Rational &Rational::neg() {
    fmpq_neg(&num_, &num_);
    return *this;
}

inline void Rational::swap(Rational &x) noexcept {
    fmpq_swap(&num_, &x.num_);
}

inline void Rational::canonicalize() {
    fmpq_canonicalise(&num_);
}
// addition

[[nodiscard]] inline Rational operator+(Rational const &a, fixed_int b) {
#if __FLINT_RELEASE >= 20600
    Rational c;
    fmpq_add_si(&c.num_, &a.num_, b);
    return c;
#else
    return a + Rational{b};
#endif
}

[[nodiscard]] inline Rational operator+(Rational const &a, Integer const &b) {
    Rational c;
    fmpq_add_fmpz(&c.num_, &a.num_, &b.impl());
    return c;
}

[[nodiscard]] inline Rational operator+(Rational const &a, Rational const &b) {
    Rational c;
    fmpq_add(&c.num_, &a.num_, &b.num_);
    return c;
}

[[nodiscard]] inline Rational operator+(Rational &&a, fixed_int b) {
    return std::move(a += b);
}

[[nodiscard]] inline Rational operator+(Rational &&a, Integer const &b) {
    return std::move(a += b);
}

[[nodiscard]] inline Rational operator+(Rational &&a, Rational const &b) {
    return std::move(a += b);
}

inline Rational &operator+=(Rational &a, fixed_int b) {
    fmpq_add_si(&a.num_, &a.num_, b);
    return a;
}

inline Rational &operator+=(Rational &a, Integer const &b) {
    fmpq_add_fmpz(&a.num_, &a.num_, &b.impl());
    return a;
}

inline Rational &operator+=(Rational &a, Rational const &b) {
    fmpq_add(&a.num_, &a.num_, &b.num_);
    return a;
}

// subtraction

[[nodiscard]] inline Rational operator-(Rational a) {
    a.neg();
    return a;
}

[[nodiscard]] inline Rational operator-(Rational const &a, fixed_int b) {
#if __FLINT_RELEASE >= 20600
    Rational c;
    fmpq_sub_si(&c.num_, &a.num_, b);
    return c;
#else
    return a - Rational{b};
#endif
}

[[nodiscard]] inline Rational operator-(Rational const &a, Integer const &b) {
    Rational c;
    fmpq_sub_fmpz(&c.num_, &a.num_, &b.impl());
    return c;
}

[[nodiscard]] inline Rational operator-(Rational const &a, Rational const &b) {
    Rational c;
    fmpq_sub(&c.num_, &a.num_, &b.num_);
    return c;
}

[[nodiscard]] inline Rational operator-(Rational &&a, fixed_int b) {
    return std::move(a -= b);
}

[[nodiscard]] inline Rational operator-(Rational &&a, Integer const &b) {
    return std::move(a -= b);
}

[[nodiscard]] inline Rational operator-(Rational &&a, Rational const &b) {
    return std::move(a -= b);
}

inline Rational &operator-=(Rational &a, fixed_int b) {
    fmpq_sub_si(&a.num_, &a.num_, b);
    return a;
}

inline Rational &operator-=(Rational &a, Integer const &b) {
    fmpq_sub_fmpz(&a.num_, &a.num_, &b.impl());
    return a;
}

inline Rational &operator-=(Rational &a, Rational const &b) {
    fmpq_sub(&a.num_, &a.num_, &b.num_);
    return a;
}

// multiplication

[[nodiscard]] inline Rational operator*(Rational const &a, fixed_int b) {
#if __FLINT_RELEASE >= 20600
    Rational c;
    fmpq_mul_si(&c.num_, &a.num_, b);
    return c;
#else
    return a * Rational{b};
#endif
}

[[nodiscard]] inline Rational operator*(Rational const &a, Integer const &b) {
    Rational c;
    fmpq_mul_fmpz(&c.num_, &a.num_, &b.impl());
    return c;
}

[[nodiscard]] inline Rational operator*(Rational const &a, Rational const &b) {
    Rational c;
    fmpq_mul(&c.num_, &a.num_, &b.num_);
    return c;
}

[[nodiscard]] inline Rational operator*(Rational &&a, fixed_int b) {
    return std::move(a *= b);
}

[[nodiscard]] inline Rational operator*(Rational &&a, Integer const &b) {
    return std::move(a *= b);
}

[[nodiscard]] inline Rational operator*(Rational &&a, Rational const &b) {
    return std::move(a *= b);
}

inline Rational &operator*=(Rational &a, fixed_int b) {
#if __FLINT_RELEASE >= 20600
    fmpq_mul_si(&a.num_, &a.num_, b);
    return a;
#else
    return a *= Rational{b};
#endif
}

inline Rational &operator*=(Rational &a, Integer const &b) {
    fmpq_mul_fmpz(&a.num_, &a.num_, &b.impl());
    return a;
}

inline Rational &operator*=(Rational &a, Rational const &b) {
    fmpq_mul(&a.num_, &a.num_, &b.num_);
    return a;
}

// division

[[nodiscard]] inline Rational operator/(Rational const &a, fixed_int b) {
    return a / Integer{b};
}

[[nodiscard]] inline Rational operator/(Rational const &a, Integer const &b) {
    Rational c;
    fmpq_div_fmpz(&c.num_, &a.num_, &b.impl());
    return c;
}

[[nodiscard]] inline Rational operator/(Rational const &a, Rational const &b) {
    Rational c;
    fmpq_div(&c.num_, &a.num_, &b.num_);
    return c;
}

[[nodiscard]] inline Rational operator/(Rational &&a, fixed_int b) {
    return std::move(a /= b);
}

[[nodiscard]] inline Rational operator/(Rational &&a, Integer const &b) {
    return std::move(a /= b);
}

[[nodiscard]] inline Rational operator/(Rational &&a, Rational const &b) {
    return std::move(a /= b);
}

inline Rational &operator/=(Rational &a, fixed_int b) {
    return a /= Integer{b};
}

inline Rational &operator/=(Rational &a, Integer const &b) {
    fmpq_div_fmpz(&a.num_, &a.num_, &b.impl());
    return a;
}

inline Rational &operator/=(Rational &a, Rational const &b) {
    fmpq_div(&a.num_, &a.num_, &b.num_);
    return a;
}

// less than

[[nodiscard]] inline bool operator<(Rational const &a, fixed_int b) {
    return compare(a, b) < 0;
}

[[nodiscard]] inline bool operator<(Rational const &a, Integer const &b) {
    return compare(a, b) < 0;
}

[[nodiscard]] inline bool operator<(Rational const &a, Rational const &b) {
    return compare(a, b) < 0;
}

// less than or equal to

[[nodiscard]] inline bool operator<=(Rational const &a, fixed_int b) {
    return compare(a, b) <= 0;
}

[[nodiscard]] inline bool operator<=(Rational const &a, Integer const &b) {
    return compare(a, b) <= 0;
}

[[nodiscard]] inline bool operator<=(Rational const &a, Rational const &b) {
    return compare(a, b) <= 0;
}

// greater than

[[nodiscard]] inline bool operator>(Rational const &a, fixed_int b) {
    return compare(a, b) > 0;
}

[[nodiscard]] inline bool operator>(Rational const &a, Integer const &b) {
    return compare(a, b) > 0;
}

[[nodiscard]] inline bool operator>(Rational const &a, Rational const &b) {
    return compare(a, b) > 0;
}

// greater than or equal to

[[nodiscard]] inline bool operator>=(Rational const &a, fixed_int b) {
    return compare(a, b) >= 0;
}

[[nodiscard]] inline bool operator>=(Rational const &a, Integer const &b) {
    return compare(a, b) >= 0;
}

[[nodiscard]] inline bool operator>=(Rational const &a, Rational const &b) {
    return compare(a, b) >= 0;
}

// equal to

[[nodiscard]] inline bool operator==(Rational const &a, fixed_int b) {
#if __FLINT_RELEASE >= 20600
        return fmpq_equal_si(&a.num_, b) != 0;
#else
        return a == Rational{b};
#endif
}

[[nodiscard]] inline bool operator==(Rational const &a, Integer const &b) {
#if __FLINT_RELEASE >= 20600
        return fmpq_equal_fmpz(&a.num_, &b.impl()) != 0;
#else
        return a == Rational{b, Integer{1}};
#endif
}

[[nodiscard]] inline bool operator==(Rational const &a, Rational const &b) {
        return fmpq_equal(&a.num_, &b.num_) != 0;
}

// not equal to

[[nodiscard]] inline bool operator!=(Rational const &a, fixed_int b) {
    return !(a == b);
}

[[nodiscard]] inline bool operator!=(Rational const &a, Integer const &b) {
    return !(a == b);
}

[[nodiscard]] inline bool operator!=(Rational const &a, Rational const &b) {
    return !(a == b);
}

// printing

inline std::ostream &operator<<(std::ostream &out, Rational const &a) {
    std::unique_ptr<char, decltype(flint_free) *> buf{fmpq_get_str(nullptr, BASE, &a.num_), flint_free};
    if (buf == nullptr) {
        throw std::bad_alloc();
    }
    out << buf.get();
    return out;
}

// comparison

[[nodiscard]] inline int compare(Rational const &a, fixed_int b) {
#if __FLINT_RELEASE >= 20600
        return fmpq_cmp_si(&a.num_, b);
#else
        return compare(a, Rational{b});
#endif
}

[[nodiscard]] inline int compare(Rational const &a, Integer const &b) {
#if __FLINT_RELEASE >= 20600
        return fmpq_cmp_fmpz(&a.num_, &b.impl());
#else
        return compare(a, Rational{b, 1});
#endif
}

[[nodiscard]] inline int compare(Rational const &a, Rational const &b) {
        return fmpq_cmp(&a.num_, &b.num_);
}

#endif
