#include <parsing.hh>

#include <catch.hpp>

TEST_CASE("theory") {
    Clingo::Control ctl;
    ctl.add("base", {}, THEORY);

    SECTION("example 1") {
        ctl.add("base", {},
            "&sum { x1; x2 } <= 20.\n"
            "&sum { x1; x3 } = 5.\n"
            "&sum { x2; x3 } >= 10.\n");
        ctl.ground({{"base", {}}});

        for (auto const &eq : evaluate_theory(ctl.theory_atoms())) {
            std::cerr << eq << std::endl;
        }
    }

    SECTION("example 2") {
        ctl.add("base", {},
            "&sum { x } >= 2.\n"
            "&sum { x } <= 0.\n");
        ctl.ground({{"base", {}}});

        for (auto const &eq : evaluate_theory(ctl.theory_atoms())) {
            std::cerr << eq << std::endl;
        }
    }

    SECTION("example 3") {
        ctl.add("base", {},
            "&sum {   x;   y } >= 2.\n"
            "&sum { 2*x;  -y } >= 0.\n"
            "&sum {  -x; 2*y } >= 1.\n"
            );
        ctl.ground({{"base", {}}});

        for (auto const &eq : evaluate_theory(ctl.theory_atoms())) {
            std::cerr << eq << std::endl;
        }
    }
};

