#pragma once

#ifdef CLINGOLPX_USE_FLINT

#include <flint/fmpq.h>
#include <flint/fmpz.h>

#include <ios>
#include <iostream>
#include <memory>
#include <new>
#include <stdexcept>
#include <string>

constexpr int BASE = 10;

using fixed_int = slong;

class Integer {
    friend auto operator+(Integer const &a, fixed_int b) -> Integer;
    friend auto operator+(Integer const &a, Integer const &b) -> Integer;
    friend auto operator+(Integer &&a, fixed_int b) -> Integer;
    friend auto operator+(Integer &&a, Integer const &b) -> Integer;
    friend auto operator+=(Integer &a, fixed_int b) -> Integer &;
    friend auto operator+=(Integer &a, Integer const &b) -> Integer &;

    friend auto operator-(Integer a) -> Integer;

    friend auto operator-(Integer const &a, fixed_int b) -> Integer;
    friend auto operator-(Integer const &a, Integer const &b) -> Integer;
    friend auto operator-(Integer &&a, fixed_int b) -> Integer;
    friend auto operator-(Integer &&a, Integer const &b) -> Integer;
    friend auto operator-=(Integer &a, fixed_int b) -> Integer &;
    friend auto operator-=(Integer &a, Integer const &b) -> Integer &;

    friend auto operator*(Integer const &a, fixed_int b) -> Integer;
    friend auto operator*(Integer const &a, Integer const &b) -> Integer;
    friend auto operator*(Integer &&a, fixed_int b) -> Integer;
    friend auto operator*(Integer &&a, Integer const &b) -> Integer;
    friend auto operator*=(Integer &a, fixed_int b) -> Integer &;
    friend auto operator*=(Integer &a, Integer const &b) -> Integer &;

    friend auto operator<(Integer const &a, fixed_int b) -> bool;
    friend auto operator<(Integer const &a, Integer const &b) -> bool;

    friend auto operator<=(Integer const &a, fixed_int b) -> bool;
    friend auto operator<=(Integer const &a, Integer const &b) -> bool;

    friend auto operator>(Integer const &a, fixed_int b) -> bool;
    friend auto operator>(Integer const &a, Integer const &b) -> bool;

    friend auto operator>=(Integer const &a, fixed_int b) -> bool;
    friend auto operator>=(Integer const &a, Integer const &b) -> bool;

    friend auto operator==(Integer const &a, fixed_int b) -> bool;
    friend auto operator==(Integer const &a, Integer const &b) -> bool;

    friend auto operator!=(Integer const &a, fixed_int b) -> bool;
    friend auto operator!=(Integer const &a, Integer const &b) -> bool;

    friend auto operator<<(std::ostream &out, Integer const &a) -> std::ostream &;

    friend auto compare(Integer const &a, fixed_int b) -> int;
    friend auto compare(Integer const &a, Integer const &b) -> int;

    friend auto gcd(Integer const &a, Integer const &b) -> Integer;
    friend auto gcd_div(Integer const &a, Integer const &b) -> std::tuple<Integer, Integer, Integer>;

  public:
    Integer() noexcept;
    Integer(fixed_int val);
    Integer(char const *val, int radix);
    Integer(std::string const &val, int radix);
    Integer(Integer const &a);
    Integer(Integer &&a) noexcept;
    auto operator=(Integer const &a) -> Integer &;
    auto operator=(Integer &&a) noexcept -> Integer &;
    ~Integer() noexcept;

    void swap(Integer &x) noexcept;
    auto divide(Integer const &a) -> Integer &;
    auto add_mul(Integer const &a, Integer const &b) & -> Integer &;
    auto add_mul(Integer const &a, Integer const &b) && -> Integer;
    auto neg() -> Integer &;
    [[nodiscard]] auto impl() const -> fmpz &;

  private:
    mutable fmpz num_;
};

class Rational {
  private:
    friend auto operator+(Rational const &a, fixed_int b) -> Rational;
    friend auto operator+(Rational const &a, Integer const &b) -> Rational;
    friend auto operator+(Rational const &a, Rational const &b) -> Rational;
    friend auto operator+(Rational &&a, fixed_int b) -> Rational;
    friend auto operator+(Rational &&a, Integer const &b) -> Rational;
    friend auto operator+(Rational &&a, Rational const &b) -> Rational;
    friend auto operator+=(Rational &a, fixed_int b) -> Rational &;
    friend auto operator+=(Rational &a, Integer const &b) -> Rational &;
    friend auto operator+=(Rational &a, Rational const &b) -> Rational &;

