#include <util.hh>

#include <catch.hpp>

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

        // remove at (0,0)
        t.set(0, 0, 0);
        REQUIRE(t.get(0, 0) == 0);
        REQUIRE(t.size() == 1);

        // set 1 at (0,2)
        t.set(0, 2, 1);
        REQUIRE(t.size() == 2);

        // traverse the first row
        t.update_row(0, [](index_t j, Number &a) {
            REQUIRE(j == 2);
            REQUIRE(a == 1);
        });
        REQUIRE(t.size() == 2);

        // traverse the third column
        t.update_col(2, [](index_t i, Number &a) {
            REQUIRE(i == 0);
            REQUIRE(a == 1);
        });
        REQUIRE(t.size() == 2);

        // traverse the first column
        t.update_col(0, [](index_t j, Number &a) { });
        REQUIRE(t.size() == 1);
    }
};
