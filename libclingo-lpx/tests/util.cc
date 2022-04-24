#include <util.hh>

#include <catch2/catch.hpp>

TEST_CASE("util") {
    SECTION("tableau") {
        Tableau t;

        // check default value 0 at (0,0)
        REQUIRE(t.empty());
        REQUIRE(t.get(0, 0) == 0);

        // set 2 at (0,0)
        t.set(0, 0, 2);
        REQUIRE(t.size() == 1);
        REQUIRE(t.get(0, 0) == 2);

        // set 3 at (0,0)
        t.set(0, 0, 3);
        REQUIRE(t.size() == 1);
        REQUIRE(t.get(0, 0) == 3);

        // remove at (0,0)
        t.set(0, 0, 0);
        REQUIRE(t.get(0, 0) == 0);
        REQUIRE(t.size() == 0);

        // set 1 at (0,2)
        t.set(0, 2, 1);
        REQUIRE(t.size() == 1);

        // traverse the first row
        t.update_row(0, [](index_t j, Number &a) {
            REQUIRE(j == 2);
            REQUIRE(a == 1);
        });
        REQUIRE(t.size() == 1);

        // traverse the third column
        t.update_col(2, [](index_t i, Number &a) {
            REQUIRE(i == 0);
            REQUIRE(a == 1);
        });
        REQUIRE(t.size() == 1);

        // traverse the first column
        t.update_col(0, [](index_t j, Number &a) { });
        REQUIRE(t.size() == 1);
    }

    SECTION("strict") {
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
};