    friend auto operator-(Rational a) -> Rational;

    friend auto operator-(Rational const &a, fixed_int b) -> Rational;
    friend auto operator-(Rational const &a, Integer const &b) -> Rational;
    friend auto operator-(Rational const &a, Rational const &b) -> Rational;
    friend auto operator-(Rational &&a, fixed_int b) -> Rational;
    friend auto operator-(Rational &&a, Integer const &b) -> Rational;
    friend auto operator-(Rational &&a, Rational const &b) -> Rational;
    friend auto operator-=(Rational &a, fixed_int b) -> Rational &;
    friend auto operator-=(Rational &a, Integer const &b) -> Rational &;
    friend auto operator-=(Rational &a, Rational const &b) -> Rational &;

    friend auto operator*(Rational const &a, fixed_int b) -> Rational;
    friend auto operator*(Rational const &a, Integer const &b) -> Rational;
    friend auto operator*(Rational const &a, Rational const &b) -> Rational;
    friend auto operator*(Rational &&a, fixed_int b) -> Rational;
    friend auto operator*(Rational &&a, Integer const &b) -> Rational;
    friend auto operator*(Rational &&a, Rational const &b) -> Rational;
    friend auto operator*=(Rational &a, fixed_int b) -> Rational &;
    friend auto operator*=(Rational &a, Integer const &b) -> Rational &;
    friend auto operator*=(Rational &a, Rational const &b) -> Rational &;

    friend auto operator/(Rational const &a, fixed_int b) -> Rational;
    friend auto operator/(Rational const &a, Integer const &b) -> Rational;
    friend auto operator/(Rational const &a, Rational const &b) -> Rational;
    friend auto operator/(Rational &&a, fixed_int b) -> Rational;
    friend auto operator/(Rational &&a, Integer const &b) -> Rational;
    friend auto operator/(Rational &&a, Rational const &b) -> Rational;
    friend auto operator/=(Rational &a, fixed_int b) -> Rational &;
    friend auto operator/=(Rational &a, Integer const &b) -> Rational &;
    friend auto operator/=(Rational &a, Rational const &b) -> Rational &;

    friend auto operator<(Rational const &a, fixed_int b) -> bool;
    friend auto operator<(Rational const &a, Integer const &b) -> bool;
    friend auto operator<(Rational const &a, Rational const &b) -> bool;

    friend auto operator<=(Rational const &a, fixed_int b) -> bool;
    friend auto operator<=(Rational const &a, Integer const &b) -> bool;
    friend auto operator<=(Rational const &a, Rational const &b) -> bool;

    friend auto operator>(Rational const &a, fixed_int b) -> bool;
    friend auto operator>(Rational const &a, Integer const &b) -> bool;
    friend auto operator>(Rational const &a, Rational const &b) -> bool;

    friend auto operator>=(Rational const &a, fixed_int b) -> bool;
    friend auto operator>=(Rational const &a, Integer const &b) -> bool;
    friend auto operator>=(Rational const &a, Rational const &b) -> bool;

    friend auto operator==(Rational const &a, fixed_int b) -> bool;
    friend auto operator==(Rational const &a, Integer const &b) -> bool;
    friend auto operator==(Rational const &a, Rational const &b) -> bool;

    friend auto operator!=(Rational const &a, fixed_int b) -> bool;
    friend auto operator!=(Rational const &a, Integer const &b) -> bool;
    friend auto operator!=(Rational const &a, Rational const &b) -> bool;

    friend auto operator<<(std::ostream &out, Rational const &a) -> std::ostream &;

    friend auto compare(Rational const &a, fixed_int b) -> int;
    friend auto compare(Rational const &a, Integer const &b) -> int;
    friend auto compare(Rational const &a, Rational const &b) -> int;

