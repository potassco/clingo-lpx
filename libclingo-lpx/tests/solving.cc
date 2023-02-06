#include "solving.hh"
#include "parsing.hh"

#include <catch2/catch_test_macros.hpp>

#include <clingo.hh>
#include <iterator>
#include <optional>

namespace {

class SHM : public Clingo::SolveEventHandler {
public:
    SHM(Propagator<Rational, Rational> &prp)
    : prp_{prp} { }
    bool on_model(Clingo::Model &model) override {
        val_ = prp_.get_objective(model.thread_id());
        return true;
    }
    std::optional<std::pair<Rational, bool>> const &get_objective() const {
        return val_;
    }
private:
    std::optional<std::pair<Rational, bool>> val_;
    Propagator<Rational, Rational> &prp_;
};

Options const options{SelectionHeuristic::Conflict, StoreSATAssignments::Partial, std::nullopt, true, true};

bool run(char const *s) {
    Propagator<Rational, Rational> prp{options};
    Clingo::Control ctl;
    prp.register_control(ctl);

    ctl.add("base", {}, s);
    ctl.ground({{"base", {}}});

    return ctl.solve(Clingo::LiteralSpan{}, nullptr, false, false).get().is_satisfiable();
}

std::optional<std::pair<Rational, bool>> run_o(char const *s, bool global = false) {
    Options opts = options;
    if (global) {
        opts.global_objective = RationalQ{Rational{0}, Rational{0}};
    }
    Propagator<Rational, Rational> prp{opts};
    SHM shm{prp};
    Clingo::Control ctl;
    if (global) {
        ctl.configuration()["solve"]["models"] = "0";
    }
    prp.register_control(ctl);

    ctl.add("base", {}, s);
    ctl.ground({{"base", {}}});

    if (!ctl.solve(Clingo::LiteralSpan{}, &shm, false, false).get().is_satisfiable()) {
        return std::nullopt;
    }
    return shm.get_objective();
}

bool run_q(char const *s) {
    Propagator<Rational, RationalQ> prp{options};
    Clingo::Control ctl;
    prp.register_control(ctl);

    ctl.add("base", {}, s);
    ctl.ground({{"base", {}}});

    return ctl.solve(Clingo::LiteralSpan{}, nullptr, false, false).get().is_satisfiable();
}

size_t run_m(std::initializer_list<char const *> m) {
    Propagator<Rational, Rational> prp{options};
    Clingo::Control ctl{{"0"}};
    prp.register_control(ctl);

    int i = 0;
    int l = 0;
    for (auto const *s : m) {
        std::string n = "base" + std::to_string(i++);
        ctl.add(n.c_str(), {}, s);
        ctl.ground({{n.c_str(), {}}});
        auto h = ctl.solve();
        l += static_cast<int>(std::distance(begin(h), end(h)));
    }
    return l;
}

} // namespace

// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
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
    SECTION("multi-shot") {
        REQUIRE( run_m({"&sum { x1; x2 } <= 20.\n"
                        "&sum { x1; x2 } >= 10.\n",
                        "&sum { x1 } >= 30.\n"
                        "&sum { x3 } >= 1.\n",
                        "&sum { x2 } >= 10.\n"}) == 2);
        REQUIRE( run_m({"{a; b}.\n"
                        "&sum { x1: a; x2: b } <= 10.\n"
                        "&sum { x1: a; x2: b } >= 10.\n",
                        ":- a.\n"
                        ":- b."}) == 3);
    }
    SECTION("optimize") {
        REQUIRE( run_o("&sum {   x_1; 2*x_2; 3*x_3 } <= 30.\n"
                       "&sum { 2*x_1; 2*x_2; 5*x_3 } <= 24.\n"
                       "&sum { 4*x_1;   x_2; 2*x_3 } <= 36.\n"
                       "&sum { x_1 } >= 0.\n"
                       "&sum { x_2 } >= 0.\n"
                       "&sum { x_3 } >= 0.\n"
                       "&maximize { 3*x_1; x_2; 2*x_3 }.\n") == std::make_pair(Rational{28}, true));
        REQUIRE( run_o("&sum {   x_1; 2*x_2; 3*x_3 } <= 30.\n"
                       "&sum { 2*x_1; 2*x_2; 5*x_3 } <= 24.\n"
                       "&sum { 4*x_1;   x_2; 2*x_3 } <= 36.\n"
                       "&maximize { 3*x_1; x_2; 2*x_3 }.\n") == std::make_pair(Rational{378, 13}, true));
        REQUIRE( run_o("&sum { 2*x_1;  -x_2 } <= 2.\n"
                       "&sum { x_1;  -5*x_2 } <= -4.\n"
                       "&maximize { 2*x_1; -x_2 }.\n") == std::make_pair(Rational{2}, true));
        REQUIRE(!run_o("&sum { x; y } >= 7.\n"
                       "&sum { y } >= 3.\n"
                       "&maximize { 8*x; -5*y }.\n")->second);
    }
    SECTION("optimize-global") {
        REQUIRE(run_o("{ a; b }.\n"
                      "&sum { a; b } <= 5.\n"
                      "&sum { a } <= 2 :- a.\n"
                      "&sum { b } <= 2 :- b.\n"
                      "&maximize { a; b }.\n", true) == std::make_pair(Rational{5}, true));
    }
}
