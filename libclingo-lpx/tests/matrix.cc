#include "matrix.hh"

#include <catch2/catch_test_macros.hpp>

namespace {

std::vector<std::vector<Number>> as_num_mat(Matrix const &tab, index_t m, index_t n) {
    std::vector<std::vector<Number>> ret;
    ret.reserve(m);
    for (index_t i = 0; i < m; ++i) {
        ret.emplace_back();
        auto &row = ret.back();
        row.reserve(n);
        for (index_t j = 0; j < n; ++j) {
            row.emplace_back(tab.get(i, j));
        }
    }
    return ret;
}

std::vector<std::vector<Integer>> as_int_mat(Matrix &tab, index_t m, index_t n) {
    std::vector<std::vector<Integer>> ret;
    ret.reserve(m);
    for (index_t i = 0; i < m; ++i) {
        ret.emplace_back();
        auto &row = ret.back();
        row.reserve(n + 1);
        for (index_t j = 0; j < n; ++j) {
            row.emplace_back(0);
        }
        row.emplace_back(1);
        tab.update_row(i, [&row](index_t j, Integer const &num, Integer const &den) {
            row[j] = num;
            row.back() = den;
        });
    }
    return ret;
}

}

// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers)
// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
TEST_CASE("matrix") {
    Matrix mat;

    SECTION("fraction") {
        Integer *num = nullptr;
        Integer *den = nullptr;

        mat.set(0, 0, Number{Integer{2}, Integer{4}});
        mat.set(0, 1, Number{Integer{1}, Integer{3}});
        mat.set(0, 2, Number{Integer{1}, Integer{5}});
        mat.set(0, 3, Number{Integer{1}, Integer{2L*3*5*7}});

        mat.unsafe_get(0, 0, num, den);
        REQUIRE(*num == (3L*5*7));
        REQUIRE(*den == (2L*3*5*7));

        mat.unsafe_get(0, 1, num, den);
        REQUIRE(*num == (2L*5*7));

        mat.unsafe_get(0, 2, num, den);
        REQUIRE(*num == (2L*3*7));

        mat.unsafe_get(0, 3, num, den);
        REQUIRE(*num == 1);

        mat.set(0, 4, Number{Integer{7}, Integer{2L*3*5*7}});

        mat.unsafe_get(0, 4, num, den);
        REQUIRE(*num == 7);
        REQUIRE(*den == (2L*3*5*7));
    }

    SECTION("integer") {
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
        mat.update_row(0, [](index_t j, Integer &a_0j, Integer &d_0) {
            REQUIRE(j == 2);
            REQUIRE(a_0j == 1);
            REQUIRE(d_0 == 1);
        });
        REQUIRE(mat.size() == 1);

        // traverse the third column
        mat.update_col(2, [](index_t i, Integer &a_2j, Integer &d_2) {
            REQUIRE(i == 0);
            REQUIRE(a_2j == 1);
            REQUIRE(d_2 == 1);
        });
        REQUIRE(mat.size() == 1);

        // traverse the first column
        size_t n = 0;
        mat.update_col(0, [&n](index_t i, Integer &a_i0, Integer &d_i) {
            static_cast<void>(i);
            static_cast<void>(a_i0);
            static_cast<void>(d_i);
            ++n;
        });
        REQUIRE(n == 0);
        REQUIRE(mat.size() == 1);
    }

    SECTION("pivot") {
        // row 0
        mat.set(0, 0, Number{Integer{1}, Integer{1}});
        mat.set(0, 1, Number{Integer{2}, Integer{1}});
        mat.set(0, 2, Number{Integer{3}, Integer{1}});
        // row 1
        mat.set(1, 0, Number{Integer{4}, Integer{1}});
        mat.set(1, 1, Number{Integer{5}, Integer{1}});
        mat.set(1, 2, Number{Integer{6}, Integer{1}});
        // row 2
        mat.set(2, 0, Number{Integer{7}, Integer{1}});
        mat.set(2, 1, Number{Integer{8}, Integer{1}});
        mat.set(2, 2, Number{Integer{9}, Integer{1}});

        Integer *num = nullptr;
        Integer *den = nullptr;
        index_t i = 1;
        index_t j = 1;

        mat.unsafe_get(i, j, num, den);
        mat.pivot(i, j, *num, *den);

        std::vector<std::vector<Number>> num_ret = as_num_mat(mat, 3, 3);
        std::vector<std::vector<Integer>> int_ret = as_int_mat(mat, 3, 3);

        std::vector<std::vector<Number>> num_sol = {
            { { 3, -5}, {2, -5}, {-3, -5} },
            { { 4,  5}, {1,  5}, { 6,  5} },
            { {-3, -5}, {8, -5}, { 3, -5} } };
        std::vector<std::vector<Integer>> int_sol = {
            {  3, 2, -3, -5 },
            {  4, 1,  6, 5 },
            { -3, 8,  3, -5 } };

        REQUIRE(num_ret == num_sol);
        REQUIRE(int_ret == int_sol);
    }
}
// NOLINTEND(cppcoreguidelines-avoid-magic-numbers)
