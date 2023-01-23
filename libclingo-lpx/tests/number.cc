#include "number.hh"

#include <catch2/catch_test_macros.hpp>

// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
TEST_CASE("number") {
    NumberQ a{Number(4), Number(3)};
    NumberQ b{Number(2), Number(1)};
    Number c{3};

    REQUIRE(a + b == NumberQ{6, 4});
    REQUIRE(b + a == NumberQ{6, 4});
    REQUIRE(a + c == NumberQ{7, 3});
    REQUIRE(c + a == NumberQ{7, 3});

    REQUIRE(a - b == NumberQ{2, 2});
    REQUIRE(b - a == NumberQ{-2, -2});
    REQUIRE(a - c == NumberQ{1, 3});
    REQUIRE(c - a == NumberQ{-1, -3});

    REQUIRE((a += b) == NumberQ{6, 4});
    REQUIRE((a -= b) == NumberQ{4, 3});
    REQUIRE((a += c) == NumberQ{7, 3});
    REQUIRE((a -= c) == NumberQ{4, 3});

    REQUIRE(a * c == NumberQ{12, 9});
    REQUIRE(c * a == NumberQ{12, 9});

    REQUIRE(a / c == NumberQ{Number{"4/3", 10}, 1});

    REQUIRE((a *= c) == NumberQ{12, 9});
    REQUIRE((a /= c) == NumberQ{4, 3});
}