  public:
    Rational() noexcept;
    Rational(fixed_int val);
    Rational(Integer num, Integer den);
    Rational(char const *val, int radix);
    Rational(std::string const &val, int radix);
    Rational(Rational const &a);
    Rational(Rational &&a) noexcept;
    auto operator=(Rational const &a) -> Rational &;
    auto operator=(Rational &&a) noexcept -> Rational &;
    ~Rational() noexcept;

    [[nodiscard]] auto num() -> Integer &;
    [[nodiscard]] auto num() const -> Integer const &;
    [[nodiscard]] auto den() -> Integer &;
    [[nodiscard]] auto den() const -> Integer const &;
    auto neg() -> Rational &;
    void swap(Rational &x) noexcept;
    void canonicalize();

  private:
    mutable fmpq num_;
};

// Integer

inline Integer::Integer() noexcept { // NOLINT
    fmpz_init(&num_);
}

inline Integer::Integer(fixed_int val) : Integer() { fmpz_set_si(&num_, val); }

inline Integer::Integer(char const *val, int radix) : Integer() {
    if (fmpz_set_str(&num_, val, radix) != 0) {
        throw std::runtime_error("could not parse number");
    }
}

inline Integer::Integer(std::string const &val, int radix) : Integer(val.c_str(), radix) {}

inline Integer::Integer(Integer const &a) : Integer() { fmpz_set(&num_, &a.num_); }

inline Integer::Integer(Integer &&a) noexcept : Integer() { swap(a); }

inline auto Integer::operator=(Integer const &a) -> Integer & {
    fmpz_set(&num_, &a.num_);
    return *this;
}

inline auto Integer::operator=(Integer &&a) noexcept -> Integer & {
    swap(a);
    return *this;
}

inline Integer::~Integer() noexcept { fmpz_clear(&num_); }

inline void Integer::swap(Integer &x) noexcept { fmpz_swap(&num_, &x.num_); }

inline auto Integer::divide(Integer const &a) -> Integer & {
    fmpz_divexact(&num_, &num_, &a.num_);
    return *this;
}

inline auto Integer::add_mul(Integer const &a, Integer const &b) & -> Integer & {
    fmpz_addmul(&num_, &a.num_, &b.num_);
    return *this;
}

inline auto Integer::add_mul(Integer const &a, Integer const &b) && -> Integer {
    return std::move(this->add_mul(a, b));
}

inline auto Integer::neg() -> Integer & {
    fmpz_neg(&num_, &num_);
    return *this;
}

inline auto Integer::impl() const -> fmpz & { return num_; }

// addition

[[nodiscard]] inline auto operator+(Integer const &a, fixed_int b) -> Integer {
#if __FLINT_RELEASE >= 20600
    Integer c;
    fmpz_add_si(&c.num_, &a.num_, b);
    return c;
#else
    return a + Integer{b};
#endif
}

[[nodiscard]] inline auto operator+(Integer const &a, Integer const &b) -> Integer {
    Integer c;
    fmpz_add(&c.num_, &a.num_, &b.num_);
    return c;
}

[[nodiscard]] inline auto operator+(Integer &&a, fixed_int b) -> Integer { return std::move(a += b); }

[[nodiscard]] inline auto operator+(Integer &&a, Integer const &b) -> Integer { return std::move(a += b); }

inline auto operator+=(Integer &a, fixed_int b) -> Integer & {
#if __FLINT_RELEASE >= 20600
    fmpz_add_si(&a.num_, &a.num_, b);
    return a;
#else
    return a += Integer{b};
#endif
}

inline auto operator+=(Integer &a, Integer const &b) -> Integer & {
    fmpz_add(&a.num_, &a.num_, &b.num_);
    return a;
}

// subtraction

[[nodiscard]] inline auto operator-(Integer a) -> Integer {
    a.neg();
    return a;
}

[[nodiscard]] inline auto operator-(Integer const &a, fixed_int b) -> Integer {
#if __FLINT_RELEASE >= 20600
    Integer c;
    fmpz_sub_si(&c.num_, &a.num_, b);
    return c;
#else
    return a - Integer{b};
#endif
}

[[nodiscard]] inline auto operator-(Integer const &a, Integer const &b) -> Integer {
    Integer c;
    fmpz_sub(&c.num_, &a.num_, &b.num_);
    return c;
}

