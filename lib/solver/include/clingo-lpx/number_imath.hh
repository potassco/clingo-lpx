#pragma once

#ifdef CLINGOLPX_USE_IMATH

#include <imath.h>
#include <imrat.h>

#include <array>
#include <climits>
#include <compare>
#include <concepts>
#include <cstdint>
#include <iostream>
#include <limits>
#include <memory>
#include <new>
#include <numeric>
#include <optional>
#include <ranges>
#include <span>
#include <stdexcept>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>

#if defined(_MSC_VER) && !defined(__clang__) && defined(_M_X64)
#include <intrin.h>
#endif

namespace ClingoLPX {

constexpr int BASE = 10;

using fixed_int = std::int64_t;

namespace Detail {

//! The largest magnitude of an integer stored without imath.
//!
//! The smallest 64-bit integer is excluded so that negating a small integer or
//! taking its absolute value cannot overflow.
constexpr std::int64_t small_max = std::numeric_limits<std::int64_t>::max();

inline void mp_handle_error(mp_result res) {
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

//! Add two small integers returning false if the result is not small.
inline auto small_add(std::int64_t a, std::int64_t b, std::int64_t &c) -> bool {
    if ((b > 0 && a > small_max - b) || (b < 0 && a < -small_max - b)) {
        return false;
    }
    c = a + b;
    return true;
}

//! Subtract two small integers returning false if the result is not small.
inline auto small_sub(std::int64_t a, std::int64_t b, std::int64_t &c) -> bool {
    if ((b < 0 && a > small_max + b) || (b > 0 && a < -small_max + b)) {
        return false;
    }
    c = a - b;
    return true;
}

//! Multiply two small integers returning false if the result is not small.
inline auto small_mul(std::int64_t a, std::int64_t b, std::int64_t &c) -> bool {
#if defined(__GNUC__) || defined(__clang__)
    return !__builtin_mul_overflow(a, b, &c) && c != std::numeric_limits<std::int64_t>::min();
#elif defined(_MSC_VER) && defined(_M_X64)
    std::int64_t hi = 0;
    c = _mul128(a, b, &hi);
    return hi == (c >> 63) && c != std::numeric_limits<std::int64_t>::min();
#else
    if (a == 0 || b == 0) {
        c = 0;
        return true;
    }
    if ((a < 0 ? -a : a) > small_max / (b < 0 ? -b : b)) {
        return false;
    }
    c = a * b;
    return true;
#endif
}

//! Compare two small integers.
inline auto small_cmp(std::int64_t a, std::int64_t b) -> int {
    return static_cast<int>(a > b) - static_cast<int>(a < b);
}

//! Store a 64-bit integer in an initialized imath integer.
inline void mp_set(mp_int z, std::int64_t v) {
    if constexpr (sizeof(mp_small) >= sizeof(std::int64_t)) {
        mp_handle_error(mp_int_set_value(z, static_cast<mp_small>(v)));
    } else {
        // NOTE: imath only offers conversions from long, which might have 32 bits
        auto u = v < 0 ? 0 - static_cast<std::uint64_t>(v) : static_cast<std::uint64_t>(v);
        auto buf = std::array<unsigned char, sizeof(u)>{};
        for (auto &byte : buf | std::views::reverse) {
            byte = static_cast<unsigned char>(u & UCHAR_MAX);
            u >>= CHAR_BIT;
        }
        mp_handle_error(mp_int_read_unsigned(z, buf.data(), static_cast<int>(buf.size())));
        if (v < 0) {
            mp_handle_error(mp_int_neg(z, z));
        }
    }
}

//! Get the value of an imath integer if it is small.
inline auto mp_get(mp_int z, std::int64_t &v) -> bool {
    static_assert(MP_DIGIT_BIT < std::numeric_limits<std::uint64_t>::digits);
    if (mp_int_count_bits(z) > std::numeric_limits<std::int64_t>::digits) {
        return false;
    }
    auto u = std::uint64_t{0};
    for (auto digit : std::span{MP_DIGITS(z), MP_USED(z)} | std::views::reverse) {
        u = (u << MP_DIGIT_BIT) | digit;
    }
    v = MP_SIGN(z) == MP_NEG ? -static_cast<std::int64_t>(u) : static_cast<std::int64_t>(u);
    return true;
}

} // namespace Detail

class Integer;
template <class T>
concept IsInteger = std::same_as<std::remove_cvref_t<T>, Integer>;

//! An arbitrary precision integer.
//!
//! Integers fitting into 64 bits are stored directly and only larger ones use
//! imath. This avoids allocations and the generic algorithms of imath for the
//! common case of small numbers.
class Integer {
  public:
    Integer() noexcept = default;
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
    [[nodiscard]] auto as_int() const -> std::optional<int>;
    [[nodiscard]] auto sign() const -> int;

