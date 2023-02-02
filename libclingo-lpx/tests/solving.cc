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
    std::optional<Rational> const &get_objective() const {
        return val_;
    }
private:
    std::optional<Rational> val_;
    Propagator<Rational, Rational> &prp_;
};

bool run(char const *s) {
    Propagator<Rational, Rational> prp{SelectionHeuristic::Match, true};
    Clingo::Control ctl;
    prp.register_control(ctl);

    ctl.add("base", {}, s);
    ctl.ground({{"base", {}}});

    return ctl.solve(Clingo::LiteralSpan{}, nullptr, false, false).get().is_satisfiable();
}

std::optional<Rational> run_m(char const *s) {
    Propagator<Rational, Rational> prp{SelectionHeuristic::Match, true};
    SHM shm{prp};
    Clingo::Control ctl;
    prp.register_control(ctl);

    ctl.add("base", {}, s);
    ctl.ground({{"base", {}}});

    if (!ctl.solve(Clingo::LiteralSpan{}, &shm, false, false).get().is_satisfiable()) {
        return std::nullopt;
    }
    return shm.get_objective();
}

bool run_q(char const *s) {
    Propagator<Rational, RationalQ> prp{SelectionHeuristic::Match, true};
    Clingo::Control ctl;
    prp.register_control(ctl);

    ctl.add("base", {}, s);
    ctl.ground({{"base", {}}});

    return ctl.solve(Clingo::LiteralSpan{}, nullptr, false, false).get().is_satisfiable();
}

size_t run_m(std::initializer_list<char const *> m) {
    Propagator<Rational, Rational> prp{SelectionHeuristic::Match, true};
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
        REQUIRE( run_m("&sum { 2*x_1;  -x_2 } <= 2.\n"
                       "&sum { x_1;  -5*x_2 } <= -4.\n"
                       "&maximize { 2*x_1; -x_2 }.\n") == Rational{2});
    }
}
