#include <clingo-lpx/parsing.hh>
#include <clingo-lpx/problem.hh>
#include <clingo-lpx/solving.hh>

#include "helpers.hh"

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <clingo/control.hh>
#include <clingo/core.hh>
#include <cstdint>
#include <memory>
#include <numeric>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

namespace ClingoLPX::Test {

namespace {

template <typename V> class SHM : public Clingo::SolveEventHandler {
  public:
    explicit SHM(Propagator<V> &prp) : prp_{prp} {}

    auto get_objective() const -> std::optional<std::pair<V, bool>> const & { return val_; }

  private:
    auto do_model(Clingo::Model model) -> bool override {
        prp_.on_model(model);
        val_ = prp_.get_objective(model.thread_id());
        return true;
    }

    std::optional<std::pair<V, bool>> val_;
    Propagator<V> &prp_;
};

template <typename V = Rational>
auto run_o(char const *s, bool global = false, long c = 0, long k = 0) -> std::optional<std::pair<V, bool>> {
    Options opts = options;
    if (global) {
        opts.global_objective = RationalQ{Rational{c}, Rational{k}};
    }
    auto lib = Clingo::Library{};
    auto ctl = Clingo::Control{lib};
    register_theory(ctl);
    auto &prp = ctl.register_propagator(std::make_unique<Propagator<V>>(lib, opts));
    auto shm = SHM<V>{prp};
    if (global) {
        ctl.config()["solve"]["models"] = "0";
    }
    ctl.parse_string(s);
    ctl.ground();

    if (!ctl.solve({}, std::ref(shm)).satisfiable()) {
        return std::nullopt;
    }
    return shm.get_objective();
}

auto run_m(std::initializer_list<std::string_view> m) -> size_t {
    auto lib = Clingo::Library{};
    auto ctl = Clingo::Control{lib, {"0"}};
    register_theory(ctl);
    ctl.register_propagator(std::make_unique<Propagator<Rational>>(lib, options));

    int i = 0;
    int l = 0;
    for (auto s : m) {
        auto part = "base" + std::to_string(i++);
        auto n = "#program " + part + ".\n";
        n.append(s.begin(), s.end());
        ctl.parse_string(n);
        ctl.ground({{part, {}}});
        for (auto h = ctl.start_solve(); [[maybe_unused]] auto m : h) {
            ++l;
        }
    }
    return l;
}

auto random_lp(Random &rng, fixed_int atoms) -> LP {
    auto n = static_cast<size_t>(rng(1, 4));
    auto lp = LP{{}, std::vector<fixed_int>(n, 0), atoms};
    auto nonzero = [&]() -> fixed_int { return rng(0, 1) == 0 ? rng(-3, -1) : rng(1, 3); };
    for (auto m = rng(0, 5); m > 0; --m) {
        // separate statements keep the sequence of random numbers portable
        auto r = rng(0, 4);
        auto rel = r < 2 ? Relation::LessEqual : Relation::GreaterEqual;
        if (r == 4) {
            rel = Relation::Equal;
        }
        auto rhs = rng(0, 2) == 0 ? 0 : rng(-6, 6);
        auto guard = atoms > 0 && rng(0, 2) == 0 ? rng(0, atoms - 1) : -1;
        auto &c = lp.cons.emplace_back(std::vector<fixed_int>(n, 0), rel, rhs, guard);
        for (auto &x : c.co) {
            x = rng(0, 2) == 0 ? 0 : nonzero();
        }
        c.co[static_cast<size_t>(rng(0, static_cast<fixed_int>(n) - 1))] = nonzero();
    }
    // bounds on variables exercise the bound flipping in Solver::optimize
    for (size_t i = 0; i < n; ++i) {
        for (auto rel : {Relation::LessEqual, Relation::GreaterEqual}) {
            if (rng(0, 2) != 0) {
                auto rhs = rel == Relation::LessEqual ? rng(-1, 4) : rng(-4, 1);
                auto &c = lp.cons.emplace_back(std::vector<fixed_int>(n, 0), rel, rhs, -1);
                c.co[i] = 1;
            }
        }
    }
    for (auto &x : lp.obj) {
        x = rng(-2, 2);
    }
    lp.obj[static_cast<size_t>(rng(0, static_cast<fixed_int>(n) - 1))] = nonzero();
    return lp;
}

auto to_program(LP const &lp) -> std::string {
    auto out = std::ostringstream{};
    auto terms = [&](std::vector<fixed_int> const &co) -> void {
        char const *sep = "";
        for (size_t i = 0; i < co.size(); ++i) {
            if (co[i] != 0) {
                out << sep << co[i] << "*x" << i;
                sep = "; ";
            }
        }
    };
    for (fixed_int i = 0; i < lp.atoms; ++i) {
        out << "{ a" << i << " }.\n";
    }
    for (auto const &c : lp.cons) {
        out << "&sum { ";
        terms(c.co);
        out << " } " << c.rel << " " << c.rhs;
        if (c.guard >= 0) {
            out << " :- a" << c.guard;
        }
        out << ".\n";
    }
    out << "&maximize { ";
    terms(lp.obj);
    out << " }.\n";
    return out.str();
}

//! Maximize over the constraints intersected with the box |x_i| <= box by enumerating all vertices.
auto lp_max(std::vector<LPConstraint> cons, std::vector<fixed_int> const &obj, fixed_int box)
    -> std::optional<Rational> {
    auto n = obj.size();
    for (size_t i = 0; i < n; ++i) {
        for (auto [rel, rhs] : {std::pair{Relation::LessEqual, box}, std::pair{Relation::GreaterEqual, -box}}) {
            auto &c = cons.emplace_back(std::vector<fixed_int>(n, 0), rel, rhs, -1);
            c.co[i] = 1;
        }
    }
    auto dot = [](std::vector<fixed_int> const &co, std::vector<Rational> const &x) -> Rational {
        auto s = Rational{};
        for (size_t i = 0; i < co.size(); ++i) {
            s += x[i] * co[i];
        }
        return s;
    };
    auto feasible = [&](std::vector<Rational> const &x) -> bool {
        return std::ranges::all_of(cons, [&](LPConstraint const &c) -> bool {
            auto s = dot(c.co, x);
            switch (c.rel) {
                case Relation::LessEqual: {
                    return s <= c.rhs;
                }
                case Relation::GreaterEqual: {
                    return s >= c.rhs;
                }
                default: {
                    return s == c.rhs;
                }
            }
        });
    };
    std::optional<Rational> best;
    std::vector<size_t> sel(n);
    std::iota(sel.begin(), sel.end(), 0);
    while (true) {
        // solve the system where the selected constraints are tight
        std::vector<std::vector<Rational>> a;
        for (auto k : sel) {
            auto &row = a.emplace_back(cons[k].co.begin(), cons[k].co.end());
            row.emplace_back(cons[k].rhs);
        }
        bool regular = true;
        for (size_t c = 0; c < n && regular; ++c) {
            auto p = c;
            while (p < n && a[p][c] == 0) {
                ++p;
            }
            if (p == n) {
                regular = false;
                break;
            }
            std::swap(a[c], a[p]);
            for (size_t r = 0; r < n; ++r) {
                if (r != c && a[r][c] != 0) {
                    auto f = a[r][c] / a[c][c];
                    for (size_t j = c; j <= n; ++j) {
                        a[r][j] -= f * a[c][j];
                    }
                }
            }
        }
        if (regular) {
            std::vector<Rational> x;
            x.reserve(n);
            for (size_t i = 0; i < n; ++i) {
                x.emplace_back(a[i][n] / a[i][i]);
            }
            if (feasible(x)) {
                auto v = dot(obj, x);
                if (!best.has_value() || v > *best) {
                    best = std::move(v);
                }
            }
        }
        // advance to the next n-subset of the constraints
        auto i = n;
        while (i > 0 && sel[i - 1] == cons.size() - n + i - 1) {
            --i;
        }
        if (i == 0) {
            return best;
        }
        ++sel[i - 1];
        for (auto j = i; j < n; ++j) {
            sel[j] = sel[j - 1] + 1;
        }
    }
}

//! Compute the optimum over all Boolean assignments using a reference implementation.
auto lp_optimum(LP const &lp) -> std::optional<std::pair<Rational, bool>> {
    std::optional<std::pair<Rational, bool>> res;
    for (uint64_t mask = 0; mask < (uint64_t{1} << static_cast<uint64_t>(lp.atoms)); ++mask) {
        std::vector<LPConstraint> cons;
        std::ranges::copy_if(lp.cons, std::back_inserter(cons), [&](LPConstraint const &c) -> bool {
            return c.guard < 0 || ((mask >> static_cast<uint64_t>(c.guard)) & 1U) != 0;
        });
        // vertices of the polyhedron have small coordinates; if the optimum
        // grows with the box, the problem is unbounded
        auto small = lp_max(cons, lp.obj, 10000);
        if (!small.has_value()) {
            continue;
        }
        auto bounded = small == lp_max(cons, lp.obj, 100000);
        if (!res.has_value() || (res->second && (!bounded || *small > res->first))) {
            res = std::make_pair(std::move(*small), bounded);
        }
    }
    return res;
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

// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
TEST_CASE("optimize-random") {
    // Compare against a reference implementation on random problems. Problems
    // with atoms are solved with a global objective, which has to find the
    // optimum over all Boolean assignments.
    auto rng = Random{42};
    for (int it = 0; it < 300; ++it) {
        auto lp = random_lp(rng, it % 3 == 0 ? rng(1, 2) : 0);
        auto prg = to_program(lp);
        INFO(prg);
        auto global = lp.atoms > 0;
        auto exp = lp_optimum(lp);
        auto res = run_o(prg.c_str(), global);
        auto res_q = run_o<RationalQ>(prg.c_str(), global);
        REQUIRE(res.has_value() == exp.has_value());
        REQUIRE(res_q.has_value() == exp.has_value());
        if (exp.has_value()) {
            REQUIRE(res->second == exp->second);
            REQUIRE(res_q->second == exp->second);
            if (exp->second) {
                REQUIRE(res->first == exp->first);
                REQUIRE(res_q->first == RationalQ{exp->first});
            }
        }
    }
}

} // namespace ClingoLPX::Test