    //! Set this integer to the result of the given operation.
    //!
    //! The arguments may alias this integer.
    //! @{
    auto set_add(Integer const &a, Integer const &b) -> Integer &;
    auto set_add(Integer const &a, fixed_int b) -> Integer &;
    auto set_sub(Integer const &a, Integer const &b) -> Integer &;
    auto set_sub(Integer const &a, fixed_int b) -> Integer &;
    auto set_mul(Integer const &a, Integer const &b) -> Integer &;
    auto set_mul(Integer const &a, fixed_int b) -> Integer &;
    auto set_gcd(Integer const &a, Integer const &b) -> Integer &;
    auto set_divexact(Integer const &a, Integer const &b) -> Integer &;
    //! @}

    [[nodiscard]] auto cmp(Integer const &b) const -> int;
    [[nodiscard]] auto cmp(fixed_int b) const -> int;
    [[nodiscard]] auto equal(Integer const &b) const -> bool;
    [[nodiscard]] auto equal(fixed_int b) const -> bool;
    void print(std::ostream &out) const;

  private:
    friend class Rational;

    //! An imath integer to pass an integer to imath.
    class Arg;
    //! An imath integer receiving the result of an imath operation.
    class Res;

    void set_small_(std::int64_t v) noexcept;
    void set_mp_(mp_int z);
    void adopt_(Res &res) noexcept;
    template <class F> auto mp_op_(Integer const &a, Integer const &b, F f) -> Integer &;

    //! The value if mp_ is null.
    std::int64_t small_ = 0;
    //! The value if it does not fit into 64 bits.
    mp_int mp_ = nullptr;
};

class Rational;
template <class T>
concept IsRational = std::same_as<std::remove_cvref_t<T>, Rational>;

//! An arbitrary precision rational number.
//!
//! The number is kept in canonical form, i.e., the numerator and the
//! denominator are coprime and the denominator is positive.
class Rational {
  public:
    Rational() noexcept;
    // NOLINTNEXTLINE(cppcoreguidelines-explicit-constructor)
    Rational(fixed_int val);
    Rational(Integer num, Integer den);
    Rational(char const *val, int radix);
    Rational(std::string const &val, int radix);

    [[nodiscard]] auto num() -> Integer &;
    [[nodiscard]] auto num() const -> Integer const &;
    [[nodiscard]] auto den() -> Integer &;
    [[nodiscard]] auto den() const -> Integer const &;
    auto neg() -> Rational &;
    void swap(Rational &x) noexcept;
    void canonicalize();

    //! Set this number to the result of the given operation.
    //!
    //! The arguments may alias this number.
    //! @{
    auto set_add(Rational const &a, Rational const &b) -> Rational &;
    auto set_add(Rational const &a, Integer const &b) -> Rational &;
    auto set_sub(Rational const &a, Rational const &b) -> Rational &;
    auto set_sub(Rational const &a, Integer const &b) -> Rational &;
    auto set_mul(Rational const &a, Rational const &b) -> Rational &;
    auto set_mul(Rational const &a, Integer const &b) -> Rational &;
    auto set_div(Rational const &a, Rational const &b) -> Rational &;
    auto set_div(Rational const &a, Integer const &b) -> Rational &;
    //! @}

    [[nodiscard]] auto cmp(Rational const &b) const -> int;
    [[nodiscard]] auto cmp(Integer const &b) const -> int;
    void print(std::ostream &out) const;

  private:
    template <bool Sub> auto add_sub_(Rational const &a, Rational const &b) -> Rational &;
    template <bool Sub> auto add_sub_(Rational const &a, Integer const &b) -> Rational &;
    void assign_(Integer &&num, Integer &&den) noexcept;
    void set_zero_() noexcept;

    Integer num_;
    Integer den_;
};

// Integer

class Integer::Arg {
  public:
    explicit Arg(Integer const &a) {
        if (a.mp_ != nullptr) {
            ptr_ = a.mp_;
        } else {
            mp_int_init(&tmp_);
            ptr_ = &tmp_;
            try {
                Detail::mp_set(ptr_, a.small_);
            } catch (...) {
                mp_int_clear(&tmp_);
                throw;
            }
        }
    }
    Arg(Arg const &) = delete;
    Arg(Arg &&) = delete;
    auto operator=(Arg const &) -> Arg & = delete;
    auto operator=(Arg &&) -> Arg & = delete;
    ~Arg() noexcept {
        if (ptr_ == &tmp_) {
            mp_int_clear(&tmp_);
        }
    }
    [[nodiscard]] auto get() const noexcept -> mp_int { return ptr_; }

