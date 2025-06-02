#include "tableau.hh"

#include <catch2/catch_test_macros.hpp>

namespace {

auto as_num_mat(Tableau const &tab, index_t m, index_t n) -> std::vector<std::vector<Rational>> {
    std::vector<std::vector<Rational>> ret;
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

auto as_int_mat(Tableau &tab, index_t m, index_t n) -> std::vector<std::vector<Integer>> {
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

} // namespace

// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers)
// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
TEST_CASE("matrix") {
    Tableau tab;

    SECTION("fraction") {
        Integer *num = nullptr;
        Integer *den = nullptr;

        tab.set(0, 0, Rational{Integer{2}, Integer{4}});
        tab.set(0, 1, Rational{Integer{1}, Integer{3}});
        tab.set(0, 2, Rational{Integer{1}, Integer{5}});
        tab.set(0, 3, Rational{Integer{1}, Integer{2L * 3 * 5 * 7}});

        tab.unsafe_get(0, 0, num, den);
        REQUIRE(*num == (3L * 5 * 7));
        REQUIRE(*den == (2L * 3 * 5 * 7));

        tab.unsafe_get(0, 1, num, den);
        REQUIRE(*num == (2L * 5 * 7));

        tab.unsafe_get(0, 2, num, den);
        REQUIRE(*num == (2L * 3 * 7));

        tab.unsafe_get(0, 3, num, den);
        REQUIRE(*num == 1);

        tab.set(0, 4, Rational{Integer{7}, Integer{2L * 3 * 5 * 7}});

        tab.unsafe_get(0, 4, num, den);
        REQUIRE(*num == 7);
        REQUIRE(*den == (2L * 3 * 5 * 7));
    }

    SECTION("integer") {
        // check default value 0 at (0,0)
        REQUIRE(tab.empty());
        REQUIRE(tab.get(0, 0) == 0);

        // set 2 at (0,0)
        tab.set(0, 0, 2);
        REQUIRE(tab.size() == 1);
        REQUIRE(tab.get(0, 0) == 2);

        // set 3 at (0,0)
        tab.set(0, 0, 3);
        REQUIRE(tab.size() == 1);
        REQUIRE(tab.get(0, 0) == 3);

        // remove at (0,0)
        tab.set(0, 0, 0);
        REQUIRE(tab.get(0, 0) == 0);
        REQUIRE(tab.empty());

        // set 1 at (0,2)
        tab.set(0, 2, 1);
        REQUIRE(tab.size() == 1);

        // traverse the first row
        tab.update_row(0, [](index_t j, Integer &a_0j, Integer &d_0) {
            REQUIRE(j == 2);
            REQUIRE(a_0j == 1);
            REQUIRE(d_0 == 1);
        });
        REQUIRE(tab.size() == 1);

        // traverse the third column
        tab.update_col(2, [](index_t i, Integer &a_2j, Integer &d_2) {
            REQUIRE(i == 0);
            REQUIRE(a_2j == 1);
            REQUIRE(d_2 == 1);
        });
        REQUIRE(tab.size() == 1);

        // traverse the first column
        size_t n = 0;
        tab.update_col(0, [&n](index_t i, Integer &a_i0, Integer &d_i) {
            static_cast<void>(i);
            static_cast<void>(a_i0);
            static_cast<void>(d_i);
            ++n;
        });
        REQUIRE(n == 0);
        REQUIRE(tab.size() == 1);
    }

    SECTION("pivot") {
        // row 0
        tab.set(0, 0, Rational{Integer{1}, Integer{1}});
        tab.set(0, 1, Rational{Integer{2}, Integer{1}});
        tab.set(0, 2, Rational{Integer{3}, Integer{1}});
        // row 1
        tab.set(1, 0, Rational{Integer{4}, Integer{1}});
        tab.set(1, 1, Rational{Integer{5}, Integer{1}});
        tab.set(1, 2, Rational{Integer{6}, Integer{1}});
        // row 2
        tab.set(2, 0, Rational{Integer{7}, Integer{1}});
        tab.set(2, 1, Rational{Integer{8}, Integer{1}});
        tab.set(2, 2, Rational{Integer{9}, Integer{1}});

        Integer *num = nullptr;
        Integer *den = nullptr;
        index_t i = 1;
        index_t j = 1;

        tab.unsafe_get(i, j, num, den);
        tab.pivot(i, j, *num, *den);

        std::vector<std::vector<Rational>> num_ret = as_num_mat(tab, 3, 3);
        std::vector<std::vector<Integer>> int_ret = as_int_mat(tab, 3, 3);

        std::vector<std::vector<Rational>> num_sol = {
            {{-3, 5}, {2, 5}, {3, 5}}, {{-4, 5}, {1, 5}, {-6, 5}}, {{3, 5}, {8, 5}, {-3, 5}}};
        std::vector<std::vector<Integer>> int_sol = {{-3, 2, 3, 5}, {-4, 1, -6, 5}, {3, 8, -3, 5}};

        REQUIRE(num_ret == num_sol);
        REQUIRE(int_ret == int_sol);
    }
}
// NOLINTEND(cppcoreguidelines-avoid-magic-numbers)
