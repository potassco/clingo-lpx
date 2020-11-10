#include <parsing.hh>
#include <solving.hh>

#include <catch.hpp>

namespace {

bool run(char const *s) {
    Clingo::Control ctl;
    ctl.add("base", {}, THEORY);

    ctl.add("base", {}, s);
    ctl.ground({{"base", {}}});

    ClingoLPPropagator prp;
    ctl.register_propagator(prp);
    return ctl.solve(Clingo::LiteralSpan{}, nullptr, false, false).get().is_satisfiable();
}

} // namespace

TEST_CASE("solving") {
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
};

