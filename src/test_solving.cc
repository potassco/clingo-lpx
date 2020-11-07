#include <parsing.hh>
#include <solving.hh>

#include <catch.hpp>

namespace {

bool run(char const *s) {
    Clingo::Control ctl;
    ctl.add("base", {}, THEORY);

    ctl.add("base", {}, s);
    ctl.ground({{"base", {}}});

    Solver slv{evaluate_theory(ctl.theory_atoms())};
    slv.prepare();
    return slv.solve().has_value();
}

} // namespace

TEST_CASE("solving") {
    REQUIRE( run("&sum { x1; x2 } <= 20.\n"
                 "&sum { x1; x3 } = 5.\n"
                 "&sum { x2; x3 } >= 10.\n"));

    REQUIRE(!run("&sum { x } >= 2.\n"
                 "&sum { x } <= 0.\n"));

    REQUIRE( run("&sum {   x;   y } >= 2.\n"
                 "&sum { 2*x;  -y } >= 0.\n"
                 "&sum {  -x; 2*y } >= 1.\n"));
};