  private:
    mpz_t tmp_{};
    mp_int ptr_ = nullptr;
};

class Integer::Res {
  public:
    Res() : ptr_{mp_int_alloc()} {
        if (ptr_ == nullptr) {
            throw std::bad_alloc();
        }
    }
    Res(Res const &) = delete;
    Res(Res &&) = delete;
    auto operator=(Res const &) -> Res & = delete;
    auto operator=(Res &&) -> Res & = delete;
    ~Res() noexcept {
        if (ptr_ != nullptr) {
            mp_int_free(ptr_);
        }
    }
    [[nodiscard]] auto get() const noexcept -> mp_int { return ptr_; }
    [[nodiscard]] auto release() noexcept -> mp_int { return std::exchange(ptr_, nullptr); }

  private:
    mp_int ptr_;
};

inline Integer::Integer(fixed_int val) {
    if (val != std::numeric_limits<std::int64_t>::min()) {
        small_ = val;
    } else {
        auto res = Res{};
        Detail::mp_set(res.get(), val);
        mp_ = res.release();
    }
}

inline Integer::Integer(char const *val, int radix) {
    auto res = Res{};
    Detail::mp_handle_error(mp_int_read_string(res.get(), radix, val));
    adopt_(res);
}

inline Integer::Integer(std::string const &val, int radix) : Integer(val.c_str(), radix) {}

inline Integer::Integer(Integer const &a) : small_{a.small_} {
    if (a.mp_ != nullptr) {
        set_mp_(a.mp_);
    }
}

inline Integer::Integer(Integer &&a) noexcept
    : small_{std::exchange(a.small_, 0)}, mp_{std::exchange(a.mp_, nullptr)} {}

inline auto Integer::operator=(Integer const &a) -> Integer & {
    if (this != &a) {
        if (a.mp_ != nullptr) {
            set_mp_(a.mp_);
        } else {
            set_small_(a.small_);
        }
    }
    return *this;
}

inline auto Integer::operator=(Integer &&a) noexcept -> Integer & {
    swap(a);
    return *this;
}

inline Integer::~Integer() noexcept {
    if (mp_ != nullptr) {
        mp_int_free(mp_);
    }
}

inline void Integer::swap(Integer &x) noexcept {
    std::swap(small_, x.small_);
    std::swap(mp_, x.mp_);
}

inline void Integer::set_small_(std::int64_t v) noexcept {
    if (mp_ != nullptr) {
        mp_int_free(std::exchange(mp_, nullptr));
    }
    small_ = v;
}

inline void Integer::set_mp_(mp_int z) {
    auto res = Res{};
    Detail::mp_handle_error(mp_int_copy(z, res.get()));
    adopt_(res);
}

inline void Integer::adopt_(Res &res) noexcept {
    auto v = std::int64_t{0};
    if (Detail::mp_get(res.get(), v)) {
        set_small_(v);
    } else {
        if (mp_ != nullptr) {
            mp_int_free(mp_);
        }
        small_ = 0;
        mp_ = res.release();
    }
}

template <class F> inline auto Integer::mp_op_(Integer const &a, Integer const &b, F f) -> Integer & {
    auto x = Arg{a};
    auto y = Arg{b};
    auto res = Res{};
    Detail::mp_handle_error(f(x.get(), y.get(), res.get()));
    adopt_(res);
    return *this;
}

inline auto Integer::set_add(Integer const &a, Integer const &b) -> Integer & {
    if (a.mp_ == nullptr && b.mp_ == nullptr) {
        if (auto c = std::int64_t{0}; Detail::small_add(a.small_, b.small_, c)) {
            set_small_(c);
            return *this;
        }
    }
    return mp_op_(a, b, mp_int_add);
}

inline auto Integer::set_add(Integer const &a, fixed_int b) -> Integer & {
    if (a.mp_ == nullptr) {
        if (auto c = std::int64_t{0}; Detail::small_add(a.small_, b, c)) {
            set_small_(c);
            return *this;
        }
    }
    return set_add(a, Integer{b});
}

inline auto Integer::set_sub(Integer const &a, Integer const &b) -> Integer & {
    if (a.mp_ == nullptr && b.mp_ == nullptr) {
        if (auto c = std::int64_t{0}; Detail::small_sub(a.small_, b.small_, c)) {
            set_small_(c);
            return *this;
        }
    }
    return mp_op_(a, b, mp_int_sub);
}

inline auto Integer::set_sub(Integer const &a, fixed_int b) -> Integer & {
    if (a.mp_ == nullptr) {
        if (auto c = std::int64_t{0}; Detail::small_sub(a.small_, b, c)) {
            set_small_(c);
            return *this;
        }
    }
    return set_sub(a, Integer{b});
}

inline auto Integer::set_mul(Integer const &a, Integer const &b) -> Integer & {
    if (a.mp_ == nullptr && b.mp_ == nullptr) {
        if (auto c = std::int64_t{0}; Detail::small_mul(a.small_, b.small_, c)) {
            set_small_(c);
            return *this;
        }
    }
    return mp_op_(a, b, mp_int_mul);
}

inline auto Integer::set_mul(Integer const &a, fixed_int b) -> Integer & {
    if (a.mp_ == nullptr) {
        if (auto c = std::int64_t{0}; Detail::small_mul(a.small_, b, c)) {
            set_small_(c);
            return *this;
        }
    }
    return set_mul(a, Integer{b});
}

inline auto Integer::set_gcd(Integer const &a, Integer const &b) -> Integer & {
    if (a.mp_ == nullptr && b.mp_ == nullptr) {
        set_small_(std::gcd(a.small_, b.small_));
        return *this;
    }
    // NOTE: imath fails if both arguments are zero, which cannot happen here
    return mp_op_(a, b, mp_int_gcd);
}

inline auto Integer::set_divexact(Integer const &a, Integer const &b) -> Integer & {
    if (a.mp_ == nullptr && b.mp_ == nullptr) {
        if (b.small_ == 0) {
            throw std::domain_error("division by zero");
        }
        set_small_(a.small_ / b.small_);
        return *this;
    }
    return mp_op_(a, b, [](mp_int x, mp_int y, mp_int z) { return mp_int_div(x, y, z, nullptr); });
}

inline auto Integer::divide(Integer const &a) -> Integer & { return set_divexact(*this, a); }

inline auto Integer::add_mul(Integer const &a, Integer const &b) & -> Integer & {
    if (mp_ == nullptr && a.mp_ == nullptr && b.mp_ == nullptr) {
        if (auto c = std::int64_t{0}, d = std::int64_t{0};
            Detail::small_mul(a.small_, b.small_, c) && Detail::small_add(small_, c, d)) {
            small_ = d;
            return *this;
        }
    }
    auto c = Integer{};
    c.set_mul(a, b);
    return set_add(*this, c);
}

inline auto Integer::add_mul(Integer const &a, Integer const &b) && -> Integer { return std::move(add_mul(a, b)); }

inline auto Integer::neg() -> Integer & {
    if (mp_ == nullptr) {
        small_ = -small_;
    } else {
        Detail::mp_handle_error(mp_int_neg(mp_, mp_));
    }
    return *this;
}

inline auto Integer::as_int() const -> std::optional<int> {
    if (mp_ == nullptr && std::numeric_limits<int>::min() <= small_ && small_ <= std::numeric_limits<int>::max()) {
        return static_cast<int>(small_);
    }
    return std::nullopt;
}

inline auto Integer::sign() const -> int {
    if (mp_ == nullptr) {
        return Detail::small_cmp(small_, 0);
    }
    return MP_SIGN(mp_) == MP_NEG ? -1 : 1;
}

inline auto Integer::cmp(Integer const &b) const -> int {
    if (mp_ == nullptr && b.mp_ == nullptr) {
        return Detail::small_cmp(small_, b.small_);
    }
    // a number stored by imath has a larger magnitude than a small number
    if (b.mp_ == nullptr) {
        return sign();
    }
    if (mp_ == nullptr) {
        return -b.sign();
    }
    return mp_int_compare(mp_, b.mp_);
}

inline auto Integer::cmp(fixed_int b) const -> int {
    if (mp_ == nullptr) {
        return Detail::small_cmp(small_, b);
    }
    return cmp(Integer{b});
}

inline auto Integer::equal(Integer const &b) const -> bool {
    if (mp_ == nullptr || b.mp_ == nullptr) {
        return mp_ == b.mp_ && small_ == b.small_;
    }
    return mp_int_compare(mp_, b.mp_) == 0;
}

inline auto Integer::equal(fixed_int b) const -> bool { return mp_ == nullptr && small_ == b; }

inline void Integer::print(std::ostream &out) const {
    if (mp_ == nullptr) {
        out << small_;
        return;
    }
    auto len = mp_int_string_len(mp_, BASE);
    auto buf = std::make_unique<char[]>(len); // NOLINT
    Detail::mp_handle_error(mp_int_to_string(mp_, BASE, buf.get(), len));
    out << buf.get();
}

// addition

[[nodiscard]] inline auto operator+(IsInteger auto &&a, fixed_int b) -> Integer {
    if constexpr (std::is_rvalue_reference_v<decltype(a)>) {
        auto ret = std::forward<decltype(a)>(a);
        ret.set_add(ret, b);
        return ret;
    } else {
        Integer c;
        c.set_add(a, b);
        return c;
    }
}

[[nodiscard]] inline auto operator+(IsInteger auto &&a, Integer const &b) -> Integer {
    if constexpr (std::is_rvalue_reference_v<decltype(a)>) {
        auto ret = std::forward<decltype(a)>(a);
        ret.set_add(ret, b);
        return ret;
    } else {
        Integer c;
        c.set_add(a, b);
        return c;
    }
}

inline auto operator+=(Integer &a, fixed_int b) -> Integer & { return a.set_add(a, b); }

inline auto operator+=(Integer &a, Integer const &b) -> Integer & { return a.set_add(a, b); }

// subtraction

[[nodiscard]] inline auto operator-(Integer a) -> Integer {
    a.neg();
    return a;
}

[[nodiscard]] inline auto operator-(IsInteger auto &&a, fixed_int b) -> Integer {
    if constexpr (std::is_rvalue_reference_v<decltype(a)>) {
        auto ret = std::forward<decltype(a)>(a);
        ret.set_sub(ret, b);
        return ret;
    } else {
        Integer c;
        c.set_sub(a, b);
        return c;
    }
}

[[nodiscard]] inline auto operator-(IsInteger auto &&a, Integer const &b) -> Integer {
    if constexpr (std::is_rvalue_reference_v<decltype(a)>) {
        auto ret = std::forward<decltype(a)>(a);
        ret.set_sub(ret, b);
        return ret;
    } else {
        Integer c;
        c.set_sub(a, b);
        return c;
    }
}

inline auto operator-=(Integer &a, fixed_int b) -> Integer & { return a.set_sub(a, b); }

inline auto operator-=(Integer &a, Integer const &b) -> Integer & { return a.set_sub(a, b); }

// multiplication

[[nodiscard]] inline auto operator*(IsInteger auto &&a, fixed_int b) -> Integer {
    if constexpr (std::is_rvalue_reference_v<decltype(a)>) {
        auto ret = std::forward<decltype(a)>(a);
        ret.set_mul(ret, b);
        return ret;
    } else {
        Integer c;
        c.set_mul(a, b);
        return c;
    }
}

[[nodiscard]] inline auto operator*(IsInteger auto &&a, Integer const &b) -> Integer {
    if constexpr (std::is_rvalue_reference_v<decltype(a)>) {
        auto ret = std::forward<decltype(a)>(a);
        ret.set_mul(ret, b);
        return ret;
    } else {
        Integer c;
        c.set_mul(a, b);
        return c;
    }
}

inline auto operator*=(Integer &a, fixed_int b) -> Integer & { return a.set_mul(a, b); }

inline auto operator*=(Integer &a, Integer const &b) -> Integer & { return a.set_mul(a, b); }

// comparison

[[nodiscard]] inline auto compare(Integer const &a, fixed_int b) -> int { return a.cmp(b); }

[[nodiscard]] inline auto compare(Integer const &a, Integer const &b) -> int { return a.cmp(b); }

[[nodiscard]] inline auto operator==(Integer const &a, fixed_int b) -> bool { return a.equal(b); }

[[nodiscard]] inline auto operator==(Integer const &a, Integer const &b) -> bool { return a.equal(b); }

[[nodiscard]] inline auto operator<=>(Integer const &a, fixed_int b) -> std::strong_ordering { return a.cmp(b) <=> 0; }

[[nodiscard]] inline auto operator<=>(Integer const &a, Integer const &b) -> std::strong_ordering {
    return a.cmp(b) <=> 0;
}

// printing

inline auto operator<<(std::ostream &out, Integer const &a) -> std::ostream & {
    a.print(out);
    return out;
}

// gcd

[[nodiscard]] inline auto gcd(Integer const &a, Integer const &b) -> Integer {
    Integer g;
    g.set_gcd(a, b);
    return g;
}

[[nodiscard]] inline auto gcd_div(Integer const &a, Integer const &b) -> std::tuple<Integer, Integer, Integer> {
    std::tuple<Integer, Integer, Integer> ret;
    auto &[g, ag, bg] = ret;
    g.set_gcd(a, b);
    ag.set_divexact(a, g);
    bg.set_divexact(b, g);
    return ret;
}

// Rational

inline Rational::Rational() noexcept { den_.small_ = 1; }

inline Rational::Rational(fixed_int val) : num_{val} { den_.small_ = 1; }

inline Rational::Rational(Integer num, Integer den) : num_{std::move(num)}, den_{std::move(den)} { canonicalize(); }

inline Rational::Rational(char const *val, int radix) {
    auto q = mpq_t{};
    Detail::mp_handle_error(mp_rat_init(&q));
    try {
        Detail::mp_handle_error(mp_rat_read_string(&q, radix, val));
        num_.set_mp_(mp_rat_numer_ref(&q));
        den_.set_mp_(mp_rat_denom_ref(&q));
    } catch (...) {
        mp_rat_clear(&q);
        throw;
    }
    mp_rat_clear(&q);
    canonicalize();
}

inline Rational::Rational(std::string const &val, int radix) : Rational(val.c_str(), radix) {}

inline auto Rational::num() -> Integer & { return num_; }

inline auto Rational::num() const -> Integer const & { return num_; }

inline auto Rational::den() -> Integer & { return den_; }

inline auto Rational::den() const -> Integer const & { return den_; }

inline auto Rational::neg() -> Rational & {
    num_.neg();
    return *this;
}

inline void Rational::swap(Rational &x) noexcept {
    num_.swap(x.num_);
    den_.swap(x.den_);
}

inline void Rational::canonicalize() {
    auto ds = den_.sign();
    if (ds == 0) {
        throw std::domain_error("division by zero");
    }
    if (ds < 0) {
        num_.neg();
        den_.neg();
    }
    if (den_ == 1) {
        return;
    }
    auto g = gcd(num_, den_);
    if (g != 1) {
        num_.divide(g);
        den_.divide(g);
    }
}

inline void Rational::assign_(Integer &&num, Integer &&den) noexcept {
    num_ = std::move(num);
    den_ = std::move(den);
}

inline void Rational::set_zero_() noexcept {
    num_.set_small_(0);
    den_.set_small_(1);
}

template <bool Sub> inline auto Rational::add_sub_(Rational const &a, Rational const &b) -> Rational & {
    // NOTE: see Knuth, The Art of Computer Programming, Vol. 2, Section 4.5.1
    auto combine = [](Integer &c, Integer const &x, Integer const &y) {
        if constexpr (Sub) {
            c.set_sub(x, y);
        } else {
            c.set_add(x, y);
        }
    };
    Integer n;
    Integer d;
    if (a.den_ == b.den_) {
        combine(n, a.num_, b.num_);
        if (a.den_ == 1) {
            d = 1;
        } else if (auto g = gcd(n, a.den_); g == 1) {
            d = a.den_;
        } else {
            n.divide(g);
            d.set_divexact(a.den_, g);
        }
    } else if (a.den_ == 1) {
        // for x + y/v, the result (x*v + y)/v is canonical because gcd(x*v + y, v) = gcd(y, v) = 1
        combine(n, a.num_ * b.den_, b.num_);
        d = b.den_;
    } else if (b.den_ == 1) {
        combine(n, a.num_, b.num_ * a.den_);
        d = a.den_;
    } else {
        auto g = gcd(a.den_, b.den_);
        if (g == 1) {
            combine(n, a.num_ * b.den_, b.num_ * a.den_);
            d.set_mul(a.den_, b.den_);
        } else {
            // for x/u + y/v with g = gcd(u, v), the result is (t/h)/((u/g)*(v/h))
            // where t = x*(v/g) + y*(u/g) and h = gcd(t, g)
            Integer ag;
            ag.set_divexact(a.den_, g);
            Integer bg;
            bg.set_divexact(b.den_, g);
            combine(n, a.num_ * bg, b.num_ * ag);
            auto h = gcd(n, g);
            n.divide(h);
            d.set_divexact(b.den_, h);
            d *= ag;
        }
        if (n == 0) {
            set_zero_();
            return *this;
        }
    }
    assign_(std::move(n), std::move(d));
    return *this;
}

template <bool Sub> inline auto Rational::add_sub_(Rational const &a, Integer const &b) -> Rational & {
    // for x/u + y, the result (x + y*u)/u is canonical because gcd(x + y*u, u) = gcd(x, u) = 1
    auto combine = [](Integer &c, Integer const &x, Integer const &y) {
        if constexpr (Sub) {
            c.set_sub(x, y);
        } else {
            c.set_add(x, y);
        }
    };
    Integer n;
    if (a.den_ == 1) {
        combine(n, a.num_, b);
    } else {
        combine(n, a.num_, b * a.den_);
    }
    num_ = std::move(n);
    if (this != &a) {
        den_ = a.den_;
    }
    return *this;
}

inline auto Rational::set_add(Rational const &a, Rational const &b) -> Rational & { return add_sub_<false>(a, b); }

inline auto Rational::set_add(Rational const &a, Integer const &b) -> Rational & { return add_sub_<false>(a, b); }

inline auto Rational::set_sub(Rational const &a, Rational const &b) -> Rational & { return add_sub_<true>(a, b); }

inline auto Rational::set_sub(Rational const &a, Integer const &b) -> Rational & { return add_sub_<true>(a, b); }

inline auto Rational::set_mul(Rational const &a, Rational const &b) -> Rational & {
    if (a.num_ == 0 || b.num_ == 0) {
        set_zero_();
        return *this;
    }
    if (a.den_ == 1 && b.den_ == 1) {
        num_.set_mul(a.num_, b.num_);
        den_.set_small_(1);
        return *this;
    }
    // cross cancel to keep the result canonical
    auto [g1, an, bd] = gcd_div(a.num_, b.den_);
    auto [g2, bn, ad] = gcd_div(b.num_, a.den_);
    an *= bn;
    ad *= bd;
    assign_(std::move(an), std::move(ad));
    return *this;
}

inline auto Rational::set_mul(Rational const &a, Integer const &b) -> Rational & {
    if (a.num_ == 0 || b == 0) {
        set_zero_();
        return *this;
    }
    if (a.den_ == 1) {
        num_.set_mul(a.num_, b);
        den_.set_small_(1);
        return *this;
    }
    auto [g, bg, ag] = gcd_div(b, a.den_);
    bg *= a.num_;
    assign_(std::move(bg), std::move(ag));
    return *this;
}

inline auto Rational::set_div(Rational const &a, Rational const &b) -> Rational & {
    if (b.num_ == 0) {
        throw std::domain_error("division by zero");
    }
    if (a.num_ == 0) {
        set_zero_();
        return *this;
    }
    // cross cancel to keep the result canonical
    auto [g1, an, bn] = gcd_div(a.num_, b.num_);
    auto [g2, ad, bd] = gcd_div(a.den_, b.den_);
    an *= bd;
    ad *= bn;
    if (ad.sign() < 0) {
        an.neg();
        ad.neg();
    }
    assign_(std::move(an), std::move(ad));
    return *this;
}

inline auto Rational::set_div(Rational const &a, Integer const &b) -> Rational & {
    if (b == 0) {
        throw std::domain_error("division by zero");
    }
    if (a.num_ == 0) {
        set_zero_();
        return *this;
    }
    auto [g, an, bg] = gcd_div(a.num_, b);
    bg *= a.den_;
    if (bg.sign() < 0) {
        an.neg();
        bg.neg();
    }
    assign_(std::move(an), std::move(bg));
    return *this;
}

inline auto Rational::cmp(Rational const &b) const -> int {
    if (auto as = num_.sign(), bs = b.num_.sign(); as != bs) {
        return Detail::small_cmp(as, bs);
    }
    if (den_ == b.den_) {
        return num_.cmp(b.num_);
    }
    return (num_ * b.den_).cmp(b.num_ * den_);
}

inline auto Rational::cmp(Integer const &b) const -> int {
    if (den_ == 1) {
        return num_.cmp(b);
    }
    if (auto as = num_.sign(), bs = b.sign(); as != bs) {
        return Detail::small_cmp(as, bs);
    }
    return num_.cmp(b * den_);
}

inline void Rational::print(std::ostream &out) const {
    out << num_;
    if (den_ != 1) {
        out << "/" << den_;
    }
}

// addition

[[nodiscard]] inline auto operator+(IsRational auto &&a, fixed_int b) -> Rational {
    return std::forward<decltype(a)>(a) + Integer{b};
}

[[nodiscard]] inline auto operator+(IsRational auto &&a, Integer const &b) -> Rational {
    if constexpr (std::is_rvalue_reference_v<decltype(a)>) {
        auto ret = std::forward<decltype(a)>(a);
        ret.set_add(ret, b);
        return ret;
    } else {
        Rational c;
        c.set_add(a, b);
        return c;
    }
}

[[nodiscard]] inline auto operator+(IsRational auto &&a, Rational const &b) -> Rational {
    if constexpr (std::is_rvalue_reference_v<decltype(a)>) {
        auto ret = std::forward<decltype(a)>(a);
        ret.set_add(ret, b);
        return ret;
    } else {
        Rational c;
        c.set_add(a, b);
        return c;
    }
}

inline auto operator+=(Rational &a, fixed_int b) -> Rational & { return a.set_add(a, Integer{b}); }

inline auto operator+=(Rational &a, Integer const &b) -> Rational & { return a.set_add(a, b); }

inline auto operator+=(Rational &a, Rational const &b) -> Rational & { return a.set_add(a, b); }

// subtraction

[[nodiscard]] inline auto operator-(Rational a) -> Rational {
    a.neg();
    return a;
}

[[nodiscard]] inline auto operator-(IsRational auto &&a, fixed_int b) -> Rational {
    return std::forward<decltype(a)>(a) - Integer{b};
}

[[nodiscard]] inline auto operator-(IsRational auto &&a, Integer const &b) -> Rational {
    if constexpr (std::is_rvalue_reference_v<decltype(a)>) {
        auto ret = std::forward<decltype(a)>(a);
        ret.set_sub(ret, b);
        return ret;
    } else {
        Rational c;
        c.set_sub(a, b);
        return c;
    }
}

[[nodiscard]] inline auto operator-(IsRational auto &&a, Rational const &b) -> Rational {
    if constexpr (std::is_rvalue_reference_v<decltype(a)>) {
        auto ret = std::forward<decltype(a)>(a);
        ret.set_sub(ret, b);
        return ret;
    } else {
        Rational c;
        c.set_sub(a, b);
        return c;
    }
}

inline auto operator-=(Rational &a, fixed_int b) -> Rational & { return a.set_sub(a, Integer{b}); }

inline auto operator-=(Rational &a, Integer const &b) -> Rational & { return a.set_sub(a, b); }

inline auto operator-=(Rational &a, Rational const &b) -> Rational & { return a.set_sub(a, b); }

// multiplication

[[nodiscard]] inline auto operator*(IsRational auto &&a, fixed_int b) -> Rational {
    return std::forward<decltype(a)>(a) * Integer{b};
}

[[nodiscard]] inline auto operator*(IsRational auto &&a, Integer const &b) -> Rational {
    if constexpr (std::is_rvalue_reference_v<decltype(a)>) {
        auto ret = std::forward<decltype(a)>(a);
        ret.set_mul(ret, b);
        return ret;
    } else {
        Rational c;
        c.set_mul(a, b);
        return c;
    }
}

[[nodiscard]] inline auto operator*(IsRational auto &&a, Rational const &b) -> Rational {
    if constexpr (std::is_rvalue_reference_v<decltype(a)>) {
        auto ret = std::forward<decltype(a)>(a);
        ret.set_mul(ret, b);
        return ret;
    } else {
        Rational c;
        c.set_mul(a, b);
        return c;
    }
}

inline auto operator*=(Rational &a, fixed_int b) -> Rational & { return a.set_mul(a, Integer{b}); }

inline auto operator*=(Rational &a, Integer const &b) -> Rational & { return a.set_mul(a, b); }

inline auto operator*=(Rational &a, Rational const &b) -> Rational & { return a.set_mul(a, b); }

// division

[[nodiscard]] inline auto operator/(IsRational auto &&a, fixed_int b) -> Rational {
    return std::forward<decltype(a)>(a) / Integer{b};
}

[[nodiscard]] inline auto operator/(IsRational auto &&a, Integer const &b) -> Rational {
    if constexpr (std::is_rvalue_reference_v<decltype(a)>) {
        auto ret = std::forward<decltype(a)>(a);
        ret.set_div(ret, b);
        return ret;
    } else {
        Rational c;
        c.set_div(a, b);
        return c;
    }
}

[[nodiscard]] inline auto operator/(IsRational auto &&a, Rational const &b) -> Rational {
    if constexpr (std::is_rvalue_reference_v<decltype(a)>) {
        auto ret = std::forward<decltype(a)>(a);
        ret.set_div(ret, b);
        return ret;
    } else {
        Rational c;
        c.set_div(a, b);
        return c;
    }
}

inline auto operator/=(Rational &a, fixed_int b) -> Rational & { return a.set_div(a, Integer{b}); }

inline auto operator/=(Rational &a, Integer const &b) -> Rational & { return a.set_div(a, b); }

inline auto operator/=(Rational &a, Rational const &b) -> Rational & { return a.set_div(a, b); }

// comparison

[[nodiscard]] inline auto compare(Rational const &a, fixed_int b) -> int { return a.cmp(Integer{b}); }

[[nodiscard]] inline auto compare(Rational const &a, Integer const &b) -> int { return a.cmp(b); }

[[nodiscard]] inline auto compare(Rational const &a, Rational const &b) -> int { return a.cmp(b); }

[[nodiscard]] inline auto operator==(Rational const &a, fixed_int b) -> bool { return a.den() == 1 && a.num() == b; }

[[nodiscard]] inline auto operator==(Rational const &a, Integer const &b) -> bool {
    return a.den() == 1 && a.num() == b;
}

[[nodiscard]] inline auto operator==(Rational const &a, Rational const &b) -> bool {
    return a.num() == b.num() && a.den() == b.den();
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
    a.print(out);
    return out;
}

} // namespace ClingoLPX

#endif
