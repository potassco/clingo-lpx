#pragma once

#include <functional>
#include <iostream>
#include <stdexcept>
#include <utility>

#include "number_flint.hh"
#include "number_imath.hh"

class RationalQ {
  private:
    friend auto operator+(RationalQ const &a, fixed_int b) -> RationalQ;
    friend auto operator+(RationalQ const &a, Integer const &b) -> RationalQ;
    friend auto operator+(RationalQ const &a, Rational const &b) -> RationalQ;
    friend auto operator+(RationalQ const &a, RationalQ const &b) -> RationalQ;
    friend auto operator+(RationalQ &&a, fixed_int b) -> RationalQ;
    friend auto operator+(RationalQ &&a, Integer const &b) -> RationalQ;
    friend auto operator+(RationalQ &&a, Rational const &b) -> RationalQ;
    friend auto operator+(RationalQ &&a, RationalQ const &b) -> RationalQ;
    friend auto operator+=(RationalQ &a, fixed_int b) -> RationalQ &;
    friend auto operator+=(RationalQ &a, Integer const &b) -> RationalQ &;
    friend auto operator+=(RationalQ &a, Rational const &b) -> RationalQ &;
    friend auto operator+=(RationalQ &a, RationalQ const &b) -> RationalQ &;

    friend auto operator-(RationalQ a) -> RationalQ;

    friend auto operator-(RationalQ const &a, fixed_int b) -> RationalQ;
    friend auto operator-(RationalQ const &a, Integer const &b) -> RationalQ;
    friend auto operator-(RationalQ const &a, Rational const &b) -> RationalQ;
    friend auto operator-(RationalQ const &a, RationalQ const &b) -> RationalQ;
    friend auto operator-(RationalQ &&a, fixed_int b) -> RationalQ;
    friend auto operator-(RationalQ &&a, Integer const &b) -> RationalQ;
    friend auto operator-(RationalQ &&a, Rational const &b) -> RationalQ;
    friend auto operator-(RationalQ &&a, RationalQ const &b) -> RationalQ;
    friend auto operator-=(RationalQ &a, fixed_int b) -> RationalQ &;
    friend auto operator-=(RationalQ &a, Integer const &b) -> RationalQ &;
    friend auto operator-=(RationalQ &a, Rational const &b) -> RationalQ &;
    friend auto operator-=(RationalQ &a, RationalQ const &b) -> RationalQ &;

    friend auto operator*(RationalQ const &a, fixed_int b) -> RationalQ;
    friend auto operator*(RationalQ const &a, Integer const &b) -> RationalQ;
    friend auto operator*(RationalQ const &a, Rational const &b) -> RationalQ;
    friend auto operator*(RationalQ &&a, fixed_int b) -> RationalQ;
    friend auto operator*(RationalQ &&a, Integer const &b) -> RationalQ;
    friend auto operator*(RationalQ &&a, Rational const &b) -> RationalQ;
    friend auto operator*=(RationalQ &a, fixed_int b) -> RationalQ &;
    friend auto operator*=(RationalQ &a, Integer const &b) -> RationalQ &;
    friend auto operator*=(RationalQ &a, Rational const &b) -> RationalQ &;

    friend auto operator/(RationalQ const &a, fixed_int b) -> RationalQ;
    friend auto operator/(RationalQ const &a, Integer const &b) -> RationalQ;
    friend auto operator/(RationalQ const &a, Rational const &b) -> RationalQ;
    friend auto operator/(RationalQ &&a, fixed_int b) -> RationalQ;
    friend auto operator/(RationalQ &&a, Integer const &b) -> RationalQ;
    friend auto operator/(RationalQ &&a, Rational const &b) -> RationalQ;
    friend auto operator/=(RationalQ &a, fixed_int b) -> RationalQ &;
    friend auto operator/=(RationalQ &a, Integer const &b) -> RationalQ &;
    friend auto operator/=(RationalQ &a, Rational const &b) -> RationalQ &;

    friend auto operator<(RationalQ const &a, fixed_int b) -> bool;
    friend auto operator<(RationalQ const &a, Integer const &b) -> bool;
    friend auto operator<(RationalQ const &a, Rational const &b) -> bool;
    friend auto operator<(RationalQ const &a, RationalQ const &b) -> bool;

    friend auto operator<=(RationalQ const &a, fixed_int b) -> bool;
    friend auto operator<=(RationalQ const &a, Integer const &b) -> bool;
    friend auto operator<=(RationalQ const &a, Rational const &b) -> bool;
    friend auto operator<=(RationalQ const &a, RationalQ const &b) -> bool;

    friend auto operator>(RationalQ const &a, fixed_int b) -> bool;
    friend auto operator>(RationalQ const &a, Integer const &b) -> bool;
    friend auto operator>(RationalQ const &a, Rational const &b) -> bool;
    friend auto operator>(RationalQ const &a, RationalQ const &b) -> bool;

