#pragma once

#ifdef CLINGOLPX_USE_FLINT

#include <flint/fmpq.h>
#include <flint/fmpz.h>

#include <iostream>
#include <limits>
#include <memory>
#include <new>
#include <numeric>
#include <optional>
#include <stdexcept>
#include <string>

namespace ClingoLPX {

constexpr int BASE = 10;

using fixed_int = slong;

namespace Detail {

//! Check if an fmpz is stored as a machine integer.
inline auto is_small(fmpz const &a) -> bool { return !COEFF_IS_MPZ(a); }

//! Check if all given fmpzs are stored as machine integers.
inline auto are_small(std::same_as<fmpz> auto const &...a) -> bool { return (is_small(a) && ...); }

//! Compute c = a * b if a and b are small and the product does not overflow.
inline auto mul_small(fmpz &c, fmpz const &a, fmpz const &b) -> bool {
    slong r = 0;
    if (are_small(a, b) && !__builtin_mul_overflow(a, b, &r)) {
        fmpz_set_si(&c, r);
        return true;
    }
    return false;
}

//! Compute c += a * b if all are small and there is no overflow.
inline auto add_mul_small(fmpz &c, fmpz const &a, fmpz const &b) -> bool {
    slong m = 0;
    slong r = 0;
    if (are_small(a, b, c) && !__builtin_mul_overflow(a, b, &m) && !__builtin_add_overflow(c, m, &r)) {
        fmpz_set_si(&c, r);
        return true;
    }
    return false;
}

} // namespace Detail

class Integer;
template <class T>
concept IsInteger = std::same_as<std::remove_cvref_t<T>, Integer>;

class Integer {
    friend auto operator+(IsInteger auto &&a, fixed_int b) -> Integer;
    friend auto operator+(IsInteger auto &&a, Integer const &b) -> Integer;
    friend auto operator+=(Integer &a, fixed_int b) -> Integer &;
    friend auto operator+=(Integer &a, Integer const &b) -> Integer &;

    friend auto operator-(Integer a) -> Integer;

    friend auto operator-(IsInteger auto &&a, fixed_int b) -> Integer;
    friend auto operator-(IsInteger auto &&a, Integer const &b) -> Integer;
    friend auto operator-=(Integer &a, fixed_int b) -> Integer &;
    friend auto operator-=(Integer &a, Integer const &b) -> Integer &;

    friend auto operator*(IsInteger auto &&a, fixed_int b) -> Integer;
    friend auto operator*(IsInteger auto &&a, Integer const &b) -> Integer;
    friend auto operator*=(Integer &a, fixed_int b) -> Integer &;
    friend auto operator*=(Integer &a, Integer const &b) -> Integer &;

    friend auto operator==(Integer const &a, fixed_int b) -> bool;
    friend auto operator==(Integer const &a, Integer const &b) -> bool;

    friend auto operator<=>(Integer const &a, Integer const &b) -> std::strong_ordering;
    friend auto operator<=>(Integer const &a, fixed_int b) -> std::strong_ordering;

    friend auto operator<<(std::ostream &out, Integer const &a) -> std::ostream &;

    friend auto compare(Integer const &a, fixed_int b) -> int;
    friend auto compare(Integer const &a, Integer const &b) -> int;

    friend auto gcd(Integer const &a, Integer const &b) -> Integer;
    friend auto gcd_div(Integer const &a, Integer const &b) -> std::tuple<Integer, Integer, Integer>;

  public:
    Integer() noexcept;
    // NOLINTNEXTLINE(cppcoreguidelines-explicit-constructor)
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
    [[nodiscard]] auto as_int() const -> std::optional<int>;

  private:
    mutable fmpz num_;
};

class Rational;
template <class T>
concept IsRational = std::same_as<std::remove_cvref_t<T>, Rational>;

class Rational {
  private:
    friend auto operator+(IsRational auto &&a, fixed_int b) -> Rational;
    friend auto operator+(IsRational auto &&a, Integer const &b) -> Rational;
    friend auto operator+(IsRational auto &&a, Rational const &b) -> Rational;
    friend auto operator+=(Rational &a, fixed_int b) -> Rational &;
    friend auto operator+=(Rational &a, Integer const &b) -> Rational &;
    friend auto operator+=(Rational &a, Rational const &b) -> Rational &;

    friend auto operator-(Rational a) -> Rational;