[[nodiscard]] inline auto operator-(Integer &&a, fixed_int b) -> Integer { return std::move(a -= b); }

[[nodiscard]] inline auto operator-(Integer &&a, Integer const &b) -> Integer { return std::move(a -= b); }

inline auto operator-=(Integer &a, fixed_int b) -> Integer & {
#if __FLINT_RELEASE >= 20600
    fmpz_sub_si(&a.num_, &a.num_, b);
    return a;
#else
    return a -= Integer{b};
#endif
}

inline auto operator-=(Integer &a, Integer const &b) -> Integer & {
    fmpz_sub(&a.num_, &a.num_, &b.num_);
    return a;
}

// multiplication

[[nodiscard]] inline auto operator*(Integer const &a, fixed_int b) -> Integer {
    Integer c;
    fmpz_mul_si(&c.num_, &a.num_, b);
    return c;
}

[[nodiscard]] inline auto operator*(Integer const &a, Integer const &b) -> Integer {
    Integer c;
    fmpz_mul(&c.num_, &a.num_, &b.num_);
    return c;
}

[[nodiscard]] inline auto operator*(Integer &&a, fixed_int b) -> Integer { return std::move(a *= b); }

[[nodiscard]] inline auto operator*(Integer &&a, Integer const &b) -> Integer { return std::move(a *= b); }

inline auto operator*=(Integer &a, fixed_int b) -> Integer & {
    fmpz_mul_si(&a.num_, &a.num_, b);
    return a;
}

inline auto operator*=(Integer &a, Integer const &b) -> Integer & {
    fmpz_mul(&a.num_, &a.num_, &b.num_);
    return a;
}

// less than

[[nodiscard]] inline auto operator<(Integer const &a, fixed_int b) -> bool { return compare(a, b) < 0; }

[[nodiscard]] inline auto operator<(Integer const &a, Integer const &b) -> bool { return compare(a, b) < 0; }

// less than or equal to

[[nodiscard]] inline auto operator<=(Integer const &a, fixed_int b) -> bool { return compare(a, b) <= 0; }

[[nodiscard]] inline auto operator<=(Integer const &a, Integer const &b) -> bool { return compare(a, b) <= 0; }

// greater than

[[nodiscard]] inline auto operator>(Integer const &a, fixed_int b) -> bool { return compare(a, b) > 0; }

[[nodiscard]] inline auto operator>(Integer const &a, Integer const &b) -> bool { return compare(a, b) > 0; }

// greater than or equal to

[[nodiscard]] inline auto operator>=(Integer const &a, fixed_int b) -> bool { return compare(a, b) >= 0; }

[[nodiscard]] inline auto operator>=(Integer const &a, Integer const &b) -> bool { return compare(a, b) >= 0; }

// greater equal to

[[nodiscard]] inline auto operator==(Integer const &a, fixed_int b) -> bool { return fmpz_equal_si(&a.num_, b) != 0; }

[[nodiscard]] inline auto operator==(Integer const &a, Integer const &b) -> bool {
    return fmpz_equal(&a.num_, &b.num_) != 0;
}

// not equal to

[[nodiscard]] inline auto operator!=(Integer const &a, fixed_int b) -> bool { return !(a == b); }

[[nodiscard]] inline auto operator!=(Integer const &a, Integer const &b) -> bool { return !(a == b); }

// printing

inline auto operator<<(std::ostream &out, Integer const &a) -> std::ostream & {
    std::unique_ptr<char, decltype(flint_free) *> buf{fmpz_get_str(nullptr, BASE, &a.num_), flint_free};
    if (buf == nullptr) {
        throw std::bad_alloc();
    }
    out << buf.get();
    return out;
}

// comparison

[[nodiscard]] inline auto compare(Integer const &a, fixed_int b) -> int { return fmpz_cmp_si(&a.num_, b); }

[[nodiscard]] inline auto compare(Integer const &a, Integer const &b) -> int { return fmpz_cmp_si(&a.num_, b.num_); }

// gcd

[[nodiscard]] inline auto gcd(Integer const &a, Integer const &b) -> Integer {
    Integer g;
    fmpz_gcd(&g.num_, &a.num_, &b.num_);
    return g;
}

