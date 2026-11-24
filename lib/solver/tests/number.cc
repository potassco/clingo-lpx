#include <clingo-lpx/number.hh>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>

#include <vector>

namespace ClingoLPX::Test {

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

// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
TEST_CASE("number-boundaries") {
    // The arithmetic has fast paths for numbers fitting into machine words.
    // We compare against results computed with large numbers, which take the
    // general code paths.
    auto big = Integer{"1267650600228229401496703205376", 10}; // 2^100
    std::vector<Integer> values;
    for (auto const *v : {
             "0",
             "1",
             "2",
             "3",
             "6",
             "7",
             "2147483647",
             "2147483648",
             "3037000499",
             "3037000500",
             "1350851717672992089",
             "2305843009213693951",
             "4611686018427387903",
             "4611686018427387904",
             "9223372036854775807",
             "9223372036854775808",
             "18446744073709551617",
             "1000000000000000000000000000000",
         }) {
        values.emplace_back(v, 10);
        values.emplace_back(v, 10);
        values.back().neg();
    }
    auto mul = [&](Integer const &a, Integer const &b) -> Integer { return (a + big) * b - big * b; };
    for (auto const &a : values) {
        for (auto const &b : values) {
            INFO(a << " " << b);
            REQUIRE(a * b == mul(a, b));
            auto c = a;
            REQUIRE((c *= b) == mul(a, b));
            for (auto const &d : {values[4], values[17], values[27]}) {
                auto e = d;
                REQUIRE(e.add_mul(a, b) == d + mul(a, b));
            }
            auto g = gcd(a, b);
            REQUIRE(g == gcd(mul(a, big), mul(b, big)).divide(big));
            REQUIRE(g >= 0);
            if (g != 0) {
                auto [h, ag, bg] = gcd_div(a, b);
                REQUIRE(h == g);
                REQUIRE(mul(ag, g) == a);
                REQUIRE(mul(bg, g) == b);
            }
            if (b != 0) {
                auto ab = mul(a, b);
                REQUIRE(ab.divide(b) == a);
            }
            REQUIRE((a == 1) == (a + big == Integer{1} + big));
            REQUIRE((a == -1) == (a + big == Integer{-1} + big));
        }
    }
    std::vector<Rational> rationals;
    for (auto const &a : values) {
        for (auto const &b : {values[2], values[5], values[11], values[17], values[19], values[24]}) {
            rationals.emplace_back(a, b);
        }
    }
    for (auto const &x : rationals) {
        for (auto const &y : rationals) {
            INFO(x << " " << y);
            auto diff = mul(x.num(), y.den()) - mul(y.num(), x.den());
            REQUIRE(compare(x, y) == (diff < 0 ? -1 : (diff > 0 ? 1 : 0)));
            auto den = mul(x.den(), y.den());
            auto sum = Rational{mul(x.num(), y.den()) + mul(y.num(), x.den()), den};
            auto dif = Rational{diff, den};
            REQUIRE(x + y == sum);
            REQUIRE(x - y == dif);
            auto z = x;
            REQUIRE((z += y) == sum);
            z = x;
            REQUIRE((z -= y) == dif);
            z = x;
            REQUIRE((z += z) == x + x);
            REQUIRE((z -= z) == 0);
        }
        for (auto const &a : values) {
            for (auto const &d : {values[2], values[3], values[10], values[17], values[19], values[24], values[35]}) {
                INFO(x << " " << a << " " << d);
                REQUIRE(mul_div(x, a, d) == Rational{mul(x.num(), a), mul(x.den(), d)});
            }
        }
    }
}

} // namespace ClingoLPX::Test
