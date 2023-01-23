#include "matrix.hh"

#include <catch2/catch_test_macros.hpp>

// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
TEST_CASE("matrix") {
    Matrix mat;

    // check default value 0 at (0,0)
    REQUIRE(mat.empty());
    REQUIRE(mat.get(0, 0) == 0);

    // set 2 at (0,0)
    mat.set(0, 0, 2);
    REQUIRE(mat.size() == 1);
    REQUIRE(mat.get(0, 0) == 2);

    // set 3 at (0,0)
    mat.set(0, 0, 3);
    REQUIRE(mat.size() == 1);
    REQUIRE(mat.get(0, 0) == 3);

    // remove at (0,0)
    mat.set(0, 0, 0);
    REQUIRE(mat.get(0, 0) == 0);
    REQUIRE(mat.empty());

    // set 1 at (0,2)
    mat.set(0, 2, 1);
    REQUIRE(mat.size() == 1);

    // traverse the first row
    mat.update_row(0, [](index_t j, Number &a) {
        REQUIRE(j == 2);
        REQUIRE(a == 1);
    });
    REQUIRE(mat.size() == 1);

    // traverse the third column
    mat.update_col(2, [](index_t i, Number &a) {
        REQUIRE(i == 0);
        REQUIRE(a == 1);
    });
    REQUIRE(mat.size() == 1);

    // traverse the first column
    mat.update_col(0, [](index_t j, Number &a) {
        static_cast<void>(j);
        static_cast<void>(a);
    });
    REQUIRE(mat.size() == 1);
}