[[nodiscard]] inline auto gcd_div(Integer const &a, Integer const &b) -> std::tuple<Integer, Integer, Integer> {
    std::tuple<Integer, Integer, Integer> ret;
    fmpz_gcd(&std::get<0>(ret).num_, &a.num_, &b.num_);
    fmpz_divexact(&std::get<1>(ret).num_, &a.num_, &std::get<0>(ret).num_);
    fmpz_divexact(&std::get<2>(ret).num_, &b.num_, &std::get<0>(ret).num_);
    return ret;
}

// Rational

inline Rational::Rational() noexcept { // NOLINT
    fmpq_init(&num_);
}

inline Rational::Rational(fixed_int val) : Rational() { fmpq_set_si(&num_, val, 1); }

inline Rational::Rational(char const *val, int radix) : Rational() {
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
    } else {
        if (fmpz_set_str(&num_.num, buf.substr(0, pos).c_str(), radix) != 0) {
            throw std::runtime_error("could not parse number");
        }
        if (fmpz_set_str(&num_.den, buf.substr(pos + 1).c_str(), radix) != 0) {
            throw std::runtime_error("could not parse number");
        }
        fmpq_canonicalise(&num_);
    }
#endif
}

inline Rational::Rational(std::string const &val, int radix) : Rational(val.c_str(), radix) {}

inline Rational::Rational(Rational const &a) : Rational() { fmpq_set(&num_, &a.num_); }

inline Rational::Rational(Integer num, Integer den) : Rational() {
    this->num() = std::move(num);
    this->den() = std::move(den);
    canonicalize();
}

inline Rational::Rational(Rational &&a) noexcept : Rational() { swap(a); }

inline auto Rational::operator=(Rational const &a) -> Rational & {
    fmpq_set(&num_, &a.num_);
    return *this;
}

inline auto Rational::operator=(Rational &&a) noexcept -> Rational & {
    swap(a);
    return *this;
}

inline Rational::~Rational() noexcept { fmpq_clear(&num_); }

inline auto Rational::num() -> Integer & {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    return reinterpret_cast<Integer &>(num_.num);
}

inline auto Rational::num() const -> Integer const & {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    return reinterpret_cast<Integer const &>(num_.num);
}

inline auto Rational::den() -> Integer & {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    return reinterpret_cast<Integer &>(num_.den);
}

inline auto Rational::den() const -> Integer const & {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    return reinterpret_cast<Integer const &>(num_.den);
}

inline auto Rational::neg() -> Rational & {
    fmpq_neg(&num_, &num_);
    return *this;
}

inline void Rational::swap(Rational &x) noexcept { fmpq_swap(&num_, &x.num_); }

inline void Rational::canonicalize() { fmpq_canonicalise(&num_); }
// addition

[[nodiscard]] inline auto operator+(Rational const &a, fixed_int b) -> Rational {
#if __FLINT_RELEASE >= 20600
    Rational c;
    fmpq_add_si(&c.num_, &a.num_, b);
    return c;
#else
    return a + Rational{b};
#endif
}

[[nodiscard]] inline auto operator+(Rational const &a, Integer const &b) -> Rational {
    Rational c;
    fmpq_add_fmpz(&c.num_, &a.num_, &b.impl());
    return c;
}

[[nodiscard]] inline auto operator+(Rational const &a, Rational const &b) -> Rational {
    Rational c;
    fmpq_add(&c.num_, &a.num_, &b.num_);
    return c;
}

[[nodiscard]] inline auto operator+(Rational &&a, fixed_int b) -> Rational { return std::move(a += b); }

[[nodiscard]] inline auto operator+(Rational &&a, Integer const &b) -> Rational { return std::move(a += b); }

[[nodiscard]] inline auto operator+(Rational &&a, Rational const &b) -> Rational { return std::move(a += b); }

inline auto operator+=(Rational &a, fixed_int b) -> Rational & {
    fmpq_add_si(&a.num_, &a.num_, b);
    return a;
}

inline auto operator+=(Rational &a, Integer const &b) -> Rational & {
    fmpq_add_fmpz(&a.num_, &a.num_, &b.impl());
    return a;
}

inline auto operator+=(Rational &a, Rational const &b) -> Rational & {
    fmpq_add(&a.num_, &a.num_, &b.num_);
    return a;
}

// subtraction

