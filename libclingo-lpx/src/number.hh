#pragma once

#include <utility>
#include <iostream>

#include "number_flint.hh"
#include "number_imath.hh"


class RationalQ {
private:
    friend RationalQ operator+(RationalQ const &a, fixed_int b);
    friend RationalQ operator+(RationalQ const &a, Integer const &b);
    friend RationalQ operator+(RationalQ const &a, Rational  const &b);
    friend RationalQ operator+(RationalQ const &a, RationalQ const &b);
    friend RationalQ operator+(RationalQ &&a, fixed_int b);
    friend RationalQ operator+(RationalQ &&a, Integer const &b);
    friend RationalQ operator+(RationalQ &&a, Rational  const &b);
    friend RationalQ operator+(RationalQ &&a, RationalQ const &b);
    friend RationalQ &operator+=(RationalQ &a, fixed_int b);
    friend RationalQ &operator+=(RationalQ &a, Integer const &b);
    friend RationalQ &operator+=(RationalQ &a, Rational const &b);
    friend RationalQ &operator+=(RationalQ &a, RationalQ const &b);

    friend RationalQ operator-(RationalQ a);

    friend RationalQ operator-(RationalQ const &a, fixed_int b);
    friend RationalQ operator-(RationalQ const &a, Integer const &b);
    friend RationalQ operator-(RationalQ const &a, Rational  const &b);
    friend RationalQ operator-(RationalQ const &a, RationalQ const &b);
    friend RationalQ operator-(RationalQ &&a, fixed_int b);
    friend RationalQ operator-(RationalQ &&a, Integer const &b);
    friend RationalQ operator-(RationalQ &&a, Rational const &b);
    friend RationalQ operator-(RationalQ &&a, RationalQ const &b);
    friend RationalQ &operator-=(RationalQ &a, fixed_int b);
    friend RationalQ &operator-=(RationalQ &a, Integer const &b);
    friend RationalQ &operator-=(RationalQ &a, Rational const &b);
    friend RationalQ &operator-=(RationalQ &a, RationalQ const &b);

    friend RationalQ operator*(RationalQ const &a, fixed_int b);
    friend RationalQ operator*(RationalQ const &a, Integer const &b);
    friend RationalQ operator*(RationalQ const &a, Rational  const &b);
    friend RationalQ operator*(RationalQ &&a, fixed_int b);
    friend RationalQ operator*(RationalQ &&a, Integer const &b);
    friend RationalQ operator*(RationalQ &&a, Rational const &b);
    friend RationalQ &operator*=(RationalQ &a, fixed_int b);
    friend RationalQ &operator*=(RationalQ &a, Integer const &b);
    friend RationalQ &operator*=(RationalQ &a, Rational const &b);

    friend RationalQ operator/(RationalQ const &a, fixed_int b);
    friend RationalQ operator/(RationalQ const &a, Integer const &b);
    friend RationalQ operator/(RationalQ const &a, Rational  const &b);
    friend RationalQ operator/(RationalQ &&a, fixed_int b);
    friend RationalQ operator/(RationalQ &&a, Integer const &b);
    friend RationalQ operator/(RationalQ &&a, Rational const &b);
    friend RationalQ &operator/=(RationalQ &a, fixed_int b);
    friend RationalQ &operator/=(RationalQ &a, Integer const &b);
    friend RationalQ &operator/=(RationalQ &a, Rational const &b);

    friend bool operator<(RationalQ const &a, fixed_int b);
    friend bool operator<(RationalQ const &a, Integer const &b);
    friend bool operator<(RationalQ const &a, Rational const &b);
    friend bool operator<(RationalQ const &a, RationalQ const &b);

    friend bool operator<=(RationalQ const &a, fixed_int b);
    friend bool operator<=(RationalQ const &a, Integer const &b);
    friend bool operator<=(RationalQ const &a, Rational const &b);
    friend bool operator<=(RationalQ const &a, RationalQ const &b);

    friend bool operator>(RationalQ const &a, fixed_int b);
    friend bool operator>(RationalQ const &a, Integer const &b);
    friend bool operator>(RationalQ const &a, Rational const &b);
    friend bool operator>(RationalQ const &a, RationalQ const &b);

    friend bool operator>=(RationalQ const &a, fixed_int b);
    friend bool operator>=(RationalQ const &a, Integer const &b);
    friend bool operator>=(RationalQ const &a, Rational const &b);
    friend bool operator>=(RationalQ const &a, RationalQ const &b);

    friend bool operator==(RationalQ const &a, fixed_int b);
    friend bool operator==(RationalQ const &a, Integer const &b);
    friend bool operator==(RationalQ const &a, Rational const &b);
    friend bool operator==(RationalQ const &a, RationalQ const &b);

    friend bool operator!=(RationalQ const &a, fixed_int b);
    friend bool operator!=(RationalQ const &a, Integer const &b);
    friend bool operator!=(RationalQ const &a, RationalQ const &b);
    friend bool operator!=(RationalQ const &a, Rational const &b);

    friend std::ostream &operator<<(std::ostream &out, RationalQ const &q);

