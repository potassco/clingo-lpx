#pragma once

#include <iostream>
#include <stdexcept>
#include <utility>

#include "number_flint.hh"
#include "number_imath.hh"

namespace ClingoLPX {

class RationalQ;
template <class T>
concept IsRationalQ = std::same_as<std::remove_cvref_t<T>, RationalQ>;

class RationalQ {
  private:
    friend auto operator+(IsRationalQ auto &&a, fixed_int b) -> RationalQ;
    friend auto operator+(IsRationalQ auto &&a, Integer const &b) -> RationalQ;
    friend auto operator+(IsRationalQ auto &&a, Rational const &b) -> RationalQ;
    friend auto operator+(IsRationalQ auto &&a, RationalQ const &b) -> RationalQ;
    friend auto operator+=(RationalQ &a, fixed_int b) -> RationalQ &;
    friend auto operator+=(RationalQ &a, Integer const &b) -> RationalQ &;
    friend auto operator+=(RationalQ &a, Rational const &b) -> RationalQ &;
    friend auto operator+=(RationalQ &a, RationalQ const &b) -> RationalQ &;

    friend auto operator-(RationalQ a) -> RationalQ;

    friend auto operator-(IsRationalQ auto &&a, fixed_int b) -> RationalQ;
    friend auto operator-(IsRationalQ auto &&a, Integer const &b) -> RationalQ;
    friend auto operator-(IsRationalQ auto &&a, Rational const &b) -> RationalQ;
    friend auto operator-(IsRationalQ auto &&a, RationalQ const &b) -> RationalQ;
    friend auto operator-=(RationalQ &a, fixed_int b) -> RationalQ &;
    friend auto operator-=(RationalQ &a, Integer const &b) -> RationalQ &;
    friend auto operator-=(RationalQ &a, Rational const &b) -> RationalQ &;
    friend auto operator-=(RationalQ &a, RationalQ const &b) -> RationalQ &;

    friend auto operator*(IsRationalQ auto &&a, fixed_int b) -> RationalQ;
    friend auto operator*(IsRationalQ auto &&a, Integer const &b) -> RationalQ;
    friend auto operator*(IsRationalQ auto &&a, Rational const &b) -> RationalQ;
    friend auto operator*=(RationalQ &a, fixed_int b) -> RationalQ &;
    friend auto operator*=(RationalQ &a, Integer const &b) -> RationalQ &;
    friend auto operator*=(RationalQ &a, Rational const &b) -> RationalQ &;

    friend auto operator/(IsRationalQ auto &&a, fixed_int b) -> RationalQ;
    friend auto operator/(IsRationalQ auto &&a, Integer const &b) -> RationalQ;
    friend auto operator/(IsRationalQ auto &&a, Rational const &b) -> RationalQ;
    friend auto operator/=(RationalQ &a, fixed_int b) -> RationalQ &;
    friend auto operator/=(RationalQ &a, Integer const &b) -> RationalQ &;
    friend auto operator/=(RationalQ &a, Rational const &b) -> RationalQ &;

    friend auto operator==(RationalQ const &a, fixed_int b) -> bool;
    friend auto operator==(RationalQ const &a, Integer const &b) -> bool;
    friend auto operator==(RationalQ const &a, Rational const &b) -> bool;
    friend auto operator==(RationalQ const &a, RationalQ const &b) -> bool;

    friend auto operator<=>(RationalQ const &a, fixed_int b) -> std::strong_ordering;
    friend auto operator<=>(RationalQ const &a, Integer const &b) -> std::strong_ordering;
    friend auto operator<=>(RationalQ const &a, Rational const &b) -> std::strong_ordering;
    friend auto operator<=>(RationalQ const &a, RationalQ const &b) -> std::strong_ordering;

    friend auto operator<<(std::ostream &out, RationalQ const &q) -> std::ostream &;

    friend auto mul_div(RationalQ const &x, Integer const &a, Integer const &d) -> RationalQ;

    friend auto compare(RationalQ const &a, fixed_int b) -> int;
    friend auto compare(RationalQ const &a, Integer const &b) -> int;
    friend auto compare(RationalQ const &a, Rational const &b) -> int;
    friend auto compare(RationalQ const &a, RationalQ const &b) -> int;

  public:
    // NOLINTNEXTLINE(cppcoreguidelines-explicit-constructor)
    RationalQ(Rational c = Rational{}, Rational k = Rational{}) : c_{std::move(c)}, k_{std::move(k)} {}
    RationalQ(RationalQ const &) = default;
    RationalQ(RationalQ &&) = default;
    auto operator=(RationalQ const &) -> RationalQ & = default;
    auto operator=(RationalQ &&) -> RationalQ & = default;
    ~RationalQ() = default;

