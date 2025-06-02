#include "number.hh"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>

// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
TEST_CASE("number") {
    RationalQ a{Rational(4), Rational(3)};
    RationalQ b{Rational(2), Rational(1)};
    Rational c{3};

    REQUIRE(a + b == RationalQ{6, 4});
    REQUIRE(b + a == RationalQ{6, 4});
    REQUIRE(a + c == RationalQ{7, 3});

    REQUIRE(a - b == RationalQ{2, 2});
    REQUIRE(b - a == RationalQ{-2, -2});
    REQUIRE(a - c == RationalQ{1, 3});

    REQUIRE((a += b) == RationalQ{6, 4});
    REQUIRE((a -= b) == RationalQ{4, 3});
    REQUIRE((a += c) == RationalQ{7, 3});
    REQUIRE((a -= c) == RationalQ{4, 3});

    REQUIRE(a * c == RationalQ{12, 9});

    REQUIRE(a / c == RationalQ{Rational{"4/3", 10}, 1});

    REQUIRE((a *= c) == RationalQ{12, 9});
    REQUIRE((a /= c) == RationalQ{4, 3});

    REQUIRE_THROWS(Rational{"xxx", 10});
}