[[nodiscard]] inline auto operator-(Rational a) -> Rational {
    a.neg();
    return a;
}

[[nodiscard]] inline auto operator-(Rational const &a, fixed_int b) -> Rational {
#if __FLINT_RELEASE >= 20600
    Rational c;
    fmpq_sub_si(&c.num_, &a.num_, b);
    return c;
#else
    return a - Rational{b};
#endif
}

[[nodiscard]] inline auto operator-(Rational const &a, Integer const &b) -> Rational {
    Rational c;
    fmpq_sub_fmpz(&c.num_, &a.num_, &b.impl());
    return c;
}

[[nodiscard]] inline auto operator-(Rational const &a, Rational const &b) -> Rational {
    Rational c;
    fmpq_sub(&c.num_, &a.num_, &b.num_);
    return c;
}

[[nodiscard]] inline auto operator-(Rational &&a, fixed_int b) -> Rational { return std::move(a -= b); }

[[nodiscard]] inline auto operator-(Rational &&a, Integer const &b) -> Rational { return std::move(a -= b); }

[[nodiscard]] inline auto operator-(Rational &&a, Rational const &b) -> Rational { return std::move(a -= b); }

inline auto operator-=(Rational &a, fixed_int b) -> Rational & {
    fmpq_sub_si(&a.num_, &a.num_, b);
    return a;
}

inline auto operator-=(Rational &a, Integer const &b) -> Rational & {
    fmpq_sub_fmpz(&a.num_, &a.num_, &b.impl());
    return a;
}

inline auto operator-=(Rational &a, Rational const &b) -> Rational & {
    fmpq_sub(&a.num_, &a.num_, &b.num_);
    return a;
}

// multiplication

[[nodiscard]] inline auto operator*(Rational const &a, fixed_int b) -> Rational {
#if __FLINT_RELEASE >= 20600
    Rational c;
    fmpq_mul_si(&c.num_, &a.num_, b);
    return c;
#else
    return a * Rational{b};
#endif
}

[[nodiscard]] inline auto operator*(Rational const &a, Integer const &b) -> Rational {
    Rational c;
    fmpq_mul_fmpz(&c.num_, &a.num_, &b.impl());
    return c;
}

[[nodiscard]] inline auto operator*(Rational const &a, Rational const &b) -> Rational {
    Rational c;
    fmpq_mul(&c.num_, &a.num_, &b.num_);
    return c;
}

[[nodiscard]] inline auto operator*(Rational &&a, fixed_int b) -> Rational { return std::move(a *= b); }

[[nodiscard]] inline auto operator*(Rational &&a, Integer const &b) -> Rational { return std::move(a *= b); }

[[nodiscard]] inline auto operator*(Rational &&a, Rational const &b) -> Rational { return std::move(a *= b); }

inline auto operator*=(Rational &a, fixed_int b) -> Rational & {
#if __FLINT_RELEASE >= 20600
    fmpq_mul_si(&a.num_, &a.num_, b);
    return a;
#else
    return a *= Rational{b};
#endif
}

inline auto operator*=(Rational &a, Integer const &b) -> Rational & {
    fmpq_mul_fmpz(&a.num_, &a.num_, &b.impl());
    return a;
}

inline auto operator*=(Rational &a, Rational const &b) -> Rational & {
    fmpq_mul(&a.num_, &a.num_, &b.num_);
    return a;
}

// division

[[nodiscard]] inline auto operator/(Rational const &a, fixed_int b) -> Rational { return a / Integer{b}; }

[[nodiscard]] inline auto operator/(Rational const &a, Integer const &b) -> Rational {
    Rational c;
    fmpq_div_fmpz(&c.num_, &a.num_, &b.impl());
    return c;
}

[[nodiscard]] inline auto operator/(Rational const &a, Rational const &b) -> Rational {
    Rational c;
    fmpq_div(&c.num_, &a.num_, &b.num_);
    return c;
}

[[nodiscard]] inline auto operator/(Rational &&a, fixed_int b) -> Rational { return std::move(a /= b); }

[[nodiscard]] inline auto operator/(Rational &&a, Integer const &b) -> Rational { return std::move(a /= b); }

[[nodiscard]] inline auto operator/(Rational &&a, Rational const &b) -> Rational { return std::move(a /= b); }

