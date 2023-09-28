#pragma once

#ifdef CLINGOLPX_USE_IMATH

#include <imath.h>
#include <imrat.h>

#include <ios>
#include <iostream>
#include <memory>
#include <new>
#include <stdexcept>
#include <string>

constexpr int BASE = 10;

using fixed_int = mp_small;

class Integer {
  private:
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
    [[nodiscard]] auto impl() const -> mpz_t &;

  private:
    mutable mpz_t num_;
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

inline Integer::Integer(fixed_int val) : Integer() { mp_handle_error_(mp_int_set_value(&num_, val)); }

inline Integer::Integer(char const *val, int radix) : Integer() {
    mp_handle_error_(mp_int_read_string(&num_, radix, val));
}

inline Integer::Integer(std::string const &val, int radix) : Integer(val.c_str(), radix) {}

inline Integer::Integer(Integer const &a) : Integer() { mp_handle_error_(mp_int_copy(&a.num_, &num_)); }

inline Integer::Integer(Integer &&a) noexcept : Integer() { swap(a); }

inline auto Integer::operator=(Integer const &a) -> Integer & {
    mp_handle_error_(mp_int_copy(&a.num_, &num_));
    return *this;
}

inline auto Integer::operator=(Integer &&a) noexcept -> Integer & {
    swap(a);
    return *this;
}

inline Integer::~Integer() noexcept { mp_int_clear(&num_); }

inline void Integer::swap(Integer &x) noexcept { mp_int_swap(&num_, &x.num_); }

inline auto Integer::divide(Integer const &a) -> Integer & {
    mp_int_div(&num_, &a.num_, &num_, nullptr);
    return *this;
}

inline auto Integer::add_mul(Integer const &a, Integer const &b) & -> Integer & { return *this += a * b; }

inline auto Integer::add_mul(Integer const &a, Integer const &b) && -> Integer { return std::move(*this += a * b); }

inline auto Integer::neg() -> Integer & {
    mp_handle_error_(mp_int_neg(&num_, &num_));
    return *this;
}

inline auto Integer::impl() const -> mpz_t & { return num_; }

// addition

[[nodiscard]] inline auto operator+(Integer const &a, fixed_int b) -> Integer {
    Integer c;
    mp_handle_error_(mp_int_add_value(&a.num_, b, &c.num_));
    return c;
}

[[nodiscard]] inline auto operator+(Integer const &a, Integer const &b) -> Integer {
    Integer c;
    mp_handle_error_(mp_int_add(&a.num_, &b.num_, &c.num_));
    return c;
}

[[nodiscard]] inline auto operator+(Integer &&a, fixed_int b) -> Integer { return std::move(a += b); }

[[nodiscard]] inline auto operator+(Integer &&a, Integer const &b) -> Integer { return std::move(a += b); }

inline auto operator+=(Integer &a, fixed_int b) -> Integer & {
    mp_handle_error_(mp_int_add_value(&a.num_, b, &a.num_));
    return a;
}

inline auto operator+=(Integer &a, Integer const &b) -> Integer & {
    mp_handle_error_(mp_int_add(&a.num_, &b.num_, &a.num_));
    return a;
}

// subtraction

[[nodiscard]] inline auto operator-(Integer a) -> Integer {
    a.neg();
    return a;
}

[[nodiscard]] inline auto operator-(Integer const &a, fixed_int b) -> Integer {
    Integer c;
    mp_handle_error_(mp_int_sub_value(&a.num_, b, &c.num_));
    return c;
}

[[nodiscard]] inline auto operator-(Integer const &a, Integer const &b) -> Integer {
    Integer c;
    mp_handle_error_(mp_int_sub(&a.num_, &b.num_, &c.num_));
    return c;
}

[[nodiscard]] inline auto operator-(Integer &&a, fixed_int b) -> Integer { return std::move(a -= b); }

[[nodiscard]] inline auto operator-(Integer &&a, Integer const &b) -> Integer { return std::move(a -= b); }

inline auto operator-=(Integer &a, fixed_int b) -> Integer & {
    mp_handle_error_(mp_int_sub_value(&a.num_, b, &a.num_));
    return a;
}

inline auto operator-=(Integer &a, Integer const &b) -> Integer & {
    mp_handle_error_(mp_int_sub(&a.num_, &b.num_, &a.num_));
    return a;
}

// multiplication

[[nodiscard]] inline auto operator*(Integer const &a, fixed_int b) -> Integer {
    Integer c;
    mp_handle_error_(mp_int_mul_value(&a.num_, b, &c.num_));
    return c;
}

[[nodiscard]] inline auto operator*(Integer const &a, Integer const &b) -> Integer {
    Integer c;
    mp_handle_error_(mp_int_mul(&a.num_, &b.num_, &c.num_));
    return c;
}

[[nodiscard]] inline auto operator*(Integer &&a, fixed_int b) -> Integer { return std::move(a *= b); }

[[nodiscard]] inline auto operator*(Integer &&a, Integer const &b) -> Integer { return std::move(a *= b); }

inline auto operator*=(Integer &a, fixed_int b) -> Integer & {
    mp_handle_error_(mp_int_mul_value(&a.num_, b, &a.num_));
    return a;
}

inline auto operator*=(Integer &a, Integer const &b) -> Integer & {
    mp_handle_error_(mp_int_mul(&a.num_, &b.num_, &a.num_));
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

[[nodiscard]] inline auto operator==(Integer const &a, fixed_int b) -> bool { return compare(a, b) == 0; }

[[nodiscard]] inline auto operator==(Integer const &a, Integer const &b) -> bool { return compare(a, b) == 0; }

// not equal to

[[nodiscard]] inline auto operator!=(Integer const &a, fixed_int b) -> bool { return compare(a, b) != 0; }

[[nodiscard]] inline auto operator!=(Integer const &a, Integer const &b) -> bool { return compare(a, b) != 0; }

// printing

inline auto operator<<(std::ostream &out, Integer const &a) -> std::ostream & {
    auto len = mp_int_string_len(&a.num_, BASE);
    std::unique_ptr<char[]> buf{std::make_unique<char[]>(len)}; // NOLINT
    mp_handle_error_(mp_int_to_string(&a.num_, BASE, buf.get(), len));
    out << buf.get();
    return out;
}

// comparison

[[nodiscard]] inline auto compare(Integer const &a, fixed_int b) -> int { return mp_int_compare_value(&a.num_, b); }

[[nodiscard]] inline auto compare(Integer const &a, Integer const &b) -> int {
    return mp_int_compare(&a.num_, &b.num_);
}

// gcd

[[nodiscard]] inline auto gcd(Integer const &a, Integer const &b) -> Integer {
    Integer g;
    mp_int_gcd(&a.num_, &b.num_, &g.num_);
    return g;
}

[[nodiscard]] inline auto gcd_div(Integer const &a, Integer const &b) -> std::tuple<Integer, Integer, Integer> {
    std::tuple<Integer, Integer, Integer> ret;
    mp_int_gcd(&a.num_, &b.num_, &std::get<0>(ret).num_);
    mp_int_div(&a.num_, &std::get<0>(ret).num_, &std::get<1>(ret).num_, nullptr);
    mp_int_div(&b.num_, &std::get<0>(ret).num_, &std::get<2>(ret).num_, nullptr);
    return ret;
}

// Rational

inline Rational::Rational() noexcept { // NOLINT
    mp_rat_init(&num_);
}

inline Rational::Rational(fixed_int val) : Rational() { mp_handle_error_(mp_rat_set_value(&num_, val, 1)); }

inline Rational::Rational(char const *val, int radix) : Rational() {
    mp_handle_error_(mp_rat_read_string(&num_, radix, val));
}

inline Rational::Rational(std::string const &val, int radix) : Rational(val.c_str(), radix) {}

inline Rational::Rational(Rational const &a) : Rational() { mp_handle_error_(mp_rat_copy(&a.num_, &num_)); }

inline Rational::Rational(Integer num, Integer den) : Rational() {
    this->num() = std::move(num);
    this->den() = std::move(den);
    canonicalize();
}

inline Rational::Rational(Rational &&a) noexcept : Rational() { swap(a); }

inline auto Rational::operator=(Rational const &a) -> Rational & {
    mp_handle_error_(mp_rat_copy(&a.num_, &num_));
    return *this;
}

inline auto Rational::operator=(Rational &&a) noexcept -> Rational & {
    swap(a);
    return *this;
}

inline Rational::~Rational() noexcept { mp_rat_clear(&num_); }

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
    mp_handle_error_(mp_rat_neg(&num_, &num_));
    return *this;
}

inline void Rational::swap(Rational &x) noexcept {
    mp_int_swap(&num_.num, &x.num_.num);
    mp_int_swap(&num_.den, &x.num_.den);
}

inline void Rational::canonicalize() { mp_handle_error_(mp_rat_reduce(&num_)); }

// addition

[[nodiscard]] inline auto operator+(Rational const &a, fixed_int b) -> Rational { return a + Integer{b}; }

[[nodiscard]] inline auto operator+(Rational const &a, Integer const &b) -> Rational {
    Rational c;
    mp_handle_error_(mp_rat_add_int(&a.num_, &b.impl(), &c.num_));
    return c;
}

[[nodiscard]] inline auto operator+(Rational const &a, Rational const &b) -> Rational {
    Rational c;
    mp_handle_error_(mp_rat_add(&a.num_, &b.num_, &c.num_));
    return c;
}

[[nodiscard]] inline auto operator+(Rational &&a, fixed_int b) -> Rational { return std::move(a += b); }

[[nodiscard]] inline auto operator+(Rational &&a, Integer const &b) -> Rational { return std::move(a += b); }

[[nodiscard]] inline auto operator+(Rational &&a, Rational const &b) -> Rational { return std::move(a += b); }

inline auto operator+=(Rational &a, fixed_int b) -> Rational & { return a += Integer{b}; }

inline auto operator+=(Rational &a, Integer const &b) -> Rational & {
    mp_handle_error_(mp_rat_add_int(&a.num_, &b.impl(), &a.num_));
    return a;
}

inline auto operator+=(Rational &a, Rational const &b) -> Rational & {
    mp_handle_error_(mp_rat_add(&a.num_, &b.num_, &a.num_));
    return a;
}

// subtraction

[[nodiscard]] inline auto operator-(Rational a) -> Rational {
    a.neg();
    return a;
}

[[nodiscard]] inline auto operator-(Rational const &a, fixed_int b) -> Rational { return a - Integer{b}; }

[[nodiscard]] inline auto operator-(Rational const &a, Integer const &b) -> Rational {
    Rational c;
    mp_handle_error_(mp_rat_sub_int(&a.num_, &b.impl(), &c.num_));
    return c;
}

[[nodiscard]] inline auto operator-(Rational const &a, Rational const &b) -> Rational {
    Rational c;
    mp_handle_error_(mp_rat_sub(&a.num_, &b.num_, &c.num_));
    return c;
}

[[nodiscard]] inline auto operator-(Rational &&a, fixed_int b) -> Rational { return std::move(a -= b); }

[[nodiscard]] inline auto operator-(Rational &&a, Integer const &b) -> Rational { return std::move(a -= b); }

[[nodiscard]] inline auto operator-(Rational &&a, Rational const &b) -> Rational { return std::move(a -= b); }

inline auto operator-=(Rational &a, fixed_int b) -> Rational & { return a -= Integer{b}; }

inline auto operator-=(Rational &a, Integer const &b) -> Rational & {
    mp_handle_error_(mp_rat_sub_int(&a.num_, &b.impl(), &a.num_));
    return a;
}

inline auto operator-=(Rational &a, Rational const &b) -> Rational & {
    mp_handle_error_(mp_rat_sub(&a.num_, &b.num_, &a.num_));
    return a;
}

// multiplication

[[nodiscard]] inline auto operator*(Rational const &a, fixed_int b) -> Rational { return a * Integer{b}; }

[[nodiscard]] inline auto operator*(Rational const &a, Integer const &b) -> Rational {
    Rational c;
    mp_handle_error_(mp_rat_mul_int(&a.num_, &b.impl(), &c.num_));
    return c;
}

[[nodiscard]] inline auto operator*(Rational const &a, Rational const &b) -> Rational {
    Rational c;
    mp_handle_error_(mp_rat_mul(&a.num_, &b.num_, &c.num_));
    return c;
}

[[nodiscard]] inline auto operator*(Rational &&a, fixed_int b) -> Rational { return std::move(a *= b); }

[[nodiscard]] inline auto operator*(Rational &&a, Integer const &b) -> Rational { return std::move(a *= b); }

[[nodiscard]] inline auto operator*(Rational &&a, Rational const &b) -> Rational { return std::move(a *= b); }

inline auto operator*=(Rational &a, fixed_int b) -> Rational & { return a *= Integer{b}; }

inline auto operator*=(Rational &a, Integer const &b) -> Rational & {
    mp_handle_error_(mp_rat_mul_int(&a.num_, &b.impl(), &a.num_));
    return a;
}

inline auto operator*=(Rational &a, Rational const &b) -> Rational & {
    mp_handle_error_(mp_rat_mul(&a.num_, &b.num_, &a.num_));
    return a;
}

// division

[[nodiscard]] inline auto operator/(Rational const &a, fixed_int b) -> Rational { return a / Integer{b}; }

[[nodiscard]] inline auto operator/(Rational const &a, Integer const &b) -> Rational {
    Rational c;
    mp_handle_error_(mp_rat_div_int(&a.num_, &b.impl(), &c.num_));
    return c;
}

[[nodiscard]] inline auto operator/(Rational const &a, Rational const &b) -> Rational {
    Rational c;
    mp_handle_error_(mp_rat_div(&a.num_, &b.num_, &c.num_));
    return c;
}

[[nodiscard]] inline auto operator/(Rational &&a, fixed_int b) -> Rational { return std::move(a /= b); }

[[nodiscard]] inline auto operator/(Rational &&a, Integer const &b) -> Rational { return std::move(a /= b); }

[[nodiscard]] inline auto operator/(Rational &&a, Rational const &b) -> Rational { return std::move(a /= b); }

inline auto operator/=(Rational &a, fixed_int b) -> Rational & { return a /= Integer{b}; }

inline auto operator/=(Rational &a, Integer const &b) -> Rational & {
    mp_handle_error_(mp_rat_div_int(&a.num_, &b.impl(), &a.num_));
    return a;
}

inline auto operator/=(Rational &a, Rational const &b) -> Rational & {
    mp_handle_error_(mp_rat_div(&a.num_, &b.num_, &a.num_));
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

[[nodiscard]] inline auto operator==(Rational const &a, fixed_int b) -> bool { return compare(a, b) == 0; }

[[nodiscard]] inline auto operator==(Rational const &a, Integer const &b) -> bool { return compare(a, b) == 0; }

[[nodiscard]] inline auto operator==(Rational const &a, Rational const &b) -> bool { return compare(a, b) == 0; }

// not equal to

[[nodiscard]] inline auto operator!=(Rational const &a, fixed_int b) -> bool { return compare(a, b) != 0; }

[[nodiscard]] inline auto operator!=(Rational const &a, Integer const &b) -> bool { return compare(a, b) != 0; }

[[nodiscard]] inline auto operator!=(Rational const &a, Rational const &b) -> bool { return compare(a, b) != 0; }

// printing

inline auto operator<<(std::ostream &out, Rational const &a) -> std::ostream & {
    if (mp_int_compare_value(mp_rat_denom_ref(&a.num_), 1) == 0) {
        auto len = mp_int_string_len(mp_rat_numer_ref(&a.num_), BASE);
        auto buf = std::make_unique<char[]>(len); // NOLINT
        mp_handle_error_(mp_int_to_string(mp_rat_numer_ref(&a.num_), BASE, buf.get(), len));
        out << buf.get();
    } else {
        auto len = mp_rat_string_len(&a.num_, BASE);
        auto buf = std::make_unique<char[]>(len); // NOLINT
        mp_handle_error_(mp_rat_to_string(&a.num_, BASE, buf.get(), len));
        out << buf.get();
    }
    return out;
}

// comparison

[[nodiscard]] inline auto compare(Rational const &a, fixed_int b) -> int { return mp_rat_compare_value(&a.num_, b, 1); }

[[nodiscard]] inline auto compare(Rational const &a, Integer const &b) -> int {
    // Note: this is not good.
    return compare(a, Rational{b, Integer{1}});
}

[[nodiscard]] inline auto compare(Rational const &a, Rational const &b) -> int {
    return mp_rat_compare(&a.num_, &b.num_);
}

#endif
