#include <parsing.hh>
#include <solving.hh>

#include <catch.hpp>

namespace {

bool run(char const *s) {
    ClingoLPPropagator<Number, Number> prp;
    Clingo::Control ctl;
    prp.register_control(ctl);

    ctl.add("base", {}, s);
    ctl.ground({{"base", {}}});

    return ctl.solve(Clingo::LiteralSpan{}, nullptr, false, false).get().is_satisfiable();
}

bool run_q(char const *s) {
    ClingoLPPropagator<Number, NumberQ> prp;
    Clingo::Control ctl;
    prp.register_control(ctl);

    ctl.add("base", {}, s);
    ctl.ground({{"base", {}}});

    return ctl.solve(Clingo::LiteralSpan{}, nullptr, false, false).get().is_satisfiable();
}

} // namespace

TEST_CASE("solving") {
    SECTION("non-strict") {
        REQUIRE( run("&sum { x1; x2 } <= 20.\n"
                     "&sum { x1; x3 } =   5.\n"
                     "&sum { x2; x3 } >= 10.\n"));

        REQUIRE(!run("&sum { x } >= 2.\n"
                     "&sum { x } <= 0.\n"));

        REQUIRE(!run("&sum { -x } <= -2.\n"
                     "&sum {  x } <=  0.\n"));

        REQUIRE(!run("&sum { 4*x } <= 4.\n"
                     "&sum {   x } >= 2.\n"));

        REQUIRE(!run("&sum { x; y } >= 2.\n"
                     "&sum { x; y } <= 0.\n"
                     "&sum {    y } =  0.\n"));

        REQUIRE( run("&sum {   x;   y } >= 2.\n"
                     "&sum { 2*x;  -y } >= 0.\n"
                     "&sum {  -x; 2*y } >= 1.\n"));
    }
    SECTION("strict") {
        REQUIRE( run_q("&sum { x1; x2 } < 20.\n"
                       "&sum { x1; x3 } =  5.\n"
                       "&sum { x2; x3 } > 10.\n"));

        REQUIRE(!run_q("&sum { x } > 2.\n"
                       "&sum { x } < 0.\n"));

        REQUIRE(!run_q("&sum { -x } < -2.\n"
                       "&sum {  x } <  0.\n"));

        REQUIRE(!run_q("&sum { 4*x } < 4.\n"
                       "&sum {   x } > 2.\n"));

        REQUIRE(!run_q("&sum { x; y } > 2.\n"
                       "&sum { x; y } < 0.\n"
                       "&sum {    y } = 0.\n"));

        REQUIRE( run_q("&sum {   x;   y } > 2.\n"
                       "&sum { 2*x;  -y } > 0.\n"
                       "&sum {  -x; 2*y } > 1.\n"));

        REQUIRE(!run_q("&sum { x; -y } > 0.\n"
                       "&sum { y; -z } > 0.\n"
                       "&sum { z; -x } > 0.\n"));
    }
};

