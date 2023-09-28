#pragma once

#include <cassert>
#include <limits>
#include <stdexcept>

#ifdef CLINGOLPX_CROSSCHECK
#define assert_extra(X) assert(X) // NOLINT
#else
#define assert_extra(X) // NOLINT
#endif

namespace detail {

template <int X> using int_type = std::integral_constant<int, X>;
template <class T, class S> inline void sc_check(S s, int_type<0> t) { // same sign
    static_cast<void>(t);
    if (!std::is_same<T, S>::value && (s < std::numeric_limits<T>::min() || s > std::numeric_limits<T>::max())) {
        throw std::overflow_error("safe cast failed");
    }
}
template <class T, class S> inline void sc_check(S s, int_type<-1> t) { // Signed -> Unsigned
    static_cast<void>(t);
    if (s < 0 || static_cast<S>(static_cast<T>(s)) != s) {
        throw std::overflow_error("safe cast failed");
    }
}
template <class T, class S> inline void sc_check(S s, int_type<1> t) { // Unsigned -> Signed
    static_cast<void>(t);
    if (s > static_cast<typename std::make_unsigned<T>::type>(std::numeric_limits<T>::max())) {
        throw std::overflow_error("safe cast failed");
    }
}

} // namespace detail

//! A safe numeric cast raising an exception if the target type cannot hold the value.
template <class T, class S> inline auto safe_cast(S s) -> T {
    constexpr int sv =
        static_cast<int>(std::numeric_limits<T>::is_signed) - static_cast<int>(std::numeric_limits<S>::is_signed);
    detail::sc_check<T>(s, detail::int_type<sv>());
    return static_cast<T>(s);
}
