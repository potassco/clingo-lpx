#pragma once

#ifdef CLINGOLPX_USE_IMATH

#include <imath.h>
#include <imrat.h>

#include <ios>
#include <new>
#include <stdexcept>
#include <string>
#include <memory>
#include <iostream>

// TODO: it looks like imath also supports aliasing parameters

constexpr int BASE = 10;

using fixed_int = mp_small;

class Integer {
private:
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
    [[nodiscard]] mpz_t &impl() const;

private:
    mutable mpz_t num_;
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
    mutable mpq_t num_;
};


inline void mp_handle_error_(mp_result res) {
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

// Integer

inline Integer::Integer() noexcept { // NOLINT
    mp_int_init(&num_);
}

inline Integer::Integer(fixed_int val)
: Integer() {
    mp_handle_error_(mp_int_set_value(&num_, val));
}

inline Integer::Integer(char const *val, int radix)
: Integer() {
    mp_handle_error_(mp_int_read_string(&num_, radix, val));
}

inline Integer::Integer(std::string const &val, int radix)
: Integer(val.c_str(), radix) { }

inline Integer::Integer(Integer const &a)
: Integer() {
    mp_handle_error_(mp_int_copy(&num_, &a.num_));
}

inline Integer::Integer(Integer &&a) noexcept
: Integer() {
    swap(a);
}

inline Integer &Integer::operator=(Integer const &a) {
    mp_handle_error_(mp_int_init_copy(&num_, &a.num_));
    return *this;
}

inline Integer &Integer::operator=(Integer &&a) noexcept {
    swap(a);
    return *this;
}

inline Integer::~Integer() noexcept {
    mp_int_clear(&num_);
}

inline void Integer::swap(Integer &x) noexcept {
    mp_int_swap(&num_, &x.num_);
}

inline Integer &Integer::add_mul(Integer const &a, Integer const &b) & {
    return *this += a * b;
}

inline Integer Integer::add_mul(Integer const &a, Integer const &b) && {
    return std::move(*this += a * b);
}

inline Integer &Integer::neg() {
    mp_handle_error_(mp_int_neg(&num_, &num_));
    return *this;
}

inline mpz_t &Integer::impl() const {
    return num_;
}

// addition

[[nodiscard]] inline Integer operator+(Integer const &a, fixed_int b) {
    Integer c;
    mp_handle_error_(mp_int_add_value(&a.num_, b, &c.num_));
    return c;
}

[[nodiscard]] inline Integer operator+(Integer const &a, Integer const &b) {
    Integer c;
    mp_handle_error_(mp_int_add(&a.num_, &b.num_, &c.num_));
    return c;
}

[[nodiscard]] inline Integer operator+(Integer &&a, fixed_int b) {
    return std::move(a += b);
}

[[nodiscard]] inline Integer operator+(Integer &&a, Integer const &b) {
    return std::move(a += b);
}

inline Integer &operator+=(Integer &a, fixed_int b) {
    return a = a + b;
}

inline Integer &operator+=(Integer &a, Integer const &b) {
    return a = a + b;
}

// subtraction

[[nodiscard]] inline Integer operator-(Integer a) {
    a.neg();
    return a;
}

[[nodiscard]] inline Integer operator-(Integer const &a, fixed_int b) {
    Integer c;
    mp_handle_error_(mp_int_sub_value(&a.num_, b, &c.num_));
    return c;
}

[[nodiscard]] inline Integer operator-(Integer const &a, Integer const &b) {
    Integer c;
    mp_handle_error_(mp_int_sub(&a.num_, &b.num_, &c.num_));
    return c;
}

[[nodiscard]] inline Integer operator-(Integer &&a, fixed_int b) {
    return std::move(a -= b);
}

[[nodiscard]] inline Integer operator-(Integer &&a, Integer const &b) {
    return std::move(a -= b);
}

inline Integer &operator-=(Integer &a, fixed_int b) {
    return a = a + b;
}

inline Integer &operator-=(Integer &a, Integer const &b) {
    return a = a + b;
}

// multiplication

[[nodiscard]] inline Integer operator*(Integer const &a, fixed_int b) {
    Integer c;
    mp_handle_error_(mp_int_mul_value(&a.num_, b, &c.num_));
    return c;
}

[[nodiscard]] inline Integer operator*(Integer const &a, Integer const &b) {
    Integer c;
    mp_handle_error_(mp_int_mul(&a.num_, &b.num_, &c.num_));
    return c;
}

[[nodiscard]] inline Integer operator*(Integer &&a, fixed_int b) {
    return std::move(a *= b);
}

[[nodiscard]] inline Integer operator*(Integer &&a, Integer const &b) {
    return std::move(a *= b);
}

inline Integer &operator*=(Integer &a, fixed_int b) {
    return a = a * b;
}

inline Integer &operator*=(Integer &a, Integer const &b) {
    return a = a * b;
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
    return compare(a, b) != 0;
}

[[nodiscard]] inline bool operator==(Integer const &a, Integer const &b) {
    return compare(a, b) != 0;
}

// not equal to

[[nodiscard]] inline bool operator!=(Integer const &a, fixed_int b) {
    return compare(a, b) == 0;
}

[[nodiscard]] inline bool operator!=(Integer const &a, Integer const &b) {
    return compare(a, b) == 0;
}

// printing

inline std::ostream &operator<<(std::ostream &out, Integer const &a) {
    auto len = mp_int_string_len(&a.num_, BASE);
    std::unique_ptr<char[]> buf{new char[len]}; // NOLINT
    mp_handle_error_(mp_int_to_string(&a.num_, BASE, buf.get(), len));
    out << buf.get();
    return out;
}

// comparison

[[nodiscard]] inline int compare(Integer const &a, fixed_int b) {
    return mp_int_compare_value(&a.num_, b);
}

[[nodiscard]] inline int compare(Integer const &a, Integer const &b) {
    return mp_int_compare(&a.num_, &b.num_);
}

// gcd

[[nodiscard]] inline std::tuple<Integer, Integer, Integer> gcd(Integer const &a, Integer const &b) {
    Integer g;
    Integer r;
    Integer ga;
    Integer gb;
    mp_int_gcd(&a.num_, &b.num_, &g.num_);
    mp_int_div(&a.num_, &g.num_, &ga.num_, &r.num_);
    mp_int_div(&b.num_, &g.num_, &gb.num_, &r.num_);
    return {g, ga, gb};
}

// Rational

inline Rational::Rational() noexcept { // NOLINT
    mp_rat_init(&num_);
}

inline Rational::Rational(fixed_int val)
: Rational() {
    mp_handle_error_(mp_rat_set_value(&num_, val, 1));
}

inline Rational::Rational(char const *val, int radix)
: Rational() {
    mp_handle_error_(mp_rat_read_string(&num_, radix, val));
}

inline Rational::Rational(std::string const &val, int radix)
: Rational(val.c_str(), radix) {
}

inline Rational::Rational(Rational const &a)
: Rational() {
    mp_handle_error_(mp_rat_copy(&num_, &a.num_));
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
    mp_handle_error_(mp_rat_copy(&num_, &a.num_));
    return *this;
}

inline Rational &Rational::operator=(Rational &&a) noexcept {
    swap(a);
    return *this;
}

inline Rational::~Rational() noexcept {
    mp_rat_clear(&num_);
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
    mp_handle_error_(mp_rat_neg(&num_, &num_));
    return *this;
}

inline void Rational::swap(Rational &x) noexcept {
    mp_int_swap(&num_.num, &x.num_.num);
    mp_int_swap(&num_.den, &x.num_.den);
}

inline void Rational::canonicalize() {
    mp_handle_error_(mp_rat_reduce(&num_));
}
// addition

[[nodiscard]] inline Rational operator+(Rational const &a, fixed_int b) {
#if __FLINT_RELEASE >= 20600
    Rational c;
    fmpq_add_si(&c.num_, &a.num_, &b.num_);
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
    fmpq_sub_si(&c.num_, &a.num_, &b.num_);
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
    fmpq_mul_si(&c.num_, &a.num_, &b.num_);
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
#if __FLINT_RELEASE >= 20600
    Rational c;
    fmpq_div_si(&c.num_, &a.num_, &b.num_);
    return c;
#else
    return a / Rational{b};
#endif
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
#if __FLINT_RELEASE >= 20600
    fmpq_div_si(&a.num_, &a.num_, b);
    return a;
#else
    return a /= Rational{b};
#endif
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
        return fmpq_equal_fmpz(&a.num_, b) != 0;
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
/*

class Rational {
public:
    Rational() { // NOLINT
        mp_handle_error_(mp_rat_init(&num_));
    }
    Rational(mp_small val)
    : Rational() {
        mp_handle_error_(mp_rat_set_value(&num_, val, 1));
    }
    Rational(char const *val, mp_size radix)
    : Rational() {
        mp_handle_error_(mp_rat_read_string(&num_, radix, val));
    }
    Rational(std::string const &val, mp_size radix)
    : Rational(val.c_str(), radix) {
    }
    Rational(Rational const &a)
    : Rational() {
        mp_handle_error_(mp_rat_copy(&a.num_, &num_));
    }
    Rational(Rational &&a) noexcept
    : Rational() {
        swap(a);
    }
    Rational &operator=(Rational const &a) {
        mp_handle_error_(mp_rat_copy(&a.num_, &num_));
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
        mp_handle_error_(mp_rat_reduce(&num_));
    }

    friend Rational operator*(Rational const &a, Rational const &b) {
        Rational c;
        mp_handle_error_(mp_rat_mul(&a.num_, &b.num_, &c.num_));
        return c;
    }

    friend Rational &operator*=(Rational &a, Rational const &b) {
        Rational c;
        mp_handle_error_(mp_rat_mul(&a.num_, &b.num_, &c.num_));
        c.swap(a);
        return a;
    }

    friend Rational operator/(Rational const &a, Rational const &b) {
        Rational c;
        mp_handle_error_(mp_rat_div(&a.num_, &b.num_, &c.num_));
        return c;
    }

    friend Rational &operator/=(Rational &a, Rational const &b) {
        mp_handle_error_(mp_rat_div(&a.num_, &b.num_, &a.num_));
        return a;
    }

    friend Rational operator+(Rational const &a, Rational const &b) {
        Rational c;
        mp_handle_error_(mp_rat_add(&a.num_, &b.num_, &c.num_));
        return c;
    }
    friend Rational &operator+=(Rational &a, Rational const &b) {
        Rational c;
        mp_handle_error_(mp_rat_add(&a.num_, &b.num_, &c.num_));
        c.swap(a);
        return a;
    }

    friend Rational operator-(Rational const &a) {
        Rational b;
        mp_handle_error_(mp_rat_neg(&a.num_, &b.num_));
        return b;
    }
    friend Rational operator-(Rational const &a, Rational const &b) {
        Rational c;
        mp_handle_error_(mp_rat_sub(&a.num_, &b.num_, &c.num_));
        return c;
    }
    friend Rational &operator-=(Rational &a, Rational const &b) {
        Rational c;
        mp_handle_error_(mp_rat_sub(&a.num_, &b.num_, &c.num_));
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
            mp_handle_error_(mp_int_to_string(mp_rat_numer_ref(&a.num_), radix, buf.get(), len));
            out << buf.get();
        }
        else {
            auto len = mp_rat_string_len(&a.num_, radix);
            auto buf = std::make_unique<char[]>(len); // NOLINT
            mp_handle_error_(mp_rat_to_string(&a.num_, radix, buf.get(), len));
            out << buf.get();
        }
        return out;
    }

private:
    static void mp_handle_error_(mp_result res) {
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

*/


#endif