    void swap(RationalQ &q) noexcept;
    [[nodiscard]] auto is_rational() const -> bool;
    [[nodiscard]] auto as_rational() const -> Rational const &;
    //! The rational component.
    [[nodiscard]] auto c() const -> Rational const & { return c_; }
    //! The coefficient of the infinitesimal component.
    [[nodiscard]] auto k() const -> Rational const & { return k_; }

  private:
    Rational c_;
    Rational k_;
};

// implementation

inline void RationalQ::swap(RationalQ &q) noexcept {
    c_.swap(q.c_);
    k_.swap(q.k_);
}

inline auto RationalQ::is_rational() const -> bool { return k_ == 0; }

inline auto RationalQ::as_rational() const -> Rational const & {
    if (!is_rational()) {
        throw std::runtime_error("cannot convert number with epsilon component to rational");
    }
    return c_;
}

// comparision

[[nodiscard]] inline auto compare(RationalQ const &a, fixed_int b) -> int {
    auto ret = compare(a.c_, b);
    if (ret != 0) {
        return ret;
    }
    if (a.k_ < 0) {
        return -1;
    }
    if (a.k_ > 0) {
        return 1;
    }
    return 0;
}

[[nodiscard]] inline auto compare(RationalQ const &a, Integer const &b) -> int {
    auto ret = compare(a.c_, b);
    if (ret != 0) {
        return ret;
    }
    if (a.k_ < 0) {
        return -1;
    }
    if (a.k_ > 0) {
        return 1;
    }
    return 0;
}

[[nodiscard]] inline auto compare(RationalQ const &a, Rational const &b) -> int {
    auto ret = compare(a.c_, b);
    if (ret != 0) {
        return ret;
    }
    if (a.k_ < 0) {
        return -1;
    }
    if (a.k_ > 0) {
        return 1;
    }
    return 0;
}

[[nodiscard]] inline auto compare(RationalQ const &a, RationalQ const &b) -> int {
    auto ret = compare(a.c_, b.c_);
    if (ret != 0) {
        return ret;
    }
    return compare(a.k_, b.k_);
}

// addition

[[nodiscard]] inline auto operator+(IsRationalQ auto &&a, fixed_int b) -> RationalQ {
    if constexpr (std::is_rvalue_reference_v<decltype(a)>) {
        auto ret = std::forward<decltype(a)>(a);
        ret += b;
        return ret;
    } else {
        return RationalQ{a.c_ + b, a.k_};
    }
}

[[nodiscard]] inline auto operator+(IsRationalQ auto &&a, Integer const &b) -> RationalQ {
    if constexpr (std::is_rvalue_reference_v<decltype(a)>) {
        auto ret = std::forward<decltype(a)>(a);
        ret += b;
        return ret;
    } else {
        return RationalQ{a.c_ + b, a.k_};
    }
}

[[nodiscard]] inline auto operator+(IsRationalQ auto &&a, Rational const &b) -> RationalQ {
    if constexpr (std::is_rvalue_reference_v<decltype(a)>) {
        auto ret = std::forward<decltype(a)>(a);
        ret += b;
        return ret;
    } else {
        return RationalQ{a.c_ + b, a.k_};
    }
}

[[nodiscard]] inline auto operator+(IsRationalQ auto &&a, RationalQ const &b) -> RationalQ {
    if constexpr (std::is_rvalue_reference_v<decltype(a)>) {
        auto ret = std::forward<decltype(a)>(a);
        ret += b;
        return ret;
    } else {
        return RationalQ{a.c_ + b.c_, a.k_ + b.k_};
    }
}

inline auto operator+=(RationalQ &a, fixed_int b) -> RationalQ & {
    a.c_ += b;
    return a;
}

inline auto operator+=(RationalQ &a, Integer const &b) -> RationalQ & {
    a.c_ += b;
    return a;
}

inline auto operator+=(RationalQ &a, Rational const &b) -> RationalQ & {
    a.c_ += b;
    return a;
}

inline auto operator+=(RationalQ &a, RationalQ const &b) -> RationalQ & {
    a.c_ += b.c_;
    a.k_ += b.k_;
    return a;
}

// subtraction

[[nodiscard]] inline auto operator-(RationalQ a) -> RationalQ {
    a.c_.neg();
    a.k_.neg();
    return a;
}

[[nodiscard]] inline auto operator-(IsRationalQ auto &&a, fixed_int b) -> RationalQ {
    if constexpr (std::is_rvalue_reference_v<decltype(a)>) {
        auto ret = std::forward<decltype(a)>(a);
        ret -= b;
        return ret;
    } else {
        return RationalQ{a.c_ - b, a.k_};
    }
}

[[nodiscard]] inline auto operator-(IsRationalQ auto &&a, Integer const &b) -> RationalQ {
    if constexpr (std::is_rvalue_reference_v<decltype(a)>) {
        auto ret = std::forward<decltype(a)>(a);
        ret -= b;
        return ret;
    } else {
        return RationalQ{a.c_ - b, a.k_};
    }
}

[[nodiscard]] inline auto operator-(IsRationalQ auto &&a, Rational const &b) -> RationalQ {
    if constexpr (std::is_rvalue_reference_v<decltype(a)>) {
        auto ret = std::forward<decltype(a)>(a);
        ret -= b;
        return ret;
    } else {
        return RationalQ{a.c_ - b, a.k_};
    }
}

[[nodiscard]] inline auto operator-(IsRationalQ auto &&a, RationalQ const &b) -> RationalQ {
    if constexpr (std::is_rvalue_reference_v<decltype(a)>) {
        auto ret = std::forward<decltype(a)>(a);
        ret -= b;
        return ret;
    } else {
        return RationalQ{a.c_ - b.c_, a.k_ - b.k_};
    }
}

inline auto operator-=(RationalQ &a, fixed_int b) -> RationalQ & {
    a.c_ -= b;
    return a;
}

inline auto operator-=(RationalQ &a, Integer const &b) -> RationalQ & {
    a.c_ -= b;
    return a;
}

inline auto operator-=(RationalQ &a, Rational const &b) -> RationalQ & {
    a.c_ -= b;
    return a;
}

inline auto operator-=(RationalQ &a, RationalQ const &b) -> RationalQ & {
    a.c_ -= b.c_;
    a.k_ -= b.k_;
    return a;
}

// multiplication

[[nodiscard]] inline auto operator*(IsRationalQ auto &&a, fixed_int b) -> RationalQ {
    if constexpr (std::is_rvalue_reference_v<decltype(a)>) {
        auto ret = std::forward<decltype(a)>(a);
        ret *= b;
        return ret;
    } else {
        return RationalQ{a.c_ * b, a.k_ * b};
    }
}

[[nodiscard]] inline auto operator*(IsRationalQ auto &&a, Integer const &b) -> RationalQ {
    if constexpr (std::is_rvalue_reference_v<decltype(a)>) {
        auto ret = std::forward<decltype(a)>(a);
        ret *= b;
        return ret;
    } else {
        return RationalQ{a.c_ * b, a.k_ * b};
    }
}

[[nodiscard]] inline auto operator*(IsRationalQ auto &&a, Rational const &b) -> RationalQ {
    if constexpr (std::is_rvalue_reference_v<decltype(a)>) {
        auto ret = std::forward<decltype(a)>(a);
        ret *= b;
        return ret;
    } else {
        return RationalQ{a.c_ * b, a.k_ * b};
    }
}

inline auto operator*=(RationalQ &a, fixed_int b) -> RationalQ & {
    a.c_ *= b;
    a.k_ *= b;
    return a;
}

inline auto operator*=(RationalQ &a, Integer const &b) -> RationalQ & {
    a.c_ *= b;
    a.k_ *= b;
    return a;
}

inline auto operator*=(RationalQ &a, Rational const &b) -> RationalQ & {
    a.c_ *= b;
    a.k_ *= b;
    return a;
}

// division

[[nodiscard]] inline auto operator/(IsRationalQ auto &&a, fixed_int b) -> RationalQ {
    if constexpr (std::is_rvalue_reference_v<decltype(a)>) {
        auto ret = std::forward<decltype(a)>(a);
        ret /= b;
        return ret;
    } else {
        return RationalQ{a.c_ / b, a.k_ / b};
    }
}

[[nodiscard]] inline auto operator/(IsRationalQ auto &&a, Integer const &b) -> RationalQ {
    if constexpr (std::is_rvalue_reference_v<decltype(a)>) {
        auto ret = std::forward<decltype(a)>(a);
        ret /= b;
        return ret;
    } else {
        return RationalQ{a.c_ / b, a.k_ / b};
    }
}

[[nodiscard]] inline auto operator/(IsRationalQ auto &&a, Rational const &b) -> RationalQ {
    if constexpr (std::is_rvalue_reference_v<decltype(a)>) {
        auto ret = std::forward<decltype(a)>(a);
        ret /= b;
        return ret;
    } else {
        return RationalQ{a.c_ / b, a.k_ / b};
    }
}

inline auto operator/=(RationalQ &a, fixed_int b) -> RationalQ & {
    a.c_ /= b;
    a.k_ /= b;
    return a;
}

inline auto operator/=(RationalQ &a, Integer const &b) -> RationalQ & {
    a.c_ /= b;
    a.k_ /= b;
    return a;
}

inline auto operator/=(RationalQ &a, Rational const &b) -> RationalQ & {
    a.c_ /= b;
    a.k_ /= b;
    return a;
}

//! Compute x * a / d.
[[nodiscard]] inline auto mul_div(RationalQ const &x, Integer const &a, Integer const &d) -> RationalQ {
    return RationalQ{mul_div(x.c_, a, d), mul_div(x.k_, a, d)};
}

// comparison

[[nodiscard]] inline auto operator==(RationalQ const &a, fixed_int b) -> bool { return a.k_ == 0 && a.c_ == b; }
[[nodiscard]] inline auto operator==(RationalQ const &a, Integer const &b) -> bool { return a.k_ == 0 && a.c_ == b; }
[[nodiscard]] inline auto operator==(RationalQ const &a, Rational const &b) -> bool { return a.k_ == 0 && a.c_ == b; }
[[nodiscard]] inline auto operator==(RationalQ const &a, RationalQ const &b) -> bool {
    return a.c_ == b.c_ && a.k_ == b.k_;
}

[[nodiscard]] inline auto operator<=>(RationalQ const &a, fixed_int b) -> std::strong_ordering {
    return compare(a, b) <=> 0;
}
[[nodiscard]] inline auto operator<=>(RationalQ const &a, Integer const &b) -> std::strong_ordering {
    return compare(a, b) <=> 0;
}
[[nodiscard]] inline auto operator<=>(RationalQ const &a, Rational const &b) -> std::strong_ordering {
    return compare(a, b) <=> 0;
}
[[nodiscard]] inline auto operator<=>(RationalQ const &a, RationalQ const &b) -> std::strong_ordering {
    return compare(a, b) <=> 0;
}

inline auto operator<<(std::ostream &out, RationalQ const &q) -> std::ostream & {
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

//! Convert an integer into a rational number.
[[nodiscard]] inline auto as_rational(Integer a) -> Rational { return Rational{std::move(a), Integer{1}}; }

//! Compute the absolute value of an integer.
[[nodiscard]] inline auto magnitude(Integer a) -> Integer {
    if (a < 0) {
        a.neg();
    }
    return a;
}

//! Check if numerator and denominator of a rational number fit into machine integers.
[[nodiscard]] inline auto is_small(Rational const &a) -> bool {
    return a.num().as_int().has_value() && a.den().as_int().has_value();
}

//! Get a rational number (for uniform treatment with RationalQ).
//!
//! The function must not be called with temporaries.
// NOLINTNEXTLINE(bugprone-return-const-ref-from-parameter)
[[nodiscard]] inline auto rational_part(Rational const &a) -> Rational const & { return a; }

//! Check if a value has no infinitesimal component (always true for Rational).
[[nodiscard]] inline auto is_rational(Rational const &a) -> bool {
    static_cast<void>(a);
    return true;
}

//! Round a rational number towards negative infinity.
[[nodiscard]] inline auto floor(Rational const &a) -> Integer { return floor_div(a.num(), a.den()); }

//! Round a rational number towards positive infinity.
[[nodiscard]] inline auto ceil(Rational const &a) -> Integer { return -floor_div(-a.num(), a.den()); }

//! Check whether a rational number is an integer.
[[nodiscard]] inline auto is_integer(Rational const &a) -> bool { return a.den() == 1; }

//! Check whether a rational number with an infinitesimal component is an integer.
[[nodiscard]] inline auto is_integer(RationalQ const &a) -> bool {
    return a.is_rational() && is_integer(a.as_rational());
}

//! Get the rational component of a value (which must not have an infinitesimal component).
[[nodiscard]] inline auto rational_part(RationalQ const &a) -> Rational const & { return a.as_rational(); }

//! Check if a value has no infinitesimal component.
[[nodiscard]] inline auto is_rational(RationalQ const &a) -> bool { return a.is_rational(); }

//! Round a rational number with an infinitesimal component towards negative infinity.
//!
//! The infinitesimal is assumed to be arbitrarily small but positive.
[[nodiscard]] inline auto floor(RationalQ const &a) -> Integer {
    if (is_integer(a.c())) {
        return a.k() < 0 ? a.c().num() - 1 : a.c().num();
    }
    return floor(a.c());
}

//! Compute the least common multiple of two positive integers.
[[nodiscard]] inline auto lcm(Integer const &a, Integer const &b) -> Integer {
    auto [g, ag, bg] = gcd_div(a, b);
    return std::move(ag) * b;
}

} // namespace ClingoLPX