    friend auto operator-(IsRational auto &&a, fixed_int b) -> Rational;
    friend auto operator-(IsRational auto &&a, Integer const &b) -> Rational;
    friend auto operator-(IsRational auto &&a, Rational const &b) -> Rational;
    friend auto operator-=(Rational &a, fixed_int b) -> Rational &;
    friend auto operator-=(Rational &a, Integer const &b) -> Rational &;
    friend auto operator-=(Rational &a, Rational const &b) -> Rational &;

    friend auto operator*(IsRational auto &&a, fixed_int b) -> Rational;
    friend auto operator*(IsRational auto &&a, Integer const &b) -> Rational;
    friend auto operator*(IsRational auto &&a, Rational const &b) -> Rational;
    friend auto operator*=(Rational &a, fixed_int b) -> Rational &;
    friend auto operator*=(Rational &a, Integer const &b) -> Rational &;
    friend auto operator*=(Rational &a, Rational const &b) -> Rational &;

    friend auto operator/(IsRational auto &&a, fixed_int b) -> Rational;
    friend auto operator/(IsRational auto &&a, Integer const &b) -> Rational;
    friend auto operator/(IsRational auto &&a, Rational const &b) -> Rational;
    friend auto operator/=(Rational &a, fixed_int b) -> Rational &;
    friend auto operator/=(Rational &a, Integer const &b) -> Rational &;
    friend auto operator/=(Rational &a, Rational const &b) -> Rational &;

    friend auto operator==(Rational const &a, fixed_int b) -> bool;
    friend auto operator==(Rational const &a, Integer const &b) -> bool;
    friend auto operator==(Rational const &a, Rational const &b) -> bool;

    friend auto operator<=>(Rational const &a, fixed_int b) -> std::strong_ordering;
    friend auto operator<=>(Rational const &a, Integer const &b) -> std::strong_ordering;
    friend auto operator<=>(Rational const &a, Rational const &b) -> std::strong_ordering;

    friend auto operator<<(std::ostream &out, Rational const &a) -> std::ostream &;

    friend auto compare(Rational const &a, fixed_int b) -> int;
    friend auto compare(Rational const &a, Integer const &b) -> int;
    friend auto compare(Rational const &a, Rational const &b) -> int;
    friend auto mul_div(Rational const &x, Integer const &a, Integer const &d) -> Rational;

