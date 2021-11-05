#include "../src/parsing.hh"
#include "catch.hpp"

#include <sstream>

template <typename T>
std::string str(T &&x) {
    std::ostringstream oss;
    oss << x;
    return oss.str();
}

TEST_CASE("parsing") {
    Clingo::Control ctl;
    ctl.add("base", {}, THEORY);

    SECTION("example 1") {
        ctl.add("base", {}, "&sum { x2; x3 } >= 10.\n");
        ctl.ground({{"base", {}}});

        VarMap vars;
        std::vector<Inequality> eqs;
        evaluate_theory(ctl.theory_atoms(), vars, eqs);
        REQUIRE(eqs.size() == 1);
        REQUIRE(str(eqs.front()) == "x2 + x3 >= 10");
    }

    SECTION("example 2") {
        ctl.add("base", {}, "&sum { -x } <= 0.\n");
        ctl.ground({{"base", {}}});

        VarMap vars;
        std::vector<Inequality> eqs;
        evaluate_theory(ctl.theory_atoms(), vars, eqs);
        REQUIRE(eqs.size() == 1);
        REQUIRE(str(eqs.front()) == "-x <= 0");
    }

    SECTION("example 3") {
        ctl.add("base", {}, "&sum {  -x; -2/(-3)*y } = -1.\n");
        ctl.ground({{"base", {}}});

        VarMap vars;
        std::vector<Inequality> eqs;
        evaluate_theory(ctl.theory_atoms(), vars, eqs);
        REQUIRE(eqs.size() == 1);
        REQUIRE(str(eqs.front()) == "-x + 2/3*y = -1");
    }
};