    friend auto operator>=(RationalQ const &a, fixed_int b) -> bool;
    friend auto operator>=(RationalQ const &a, Integer const &b) -> bool;
    friend auto operator>=(RationalQ const &a, Rational const &b) -> bool;
    friend auto operator>=(RationalQ const &a, RationalQ const &b) -> bool;

    friend auto operator==(RationalQ const &a, fixed_int b) -> bool;
    friend auto operator==(RationalQ const &a, Integer const &b) -> bool;
    friend auto operator==(RationalQ const &a, Rational const &b) -> bool;
    friend auto operator==(RationalQ const &a, RationalQ const &b) -> bool;

    friend auto operator!=(RationalQ const &a, fixed_int b) -> bool;
    friend auto operator!=(RationalQ const &a, Integer const &b) -> bool;
    friend auto operator!=(RationalQ const &a, RationalQ const &b) -> bool;
    friend auto operator!=(RationalQ const &a, Rational const &b) -> bool;

    friend auto operator<<(std::ostream &out, RationalQ const &q) -> std::ostream &;

    friend auto compare(RationalQ const &a, fixed_int b) -> int;
    friend auto compare(RationalQ const &a, Integer const &b) -> int;
    friend auto compare(RationalQ const &a, Rational const &b) -> int;
    friend auto compare(RationalQ const &a, RationalQ const &b) -> int;

  public:
    explicit RationalQ(Rational c = Rational{}, Rational k = Rational{}) : c_{std::move(c)}, k_{std::move(k)} {}
    RationalQ(RationalQ const &) = default;
    RationalQ(RationalQ &&) = default;
    auto operator=(RationalQ const &) -> RationalQ & = default;
    auto operator=(RationalQ &&) -> RationalQ & = default;
    ~RationalQ() = default;

    void swap(RationalQ &q);
    [[nodiscard]] auto is_rational() const -> bool;
    [[nodiscard]] auto as_rational() const -> Rational const &;

  private:
    Rational c_;
    Rational k_;
};

// implementation