  public:
    Rational() noexcept;
    // NOLINTNEXTLINE(cppcoreguidelines-explicit-constructor)
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
    if (Detail::are_small(num_, a.num_)) {
        // the result is small, too
        num_ /= a.num_;
    } else {
        fmpz_divexact(&num_, &num_, &a.num_);
    }
    return *this;
}

inline auto Integer::add_mul(Integer const &a, Integer const &b) & -> Integer & {
    if (!Detail::add_mul_small(num_, a.num_, b.num_)) {
        fmpz_addmul(&num_, &a.num_, &b.num_);
    }
    return *this;
}

inline auto Integer::add_mul(Integer const &a, Integer const &b) && -> Integer {
    return std::move(this->add_mul(a, b));
}

inline auto Integer::neg() -> Integer & {
    fmpz_neg(&num_, &num_);
    return *this;
}

inline auto Integer::as_int() const -> std::optional<int> {
    if (fmpz_fits_si(&num_) != 0) {
        if (auto res = fmpz_get_si(&num_);
            std::numeric_limits<int>::min() <= res && res <= std::numeric_limits<int>::max()) {
            return static_cast<int>(res);
        }
    }
    return std::nullopt;
}

inline auto Integer::impl() const -> fmpz & { return num_; }

// addition

[[nodiscard]] inline auto operator+(IsInteger auto &&a, fixed_int b) -> Integer {
    if constexpr (std::is_rvalue_reference_v<decltype(a)>) {
        auto ret = std::forward<decltype(a)>(a);
        ret += b;
        return ret;
    } else {
        Integer c;
        fmpz_add_si(&c.num_, &a.num_, b);
        return c;
    }
}

[[nodiscard]] inline auto operator+(IsInteger auto &&a, Integer const &b) -> Integer {
    if constexpr (std::is_rvalue_reference_v<decltype(a)>) {
        auto ret = std::forward<decltype(a)>(a);
        ret += b;
        return ret;
    } else {
        Integer c;
        fmpz_add(&c.num_, &a.num_, &b.num_);
        return c;
    }
}

inline auto operator+=(Integer &a, fixed_int b) -> Integer & {
    fmpz_add_si(&a.num_, &a.num_, b);
    return a;
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

[[nodiscard]] inline auto operator-(IsInteger auto &&a, fixed_int b) -> Integer {
    if constexpr (std::is_rvalue_reference_v<decltype(a)>) {
        auto ret = std::forward<decltype(a)>(a);
        ret -= b;
        return ret;
    } else {
        Integer c;
        fmpz_sub_si(&c.num_, &a.num_, b);
        return c;
    }
}

[[nodiscard]] inline auto operator-(IsInteger auto &&a, Integer const &b) -> Integer {
    if constexpr (std::is_rvalue_reference_v<decltype(a)>) {
        auto ret = std::forward<decltype(a)>(a);
        ret -= b;
        return ret;
    } else {
        Integer c;
        fmpz_sub(&c.num_, &a.num_, &b.num_);
        return c;
    }
}

inline auto operator-=(Integer &a, fixed_int b) -> Integer & {
    fmpz_sub_si(&a.num_, &a.num_, b);
    return a;
}

inline auto operator-=(Integer &a, Integer const &b) -> Integer & {
    fmpz_sub(&a.num_, &a.num_, &b.num_);
    return a;
}

// multiplication

[[nodiscard]] inline auto operator*(IsInteger auto &&a, fixed_int b) -> Integer {
    if constexpr (std::is_rvalue_reference_v<decltype(a)>) {
        auto ret = std::forward<decltype(a)>(a);
        ret *= b;
        return ret;
    } else {
        Integer c;
        fmpz_mul_si(&c.num_, &a.num_, b);
        return c;
    }
}

[[nodiscard]] inline auto operator*(IsInteger auto &&a, Integer const &b) -> Integer {
    if constexpr (std::is_rvalue_reference_v<decltype(a)>) {
        auto ret = std::forward<decltype(a)>(a);
        ret *= b;
        return ret;
    } else {
        Integer c;
        if (!Detail::mul_small(c.num_, a.num_, b.num_)) {
            fmpz_mul(&c.num_, &a.num_, &b.num_);
        }
        return c;
    }
}

inline auto operator*=(Integer &a, fixed_int b) -> Integer & {
    fmpz_mul_si(&a.num_, &a.num_, b);
    return a;
}

inline auto operator*=(Integer &a, Integer const &b) -> Integer & {
    if (!Detail::mul_small(a.num_, a.num_, b.num_)) {
        fmpz_mul(&a.num_, &a.num_, &b.num_);
    }
    return a;
}

// comparison

[[nodiscard]] inline auto operator==(Integer const &a, fixed_int b) -> bool {
    return Detail::is_small(a.num_) ? a.num_ == b : fmpz_equal_si(&a.num_, b) != 0;
}
[[nodiscard]] inline auto operator==(Integer const &a, Integer const &b) -> bool {
    return fmpz_equal(&a.num_, &b.num_) != 0;
}
[[nodiscard]] inline auto operator<=>(Integer const &a, Integer const &b) -> std::strong_ordering {
    return compare(a, b) <=> 0;
}
[[nodiscard]] inline auto operator<=>(Integer const &a, fixed_int b) -> std::strong_ordering {
    return compare(a, b) <=> 0;
}

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

[[nodiscard]] inline auto compare(Integer const &a, Integer const &b) -> int { return fmpz_cmp(&a.num_, &b.num_); }

// gcd

[[nodiscard]] inline auto gcd(Integer const &a, Integer const &b) -> Integer {
    Integer g;
    if (Detail::are_small(a.num_, b.num_)) {
        // the result is non-negative and small
        g.num_ = std::gcd(a.num_, b.num_);
    } else {
        fmpz_gcd(&g.num_, &a.num_, &b.num_);
    }
    return g;
}

[[nodiscard]] inline auto gcd_div(Integer const &a, Integer const &b) -> std::tuple<Integer, Integer, Integer> {
    std::tuple<Integer, Integer, Integer> ret;
    auto &[g, ag, bg] = ret;
    if (Detail::are_small(a.num_, b.num_)) {
        g.num_ = std::gcd(a.num_, b.num_);
        if (g.num_ != 0) {
            ag.num_ = a.num_ / g.num_;
            bg.num_ = b.num_ / g.num_;
        }
    } else {
        fmpz_gcd(&g.num_, &a.num_, &b.num_);
        fmpz_divexact(&ag.num_, &a.num_, &g.num_);
        fmpz_divexact(&bg.num_, &b.num_, &g.num_);
    }
    return ret;
}

//! Compute the quotient a/b rounded towards negative infinity.
[[nodiscard]] inline auto floor_div(Integer const &a, Integer const &b) -> Integer {
    Integer q;
    fmpz_fdiv_q(&q.impl(), &a.impl(), &b.impl());
    return q;
}

// Rational

inline Rational::Rational() noexcept { // NOLINT
    fmpq_init(&num_);
}

inline Rational::Rational(fixed_int val) : Rational() { fmpq_set_si(&num_, val, 1); }

inline Rational::Rational(char const *val, int radix) : Rational() {
    if (fmpq_set_str(&num_, val, radix) != 0) {
        throw std::runtime_error("could not parse number");
    }
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

[[nodiscard]] inline auto operator+(IsRational auto &&a, fixed_int b) -> Rational {
    if constexpr (std::is_rvalue_reference_v<decltype(a)>) {
        auto ret = std::forward<decltype(a)>(a);
        ret += b;
        return ret;
    } else {
        Rational c;
        fmpq_add_si(&c.num_, &a.num_, b);
        return c;
    }
}

[[nodiscard]] inline auto operator+(IsRational auto &&a, Integer const &b) -> Rational {
    if constexpr (std::is_rvalue_reference_v<decltype(a)>) {
        auto ret = std::forward<decltype(a)>(a);
        ret += b;
        return ret;
    } else {
        Rational c;
        fmpq_add_fmpz(&c.num_, &a.num_, &b.impl());
        return c;
    }
}

[[nodiscard]] inline auto operator+(IsRational auto &&a, Rational const &b) -> Rational {
    if constexpr (std::is_rvalue_reference_v<decltype(a)>) {
        auto ret = std::forward<decltype(a)>(a);
        ret += b;
        return ret;
    } else {
        Rational c;
        fmpq_add(&c.num_, &a.num_, &b.num_);
        return c;
    }
}

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

[[nodiscard]] inline auto operator-(IsRational auto &&a, fixed_int b) -> Rational {
    if constexpr (std::is_rvalue_reference_v<decltype(a)>) {
        auto ret = std::forward<decltype(a)>(a);
        ret -= b;
        return ret;
    } else {
        Rational c;
        fmpq_sub_si(&c.num_, &a.num_, b);
        return c;
    }
}

[[nodiscard]] inline auto operator-(IsRational auto &&a, Integer const &b) -> Rational {
    if constexpr (std::is_rvalue_reference_v<decltype(a)>) {
        auto ret = std::forward<decltype(a)>(a);
        ret -= b;
        return ret;
    } else {
        Rational c;
        fmpq_sub_fmpz(&c.num_, &a.num_, &b.impl());
        return c;
    }
}

[[nodiscard]] inline auto operator-(IsRational auto &&a, Rational const &b) -> Rational {
    if constexpr (std::is_rvalue_reference_v<decltype(a)>) {
        auto ret = std::forward<decltype(a)>(a);
        ret -= b;
        return ret;
    } else {
        Rational c;
        fmpq_sub(&c.num_, &a.num_, &b.num_);
        return c;
    }
}

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

[[nodiscard]] inline auto operator*(IsRational auto &&a, fixed_int b) -> Rational {
    if constexpr (std::is_rvalue_reference_v<decltype(a)>) {
        auto ret = std::forward<decltype(a)>(a);
        ret *= b;
        return ret;
    } else {
        Rational c;
        fmpq_mul_si(&c.num_, &a.num_, b);
        return c;
    }
}

[[nodiscard]] inline auto operator*(IsRational auto &&a, Integer const &b) -> Rational {
    if constexpr (std::is_rvalue_reference_v<decltype(a)>) {
        auto ret = std::forward<decltype(a)>(a);
        ret *= b;
        return ret;
    } else {
        Rational c;
        fmpq_mul_fmpz(&c.num_, &a.num_, &b.impl());
        return c;
    }
}

[[nodiscard]] inline auto operator*(IsRational auto &&a, Rational const &b) -> Rational {
    if constexpr (std::is_rvalue_reference_v<decltype(a)>) {
        auto ret = std::forward<decltype(a)>(a);
        ret *= b;
        return ret;
    } else {
        Rational c;
        fmpq_mul(&c.num_, &a.num_, &b.num_);
        return c;
    }
}

inline auto operator*=(Rational &a, fixed_int b) -> Rational & {
    fmpq_mul_si(&a.num_, &a.num_, b);
    return a;
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

[[nodiscard]] inline auto operator/(IsRational auto &&a, fixed_int b) -> Rational {
    if constexpr (std::is_rvalue_reference_v<decltype(a)>) {
        auto ret = std::forward<decltype(a)>(a);
        ret /= b;
        return ret;
    } else {
        return a / Integer{b};
    }
}

[[nodiscard]] inline auto operator/(IsRational auto &&a, Integer const &b) -> Rational {
    if constexpr (std::is_rvalue_reference_v<decltype(a)>) {
        auto ret = std::forward<decltype(a)>(a);
        ret /= b;
        return ret;
    } else {
        Rational c;
        fmpq_div_fmpz(&c.num_, &a.num_, &b.impl());
        return c;
    }
}

[[nodiscard]] inline auto operator/(IsRational auto &&a, Rational const &b) -> Rational {
    if constexpr (std::is_rvalue_reference_v<decltype(a)>) {
        auto ret = std::forward<decltype(a)>(a);
        ret /= b;
        return ret;
    } else {
        Rational c;
        fmpq_div(&c.num_, &a.num_, &b.num_);
        return c;
    }
}

inline auto operator/=(Rational &a, fixed_int b) -> Rational & { return a /= Integer{b}; }

inline auto operator/=(Rational &a, Integer const &b) -> Rational & {
    fmpq_div_fmpz(&a.num_, &a.num_, &b.impl());
    return a;
}

inline auto operator/=(Rational &a, Rational const &b) -> Rational & {
    fmpq_div(&a.num_, &a.num_, &b.num_);
    return a;
}

//! Compute x * a / d.
[[nodiscard]] inline auto mul_div(Rational const &x, Integer const &a, Integer const &d) -> Rational {
    auto const &p = x.num().impl();
    auto const &q = x.den().impl();
    if (Detail::are_small(p, q, a.impl(), d.impl())) {
        // We cancel common factors before multiplying. The result is
        // canonical because x is canonical.
        slong u = p;
        slong v = q;
        slong s = a.impl();
        slong t = d.impl();
        auto cancel = [](slong &m, slong &n) -> void {
            if (m == 1 || m == -1 || n == 1 || n == -1) {
                return;
            }
            if (auto g = std::gcd(m, n); g > 1) {
                m /= g;
                n /= g;
            }
        };
        cancel(u, t);
        cancel(s, v);
        cancel(s, t);
        auto num = static_cast<__int128>(u) * s;
        auto den = static_cast<__int128>(v) * t;
        if (den < 0) {
            num = -num;
            den = -den;
        }
        constexpr auto max = static_cast<__int128>(std::numeric_limits<slong>::max());
        if (-max <= num && num <= max && den <= max) {
            Rational r;
            fmpz_set_si(&r.num().impl(), static_cast<slong>(num));
            fmpz_set_si(&r.den().impl(), static_cast<slong>(den));
            return r;
        }
    }
    Rational r;
    fmpq_mul_fmpz(&r.num_, &x.num_, &a.impl());
    fmpq_div_fmpz(&r.num_, &r.num_, &d.impl());
    return r;
}

// comparison

[[nodiscard]] inline auto operator==(Rational const &a, fixed_int b) -> bool { return fmpq_equal_si(&a.num_, b) != 0; }

[[nodiscard]] inline auto operator==(Rational const &a, Integer const &b) -> bool {
    return fmpq_equal_fmpz(&a.num_, &b.impl()) != 0;
}

[[nodiscard]] inline auto operator==(Rational const &a, Rational const &b) -> bool {
    return fmpq_equal(&a.num_, &b.num_) != 0;
}
[[nodiscard]] inline auto operator<=>(Rational const &a, fixed_int b) -> std::strong_ordering {
    return compare(a, b) <=> 0;
}

[[nodiscard]] inline auto operator<=>(Rational const &a, Integer const &b) -> std::strong_ordering {
    return compare(a, b) <=> 0;
}

[[nodiscard]] inline auto operator<=>(Rational const &a, Rational const &b) -> std::strong_ordering {
    return compare(a, b) <=> 0;
}

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

[[nodiscard]] inline auto compare(Rational const &a, fixed_int b) -> int { return fmpq_cmp_si(&a.num_, b); }

[[nodiscard]] inline auto compare(Rational const &a, Integer const &b) -> int {
    return fmpq_cmp_fmpz(&a.num_, &b.impl());
}

[[nodiscard]] inline auto compare(Rational const &a, Rational const &b) -> int {
    auto const *an = fmpq_numref(&a.num_);
    auto const *ad = fmpq_denref(&a.num_);
    auto const *bn = fmpq_numref(&b.num_);
    auto const *bd = fmpq_denref(&b.num_);
    if (Detail::are_small(*an, *ad, *bn, *bd)) {
        // denominators are positive
        auto x = static_cast<__int128>(*an) * *bd;
        auto y = static_cast<__int128>(*bn) * *ad;
        if (x < y) {
            return -1;
        }
        return x > y ? 1 : 0;
    }
    return fmpq_cmp(&a.num_, &b.num_);
}

} // namespace ClingoLPX

#endif
