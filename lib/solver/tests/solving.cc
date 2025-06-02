#include "solving.hh"
#include "parsing.hh"

#include <catch2/catch_test_macros.hpp>

#include <clingo.hh>
#include <iterator>
#include <optional>

namespace {

template <typename V> class SHM : public Clingo::SolveEventHandler {
  public:
    SHM(Propagator<V> &prp) : prp_{prp} {}
    auto on_model(Clingo::Model &model) -> bool override {
        prp_.on_model(model);
        val_ = prp_.get_objective(model.thread_id());
        return true;
    }
    auto get_objective() const -> std::optional<std::pair<V, bool>> const & { return val_; }

  private:
    std::optional<std::pair<V, bool>> val_;
    Propagator<V> &prp_;
};

Options const options{SelectionHeuristic::Conflict, StoreSATAssignments::Partial, std::nullopt, PropagateMode::Changed,
                      true};

template <typename V = Rational> auto run(char const *s) -> bool {
    Propagator<V> prp{options};
    Clingo::Control ctl;
    prp.register_control(ctl);

    ctl.add("base", {}, s);
    ctl.ground({{"base", {}}});

    return ctl.solve(Clingo::LiteralSpan{}, nullptr, false, false).get().is_satisfiable();
}

template <typename V = Rational>
auto run_o(char const *s, bool global = false, long c = 0, long k = 0) -> std::optional<std::pair<V, bool>> {
    Options opts = options;
    if (global) {
        opts.global_objective = RationalQ{Rational{c}, Rational{k}};
    }
    Propagator<V> prp{opts};
    SHM<V> shm{prp};
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

auto run_m(std::initializer_list<char const *> m) -> size_t {
    Propagator<Rational> prp{options};
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

constexpr char const *knapsack = R"(
item(1..5).

weight(1,8;
     2,3;
     3,3;
     4,4;
     5,6).

value(1,160;
      2,50;
      3,50;
      4,70;
      5,110).

bound(10).

{ pack(I) } :- item(I).

&sum { pack(I) } >= 0 :- weight(I,_).
&sum { pack(I) } <= 1 :- weight(I,_).
&sum { pack(I) } >= 1 :- pack(I).
&sum { pack(I) } <= 0 :- item(I), not pack(I).

&sum { W*pack(I): weight(I,W) } <= B :- bound(B).

&maximize { P*pack(I): value(I,P) }.
)";

} // namespace

// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
TEST_CASE("solving") {
    SECTION("non-strict") {
        REQUIRE(run("&sum { x1; x2 } <= 20.\n"
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

        REQUIRE(run("&sum {   x;   y } >= 2.\n"
                    "&sum { 2*x;  -y } >= 0.\n"
                    "&sum {  -x; 2*y } >= 1.\n"));
    }
    SECTION("strict") {
        REQUIRE(run<RationalQ>("&sum { x1; x2 } < 20.\n"
                               "&sum { x1; x3 } =  5.\n"
                               "&sum { x2; x3 } > 10.\n"));

        REQUIRE(!run<RationalQ>("&sum { x } > 2.\n"
                                "&sum { x } < 0.\n"));

        REQUIRE(!run<RationalQ>("&sum { -x } < -2.\n"
                                "&sum {  x } <  0.\n"));

        REQUIRE(!run<RationalQ>("&sum { 4*x } < 4.\n"
                                "&sum {   x } > 2.\n"));

        REQUIRE(!run<RationalQ>("&sum { x; y } > 2.\n"
                                "&sum { x; y } < 0.\n"
                                "&sum {    y } = 0.\n"));

        REQUIRE(run<RationalQ>("&sum {   x;   y } > 2.\n"
                               "&sum { 2*x;  -y } > 0.\n"
                               "&sum {  -x; 2*y } > 1.\n"));

        REQUIRE(!run<RationalQ>("&sum { x; -y } > 0.\n"
                                "&sum { y; -z } > 0.\n"
                                "&sum { z; -x } > 0.\n"));
    }
    SECTION("multi-shot") {
        REQUIRE(run_m({"&sum { x1; x2 } <= 20.\n"
                       "&sum { x1; x2 } >= 10.\n",
                       "&sum { x1 } >= 30.\n"
                       "&sum { x3 } >= 1.\n",
                       "&sum { x2 } >= 10.\n"}) == 2);
        REQUIRE(run_m({"{a; b}.\n"
                       "&sum { x1: a; x2: b } <= 10.\n"
                       "&sum { x1: a; x2: b } >= 10.\n",
                       ":- a.\n"
                       ":- b."}) == 3);
    }
    SECTION("optimize") {
        REQUIRE(run_o("&sum {   x_1; 2*x_2; 3*x_3 } <= 30.\n"
                      "&sum { 2*x_1; 2*x_2; 5*x_3 } <= 24.\n"
                      "&sum { 4*x_1;   x_2; 2*x_3 } <= 36.\n"
                      "&sum { x_1 } >= 0.\n"
                      "&sum { x_2 } >= 0.\n"
                      "&sum { x_3 } >= 0.\n"
                      "&maximize { 3*x_1; x_2; 2*x_3 }.\n") == std::make_pair(Rational{28}, true));
        REQUIRE(run_o("&sum {   x_1; 2*x_2; 3*x_3 } <= 30.\n"
                      "&sum { 2*x_1; 2*x_2; 5*x_3 } <= 24.\n"
                      "&sum { 4*x_1;   x_2; 2*x_3 } <= 36.\n"
                      "&maximize { 3*x_1; x_2; 2*x_3 }.\n") == std::make_pair(Rational{378, 13}, true));
        REQUIRE(run_o("&sum { 2*x_1;  -x_2 } <= 2.\n"
                      "&sum { x_1;  -5*x_2 } <= -4.\n"
                      "&maximize { 2*x_1; -x_2 }.\n") == std::make_pair(Rational{2}, true));
        REQUIRE(!run_o("&sum { x; y } >= 7.\n"
                       "&sum { y } >= 3.\n"
                       "&maximize { 8*x; -5*y }.\n")
                     ->second);
    }
    SECTION("optimize-global") {
        REQUIRE(run_o("{ a; b }.\n"
                      "&sum { a; b } <= 5.\n"
                      "&sum { a } <= 2 :- a.\n"
                      "&sum { b } <= 2 :- b.\n"
                      "&maximize { a; b }.\n",
                      true) == std::make_pair(Rational{5}, true));
        REQUIRE(!run_o("{ a; b }.\n"
                       "&sum { a } <= 5 :- a.\n"
                       "&sum { b } <= 5 :- a.\n"
                       "&sum { b } <= 5 :- b.\n"
                       "&sum { a; b } <= 10 :- not a, not b.\n"
                       "&maximize { a; b }.\n",
                       true)
                     ->second);
        REQUIRE(run_o(knapsack, true) == std::make_pair(Rational{180}, true));
        REQUIRE(run_o<RationalQ>(knapsack, true, 0, 1) == std::make_pair(RationalQ{Rational{180}}, true));
    }
}