    friend int compare(RationalQ const &a, fixed_int b);
    friend int compare(RationalQ const &a, Integer const &b);
    friend int compare(RationalQ const &a, Rational const &b);
    friend int compare(RationalQ const &a, RationalQ const &b);

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

private:
    Rational c_;
    Rational k_;
};

// comparision

[[nodiscard]] inline int compare(RationalQ const &a, fixed_int b) {
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

[[nodiscard]] inline int compare(RationalQ const &a, Integer const &b) {
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

[[nodiscard]] inline int compare(RationalQ const &a, Rational const &b) {
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

[[nodiscard]] inline int compare(RationalQ const &a, RationalQ const &b) {
    auto ret = compare(a.c_, b.c_);
    if (ret != 0) {
        return ret;
    }
    return compare(a.k_, b.k_);
}

// addition

[[nodiscard]] inline RationalQ operator+(RationalQ const &a, fixed_int b) {
    return RationalQ{a.c_ + b, a.k_};
}

[[nodiscard]] inline RationalQ operator+(RationalQ const &a, Integer const &b) {
    return RationalQ{a.c_ + b, a.k_};
}

[[nodiscard]] inline RationalQ operator+(RationalQ const &a, Rational const &b) {
    return RationalQ{a.c_ + b, a.k_};
}

[[nodiscard]] inline RationalQ operator+(RationalQ const &a, RationalQ const &b) {
    return RationalQ{a.c_ + b.c_, a.k_ + b.k_};
}

[[nodiscard]] inline RationalQ operator+(RationalQ &&a, fixed_int b) {
    return std::move(a += b);
}

[[nodiscard]] inline RationalQ operator+(RationalQ &&a, Integer const &b) {
    return std::move(a += b);
}

[[nodiscard]] inline RationalQ operator+(RationalQ &&a, Rational const &b) {
    return std::move(a += b);
}

[[nodiscard]] inline RationalQ operator+(RationalQ &&a, RationalQ const &b) {
    return std::move(a += b);
}

inline RationalQ &operator+=(RationalQ &a, fixed_int b) {
    a.c_ += b;
    return a;
}

inline RationalQ &operator+=(RationalQ &a, Integer const &b) {
    a.c_ += b;
    return a;
}

inline RationalQ &operator+=(RationalQ &a, Rational const &b) {
    a.c_ += b;
    return a;
}

inline RationalQ &operator+=(RationalQ &a, RationalQ const &b) {
    a.c_ += b.c_;
    a.k_ += b.k_;
    return a;
}

// subtraction

[[nodiscard]] inline RationalQ operator-(RationalQ a) {
    a.c_.neg();
    a.k_.neg();
    return a;
}

[[nodiscard]] inline RationalQ operator-(RationalQ const &a, fixed_int b) {
    return RationalQ{a.c_ - b, a.k_};
}

[[nodiscard]] inline RationalQ operator-(RationalQ const &a, Integer const &b) {
    return RationalQ{a.c_ - b, a.k_};
}

[[nodiscard]] inline RationalQ operator-(RationalQ const &a, Rational const &b) {
    return RationalQ{a.c_ - b, a.k_};
}

[[nodiscard]] inline RationalQ operator-(RationalQ const &a, RationalQ const &b) {
    return RationalQ{a.c_ - b.c_, a.k_ - b.k_};
}

[[nodiscard]] inline RationalQ operator-(RationalQ &&a, fixed_int b) {
    return std::move(a -= b);
}

[[nodiscard]] inline RationalQ operator-(RationalQ &&a, Integer const &b) {
    return std::move(a -= b);
}

[[nodiscard]] inline RationalQ operator-(RationalQ &&a, Rational const &b) {
    return std::move(a -= b);
}

[[nodiscard]] inline RationalQ operator-(RationalQ &&a, RationalQ const &b) {
    return std::move(a -= b);
}

inline RationalQ &operator-=(RationalQ &a, fixed_int b) {
    a.c_ -= b;
    return a;
}

inline RationalQ &operator-=(RationalQ &a, Integer const &b) {
    a.c_ -= b;
    return a;
}

inline RationalQ &operator-=(RationalQ &a, Rational const &b) {
    a.c_ -= b;
    return a;
}

inline RationalQ &operator-=(RationalQ &a, RationalQ const &b) {
    a.c_ -= b.c_;
    a.k_ -= b.k_;
    return a;
}

// multiplication

[[nodiscard]] inline RationalQ operator*(RationalQ const &a, fixed_int b) {
    return RationalQ{a.c_ * b, a.k_ * b};
}

[[nodiscard]] inline RationalQ operator*(RationalQ const &a, Integer const &b) {
    return RationalQ{a.c_ * b, a.k_ * b};
}

[[nodiscard]] inline RationalQ operator*(RationalQ const &a, Rational const &b) {
    return RationalQ{a.c_ * b, a.k_ * b};
}

[[nodiscard]] inline RationalQ operator*(RationalQ &&a, fixed_int b) {
    return std::move(a *= b);
}

[[nodiscard]] inline RationalQ operator*(RationalQ &&a, Integer const &b) {
    return std::move(a *= b);
}

[[nodiscard]] inline RationalQ operator*(RationalQ &&a, Rational const &b) {
    return std::move(a *= b);
}

inline RationalQ &operator*=(RationalQ &a, fixed_int b) {
    a.c_ *= b;
    a.k_ *= b;
    return a;
}

inline RationalQ &operator*=(RationalQ &a, Integer const &b) {
    a.c_ *= b;
    a.k_ *= b;
    return a;
}

inline RationalQ &operator*=(RationalQ &a, Rational const &b) {
    a.c_ *= b;
    a.k_ *= b;
    return a;
}

// division

[[nodiscard]] inline RationalQ operator/(RationalQ const &a, fixed_int b) {
    return RationalQ{a.c_ / b, a.k_ / b};
}

[[nodiscard]] inline RationalQ operator/(RationalQ const &a, Integer const &b) {
    return RationalQ{a.c_ / b, a.k_ / b};
}

[[nodiscard]] inline RationalQ operator/(RationalQ const &a, Rational const &b) {
    return RationalQ{a.c_ / b, a.k_ / b};
}

[[nodiscard]] inline RationalQ operator/(RationalQ &&a, fixed_int b) {
    return std::move(a /= b);
}

[[nodiscard]] inline RationalQ operator/(RationalQ &&a, Integer const &b) {
    return std::move(a /= b);
}

[[nodiscard]] inline RationalQ operator/(RationalQ &&a, Rational const &b) {
    return std::move(a /= b);
}

inline RationalQ &operator/=(RationalQ &a, fixed_int b) {
    a.c_ /= b;
    a.k_ /= b;
    return a;
}

inline RationalQ &operator/=(RationalQ &a, Integer const &b) {
    a.c_ /= b;
    a.k_ /= b;
    return a;
}

inline RationalQ &operator/=(RationalQ &a, Rational const &b) {
    a.c_ /= b;
    a.k_ /= b;
    return a;
}

// comparison

[[nodiscard]] inline bool operator<(RationalQ const &a, fixed_int b) {
    return compare(a, b) < 0;
}
[[nodiscard]] inline bool operator<(RationalQ const &a, Integer const &b) {
    return compare(a, b) < 0;
}
[[nodiscard]] inline bool operator<(RationalQ const &a, Rational const &b) {
    return compare(a, b) < 0;
}
[[nodiscard]] inline bool operator<(RationalQ const &a, RationalQ const &b) {
    return compare(a, b) < 0;
}

[[nodiscard]] inline bool operator<=(RationalQ const &a, fixed_int b) {
    return compare(a, b) <= 0;
}
[[nodiscard]] inline bool operator<=(RationalQ const &a, Integer const &b) {
    return compare(a, b) <= 0;
}
[[nodiscard]] inline bool operator<=(RationalQ const &a, Rational const &b) {
    return compare(a, b) <= 0;
}
[[nodiscard]] inline bool operator<=(RationalQ const &a, RationalQ const &b) {
    return compare(a, b) <= 0;
}

[[nodiscard]] inline bool operator>(RationalQ const &a, fixed_int b) {
    return compare(a, b) > 0;
}
[[nodiscard]] inline bool operator>(RationalQ const &a, Integer const &b) {
    return compare(a, b) > 0;
}
[[nodiscard]] inline bool operator>(RationalQ const &a, Rational const &b) {
    return compare(a, b) > 0;
}
[[nodiscard]] inline bool operator>(RationalQ const &a, RationalQ const &b) {
    return compare(a, b) > 0;
}

[[nodiscard]] inline bool operator>=(RationalQ const &a, fixed_int b) {
    return compare(a, b) >= 0;
}
[[nodiscard]] inline bool operator>=(RationalQ const &a, Integer const &b) {
    return compare(a, b) >= 0;
}
[[nodiscard]] inline bool operator>=(RationalQ const &a, Rational const &b) {
    return compare(a, b) >= 0;
}
[[nodiscard]] inline bool operator>=(RationalQ const &a, RationalQ const &b) {
    return compare(a, b) >= 0;
}

[[nodiscard]] inline bool operator==(RationalQ const &a, fixed_int b) {
    return a.k_ == 0 && a.c_ == b;
}
[[nodiscard]] inline bool operator==(RationalQ const &a, Integer const &b) {
    return a.k_ == 0 && a.c_ == b;
}
[[nodiscard]] inline bool operator==(RationalQ const &a, Rational const &b) {
    return a.k_ == 0 && a.c_ == b;
}
[[nodiscard]] inline bool operator==(RationalQ const &a, RationalQ const &b) {
    return a.c_ == b.c_ && a.k_ == b.k_;
}

[[nodiscard]] inline bool operator!=(RationalQ const &a, fixed_int b) {
    return !(a == b);
}
[[nodiscard]] inline bool operator!=(RationalQ const &a, Integer const &b) {
    return !(a == b);
}
[[nodiscard]] inline bool operator!=(RationalQ const &a, Rational const &b) {
    return !(a == b);
}
[[nodiscard]] inline bool operator!=(RationalQ const &a, RationalQ const &b) {
    return !(a == b);
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