inline auto operator/=(Rational &a, fixed_int b) -> Rational & { return a /= Integer{b}; }

inline auto operator/=(Rational &a, Integer const &b) -> Rational & {
    fmpq_div_fmpz(&a.num_, &a.num_, &b.impl());
    return a;
}

inline auto operator/=(Rational &a, Rational const &b) -> Rational & {
    fmpq_div(&a.num_, &a.num_, &b.num_);
    return a;
}

// less than

[[nodiscard]] inline auto operator<(Rational const &a, fixed_int b) -> bool { return compare(a, b) < 0; }

[[nodiscard]] inline auto operator<(Rational const &a, Integer const &b) -> bool { return compare(a, b) < 0; }

[[nodiscard]] inline auto operator<(Rational const &a, Rational const &b) -> bool { return compare(a, b) < 0; }

// less than or equal to

[[nodiscard]] inline auto operator<=(Rational const &a, fixed_int b) -> bool { return compare(a, b) <= 0; }

[[nodiscard]] inline auto operator<=(Rational const &a, Integer const &b) -> bool { return compare(a, b) <= 0; }

[[nodiscard]] inline auto operator<=(Rational const &a, Rational const &b) -> bool { return compare(a, b) <= 0; }

// greater than

[[nodiscard]] inline auto operator>(Rational const &a, fixed_int b) -> bool { return compare(a, b) > 0; }

[[nodiscard]] inline auto operator>(Rational const &a, Integer const &b) -> bool { return compare(a, b) > 0; }

[[nodiscard]] inline auto operator>(Rational const &a, Rational const &b) -> bool { return compare(a, b) > 0; }

// greater than or equal to

[[nodiscard]] inline auto operator>=(Rational const &a, fixed_int b) -> bool { return compare(a, b) >= 0; }

[[nodiscard]] inline auto operator>=(Rational const &a, Integer const &b) -> bool { return compare(a, b) >= 0; }

[[nodiscard]] inline auto operator>=(Rational const &a, Rational const &b) -> bool { return compare(a, b) >= 0; }

// equal to

[[nodiscard]] inline auto operator==(Rational const &a, fixed_int b) -> bool {
#if __FLINT_RELEASE >= 20600
    return fmpq_equal_si(&a.num_, b) != 0;
#else
    return a == Rational{b};
#endif
}

[[nodiscard]] inline auto operator==(Rational const &a, Integer const &b) -> bool {
#if __FLINT_RELEASE >= 20600
    return fmpq_equal_fmpz(&a.num_, &b.impl()) != 0;
#else
    return a == Rational{b, Integer{1}};
#endif
}

[[nodiscard]] inline auto operator==(Rational const &a, Rational const &b) -> bool {
    return fmpq_equal(&a.num_, &b.num_) != 0;
}

// not equal to

[[nodiscard]] inline auto operator!=(Rational const &a, fixed_int b) -> bool { return !(a == b); }

[[nodiscard]] inline auto operator!=(Rational const &a, Integer const &b) -> bool { return !(a == b); }

[[nodiscard]] inline auto operator!=(Rational const &a, Rational const &b) -> bool { return !(a == b); }

// printing

inline auto operator<<(std::ostream &out, Rational const &a) -> std::ostream & {
    std::unique_ptr<char, decltype(flint_free) *> buf{fmpq_get_str(nullptr, BASE, &a.num_), flint_free};
    if (buf == nullptr) {
        throw std::bad_alloc();
    }
    out << buf.get();
    return out;
}

// comparison

[[nodiscard]] inline auto compare(Rational const &a, fixed_int b) -> int {
#if __FLINT_RELEASE >= 20600
    return fmpq_cmp_si(&a.num_, b);
#else
    return compare(a, Rational{b});
#endif
}

[[nodiscard]] inline auto compare(Rational const &a, Integer const &b) -> int {
#if __FLINT_RELEASE >= 20600
    return fmpq_cmp_fmpz(&a.num_, &b.impl());
#else
    return compare(a, Rational{b, 1});
#endif
}

[[nodiscard]] inline auto compare(Rational const &a, Rational const &b) -> int { return fmpq_cmp(&a.num_, &b.num_); }

#endif