inline void RationalQ::swap(RationalQ &q) {
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

[[nodiscard]] inline auto operator+(RationalQ const &a, fixed_int b) -> RationalQ { return RationalQ{a.c_ + b, a.k_}; }

[[nodiscard]] inline auto operator+(RationalQ const &a, Integer const &b) -> RationalQ {
    return RationalQ{a.c_ + b, a.k_};
}

[[nodiscard]] inline auto operator+(RationalQ const &a, Rational const &b) -> RationalQ {
    return RationalQ{a.c_ + b, a.k_};
}

[[nodiscard]] inline auto operator+(RationalQ const &a, RationalQ const &b) -> RationalQ {
    return RationalQ{a.c_ + b.c_, a.k_ + b.k_};
}

[[nodiscard]] inline auto operator+(RationalQ &&a, fixed_int b) -> RationalQ { return std::move(a += b); }

[[nodiscard]] inline auto operator+(RationalQ &&a, Integer const &b) -> RationalQ { return std::move(a += b); }

[[nodiscard]] inline auto operator+(RationalQ &&a, Rational const &b) -> RationalQ { return std::move(a += b); }

[[nodiscard]] inline auto operator+(RationalQ &&a, RationalQ const &b) -> RationalQ { return std::move(a += b); }

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

[[nodiscard]] inline auto operator-(RationalQ const &a, fixed_int b) -> RationalQ { return RationalQ{a.c_ - b, a.k_}; }

[[nodiscard]] inline auto operator-(RationalQ const &a, Integer const &b) -> RationalQ {
    return RationalQ{a.c_ - b, a.k_};
}

[[nodiscard]] inline auto operator-(RationalQ const &a, Rational const &b) -> RationalQ {
    return RationalQ{a.c_ - b, a.k_};
}

[[nodiscard]] inline auto operator-(RationalQ const &a, RationalQ const &b) -> RationalQ {
    return RationalQ{a.c_ - b.c_, a.k_ - b.k_};
}

[[nodiscard]] inline auto operator-(RationalQ &&a, fixed_int b) -> RationalQ { return std::move(a -= b); }

[[nodiscard]] inline auto operator-(RationalQ &&a, Integer const &b) -> RationalQ { return std::move(a -= b); }

[[nodiscard]] inline auto operator-(RationalQ &&a, Rational const &b) -> RationalQ { return std::move(a -= b); }

[[nodiscard]] inline auto operator-(RationalQ &&a, RationalQ const &b) -> RationalQ { return std::move(a -= b); }

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

[[nodiscard]] inline auto operator*(RationalQ const &a, fixed_int b) -> RationalQ {
    return RationalQ{a.c_ * b, a.k_ * b};
}

[[nodiscard]] inline auto operator*(RationalQ const &a, Integer const &b) -> RationalQ {
    return RationalQ{a.c_ * b, a.k_ * b};
}

[[nodiscard]] inline auto operator*(RationalQ const &a, Rational const &b) -> RationalQ {
    return RationalQ{a.c_ * b, a.k_ * b};
}

[[nodiscard]] inline auto operator*(RationalQ &&a, fixed_int b) -> RationalQ { return std::move(a *= b); }

[[nodiscard]] inline auto operator*(RationalQ &&a, Integer const &b) -> RationalQ { return std::move(a *= b); }

[[nodiscard]] inline auto operator*(RationalQ &&a, Rational const &b) -> RationalQ { return std::move(a *= b); }

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

[[nodiscard]] inline auto operator/(RationalQ const &a, fixed_int b) -> RationalQ {
    return RationalQ{a.c_ / b, a.k_ / b};
}

[[nodiscard]] inline auto operator/(RationalQ const &a, Integer const &b) -> RationalQ {
    return RationalQ{a.c_ / b, a.k_ / b};
}

[[nodiscard]] inline auto operator/(RationalQ const &a, Rational const &b) -> RationalQ {
    return RationalQ{a.c_ / b, a.k_ / b};
}

[[nodiscard]] inline auto operator/(RationalQ &&a, fixed_int b) -> RationalQ { return std::move(a /= b); }

[[nodiscard]] inline auto operator/(RationalQ &&a, Integer const &b) -> RationalQ { return std::move(a /= b); }

[[nodiscard]] inline auto operator/(RationalQ &&a, Rational const &b) -> RationalQ { return std::move(a /= b); }

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

// comparison

[[nodiscard]] inline auto operator<(RationalQ const &a, fixed_int b) -> bool { return compare(a, b) < 0; }
[[nodiscard]] inline auto operator<(RationalQ const &a, Integer const &b) -> bool { return compare(a, b) < 0; }
[[nodiscard]] inline auto operator<(RationalQ const &a, Rational const &b) -> bool { return compare(a, b) < 0; }
[[nodiscard]] inline auto operator<(RationalQ const &a, RationalQ const &b) -> bool { return compare(a, b) < 0; }

[[nodiscard]] inline auto operator<=(RationalQ const &a, fixed_int b) -> bool { return compare(a, b) <= 0; }
[[nodiscard]] inline auto operator<=(RationalQ const &a, Integer const &b) -> bool { return compare(a, b) <= 0; }
[[nodiscard]] inline auto operator<=(RationalQ const &a, Rational const &b) -> bool { return compare(a, b) <= 0; }
[[nodiscard]] inline auto operator<=(RationalQ const &a, RationalQ const &b) -> bool { return compare(a, b) <= 0; }

[[nodiscard]] inline auto operator>(RationalQ const &a, fixed_int b) -> bool { return compare(a, b) > 0; }
[[nodiscard]] inline auto operator>(RationalQ const &a, Integer const &b) -> bool { return compare(a, b) > 0; }
[[nodiscard]] inline auto operator>(RationalQ const &a, Rational const &b) -> bool { return compare(a, b) > 0; }
[[nodiscard]] inline auto operator>(RationalQ const &a, RationalQ const &b) -> bool { return compare(a, b) > 0; }

[[nodiscard]] inline auto operator>=(RationalQ const &a, fixed_int b) -> bool { return compare(a, b) >= 0; }
[[nodiscard]] inline auto operator>=(RationalQ const &a, Integer const &b) -> bool { return compare(a, b) >= 0; }
[[nodiscard]] inline auto operator>=(RationalQ const &a, Rational const &b) -> bool { return compare(a, b) >= 0; }
[[nodiscard]] inline auto operator>=(RationalQ const &a, RationalQ const &b) -> bool { return compare(a, b) >= 0; }

[[nodiscard]] inline auto operator==(RationalQ const &a, fixed_int b) -> bool { return a.k_ == 0 && a.c_ == b; }
[[nodiscard]] inline auto operator==(RationalQ const &a, Integer const &b) -> bool { return a.k_ == 0 && a.c_ == b; }
[[nodiscard]] inline auto operator==(RationalQ const &a, Rational const &b) -> bool { return a.k_ == 0 && a.c_ == b; }
[[nodiscard]] inline auto operator==(RationalQ const &a, RationalQ const &b) -> bool {
    return a.c_ == b.c_ && a.k_ == b.k_;
}

[[nodiscard]] inline auto operator!=(RationalQ const &a, fixed_int b) -> bool { return !(a == b); }
[[nodiscard]] inline auto operator!=(RationalQ const &a, Integer const &b) -> bool { return !(a == b); }
[[nodiscard]] inline auto operator!=(RationalQ const &a, Rational const &b) -> bool { return !(a == b); }
[[nodiscard]] inline auto operator!=(RationalQ const &a, RationalQ const &b) -> bool { return !(a == b); }

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
