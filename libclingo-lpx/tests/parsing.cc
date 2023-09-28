#include "parsing.hh"

#include <catch2/catch_test_macros.hpp>

#include <sstream>

template <typename T> auto str(T &&x) -> std::string {
    std::ostringstream oss;
    oss << x;
    return oss.str();
}

// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
TEST_CASE("parsing") {
    Clingo::Control ctl;
    ctl.add("base", {}, THEORY);
    auto mapper = [](Clingo::literal_t lit) {
        static_cast<void>(lit);
        return 1;
    };

    SECTION("example 1") {
        ctl.add("base", {}, "&sum { x2; x3 } >= 10.\n");
        ctl.ground({{"base", {}}});

        VarMap vars;
        std::vector<Inequality> eqs;
        std::vector<Term> objective;
        evaluate_theory(ctl.theory_atoms(), mapper, vars, eqs, objective);
        REQUIRE(eqs.size() == 1);
        REQUIRE(objective.empty());
        REQUIRE(str(eqs.front()) == "x2 + x3 >= 10");
    }

    SECTION("example 2") {
        ctl.add("base", {}, "&sum { -x } <= 0.\n");
        ctl.ground({{"base", {}}});

        VarMap vars;
        std::vector<Inequality> eqs;
        std::vector<Term> objective;
        evaluate_theory(ctl.theory_atoms(), mapper, vars, eqs, objective);
        REQUIRE(eqs.size() == 1);
        REQUIRE(objective.empty());
        REQUIRE(str(eqs.front()) == "-x <= 0");
    }

    SECTION("example 3") {
        ctl.add("base", {}, "&sum {  -x; -2/(-3)*y } = -1.\n");
        ctl.ground({{"base", {}}});

        VarMap vars;
        std::vector<Inequality> eqs;
        std::vector<Term> objective;
        evaluate_theory(ctl.theory_atoms(), mapper, vars, eqs, objective);
        REQUIRE(eqs.size() == 1);
        REQUIRE(objective.empty());
        REQUIRE(str(eqs.front()) == "-x + 2/3*y = -1");
    }

    SECTION("example 3") {
        ctl.add("base", {}, "&minimize { 3*x }. &maximize { -y }.");
        ctl.ground({{"base", {}}});

        VarMap vars;
        std::vector<Inequality> eqs;
        std::vector<Term> objective;
        evaluate_theory(ctl.theory_atoms(), mapper, vars, eqs, objective);
        REQUIRE(eqs.empty());
        REQUIRE(objective.size() == 2);
        std::sort(objective.begin(), objective.end(),
                  [](auto &a, auto &b) { return std::make_pair(a.var, a.co) < std::make_pair(b.var, b.co); });
        REQUIRE(str(Inequality{objective, 0, Relation::Equal, 0}) == "-3*x + -y = 0");
    }
}
