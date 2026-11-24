#include <clingo-lpx/integer.hh>
#include <clingo-lpx/parsing.hh>
#include <clingo-lpx/problem.hh>
#include <clingo-lpx/solving.hh>

#include "helpers.hh"

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <array>
#include <clingo/control.hh>
#include <clingo/core.hh>
#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

namespace ClingoLPX::Test {

namespace {

//! A small random mixed integer program with a Boolean layer.
//!
//! All variables are bounded so that branch and bound terminates.
struct MIP {
    LP lp;
    std::vector<bool> integer;
    std::vector<std::pair<fixed_int, fixed_int>> box;
    //! Denominator to divide constraints by to obtain rational coefficients.
    std::vector<fixed_int> den;
};

//! Generate a random mixed integer program.
//!
//! With equalities set, all variables are integer and most constraints are
//! equalities with coefficients sharing common factors, which exercises the
//! GCD test.
auto random_mip(Random &rng, bool equalities = false) -> MIP {
    auto n = static_cast<size_t>(rng(1, 4));
    auto atoms = rng(0, 2);
    auto mip = MIP{LP{{}, std::vector<fixed_int>(n, 0), atoms}, {}, {}, {}};
    auto nonzero = [&]() -> fixed_int {
        if (equalities) {
            auto a = rng(1, 3) * rng(1, 3);
            return rng(0, 1) == 0 ? -a : a;
        }
        return rng(0, 1) == 0 ? rng(-5, -1) : rng(1, 5);
    };
    // keep the number of continuous variables small for the reference implementation
    size_t continuous = 0;
    for (size_t i = 0; i < n; ++i) {
        auto integer = equalities || continuous >= 1 || rng(0, 3) != 0;
        continuous += integer ? 0 : 1;
        mip.integer.emplace_back(integer);
        auto lo = rng(-5, 0);
        auto hi = rng(0, 5);
        mip.box.emplace_back(lo, hi);
    }
    for (auto m = rng(1, 6); m > 0; --m) {
        auto r = rng(0, 4);
        auto rel = r < 2 ? Relation::LessEqual : Relation::GreaterEqual;
        if (r == 4 || (equalities && r >= 1)) {
            rel = Relation::Equal;
        }
        auto rhs = rng(-12, 12);
        auto guard = atoms > 0 && rng(0, 2) == 0 ? rng(0, atoms - 1) : -1;
        auto &c = mip.lp.cons.emplace_back(std::vector<fixed_int>(n, 0), rel, rhs, guard);
        for (auto &x : c.co) {
            x = rng(0, 3) == 0 ? 0 : nonzero();
        }
        c.co[static_cast<size_t>(rng(0, static_cast<fixed_int>(n) - 1))] = nonzero();
        mip.den.emplace_back(rng(1, 3));
    }
    for (auto &x : mip.lp.obj) {
        x = rng(-3, 3);
    }
    mip.lp.obj[static_cast<size_t>(rng(0, static_cast<fixed_int>(n) - 1))] = nonzero();
    return mip;
}

auto to_program(MIP const &mip) -> std::string {
    auto out = std::ostringstream{};
    auto n = mip.integer.size();
    for (fixed_int i = 0; i < mip.lp.atoms; ++i) {
        out << "{ a" << i << " }.\n";
    }
    for (size_t i = 0; i < n; ++i) {
        if (mip.integer[i]) {
            out << "&int { x" << i << " }.\n";
        }
        out << "&sum { x" << i << " } >= " << mip.box[i].first << ".\n";
        out << "&sum { x" << i << " } <= " << mip.box[i].second << ".\n";
    }
    for (size_t k = 0; k < mip.lp.cons.size(); ++k) {
        auto const &c = mip.lp.cons[k];
        auto d = mip.den[k];
        out << "&sum { ";
        char const *sep = "";
        for (size_t i = 0; i < n; ++i) {
            if (c.co[i] != 0) {
                out << sep << c.co[i] << "*x" << i << "/" << d;
                sep = "; ";
            }
        }
        out << " } " << c.rel << " " << c.rhs << "/" << d;
        if (c.guard >= 0) {
            out << " :- a" << c.guard;
        }
        out << ".\n";
    }
    out << "&maximize { ";
    char const *sep = "";
    for (size_t i = 0; i < n; ++i) {
        if (mip.lp.obj[i] != 0) {
            out << sep << mip.lp.obj[i] << "*x" << i;
            sep = "; ";
        }
    }
    out << " }.\n";
    return out.str();
}

//! Compute the optimum by enumerating Boolean assignments and integer points.
auto mip_optimum(MIP const &mip) -> std::optional<Rational> {
    auto n = mip.integer.size();
    std::optional<Rational> res;
    for (uint64_t mask = 0; mask < (uint64_t{1} << static_cast<uint64_t>(mip.lp.atoms)); ++mask) {
        std::vector<LPConstraint> cons;
        std::ranges::copy_if(mip.lp.cons, std::back_inserter(cons), [&](LPConstraint const &c) -> bool {
            return c.guard < 0 || ((mask >> static_cast<uint64_t>(c.guard)) & 1U) != 0;
        });
        // enumerate the values of the integer variables
        std::vector<fixed_int> point;
        point.reserve(n);
        for (size_t i = 0; i < n; ++i) {
            point.emplace_back(mip.integer[i] ? mip.box[i].first : 0);
        }
        while (true) {
            // the generator produces at most one continuous variable y with
            // bounds lo <= y <= hi
            auto c = static_cast<size_t>(std::ranges::find(mip.integer, false) - mip.integer.begin());
            std::optional<Rational> lo;
            std::optional<Rational> hi;
            if (c < n) {
                lo = Rational{mip.box[c].first};
                hi = Rational{mip.box[c].second};
            }
            auto sat = std::ranges::all_of(cons, [&](LPConstraint const &con) -> bool {
                fixed_int s = 0;
                for (size_t i = 0; i < n; ++i) {
                    if (i != c) {
                        s += con.co[i] * point[i];
                    }
                }
                auto a = c < n ? con.co[c] : 0;
                if (a == 0) {
                    switch (con.rel) {
                        case Relation::LessEqual: {
                            return s <= con.rhs;
                        }
                        case Relation::GreaterEqual: {
                            return s >= con.rhs;
                        }
                        default: {
                            return s == con.rhs;
                        }
                    }
                }
                // a*y rel rhs - s
                auto v = Rational{con.rhs - s} / Rational{a};
                bool upper = con.rel == Relation::Equal || ((con.rel == Relation::LessEqual) == (a > 0));
                bool lower = con.rel == Relation::Equal || !upper;
                if (upper && v < *hi) {
                    hi = v;
                }
                if (lower && v > *lo) {
                    lo = v;
                }
                return true;
            });
            std::optional<Rational> val;
            if (sat && (c == n || *lo <= *hi)) {
                fixed_int s = 0;
                for (size_t i = 0; i < n; ++i) {
                    if (i != c) {
                        s += mip.lp.obj[i] * point[i];
                    }
                }
                val = Rational{s};
                if (c < n) {
                    *val += Rational{mip.lp.obj[c]} * (mip.lp.obj[c] >= 0 ? *hi : *lo);
                }
            }
            if (val.has_value() && (!res.has_value() || *val > *res)) {
                res = std::move(val);
            }
            size_t i = 0;
            for (; i < n; ++i) {
                if (mip.integer[i] && point[i] < mip.box[i].second) {
                    ++point[i];
                    break;
                }
                if (mip.integer[i]) {
                    point[i] = mip.box[i].first;
                }
            }
            if (i == n) {
                break;
            }
        }
    }
    return res;
}

//! Solver options to rotate through in the random tests.
constexpr auto cut_periods = std::array<size_t, 4>{0, 1, 2, 4};
constexpr auto cut_limits = std::array<size_t, 3>{64, 1, 3};
constexpr auto stores = std::array{
    StoreSATAssignments::No,
    StoreSATAssignments::Partial,
    StoreSATAssignments::Total,
};
constexpr auto propagate_modes = std::array{
    PropagateMode::None,
    PropagateMode::Changed,
    PropagateMode::Full,
};

//! Record the last objective value and check that integer variables have integer values.
template <typename V> class MIPHandler : public Clingo::SolveEventHandler {
  public:
    MIPHandler(Propagator<V> &prp, std::string const &prg) : prp_{prp}, prg_{prg} {}

    [[nodiscard]] auto objective() const -> std::optional<std::pair<V, bool>> const & { return val_; }
    [[nodiscard]] auto fractional() const -> std::string const & { return fractional_; }

  private:
    auto do_model(Clingo::Model model) -> bool override {
        prp_.on_model(model);
        for (index_t i = 0, e = prp_.n_values(model.thread_id()); i < e; ++i) {
            auto value = prp_.get_value(model.thread_id(), i);
            auto name = std::ostringstream{};
            name << prp_.get_symbol(i);
            if (prg_.find("&int { " + name.str() + " }") != std::string::npos && !is_integer(value)) {
                auto out = std::ostringstream{};
                out << name.str() << "=" << value;
                fractional_ = out.str();
            }
        }
        val_ = prp_.get_objective(model.thread_id());
        return true;
    }

    Propagator<V> &prp_;
    std::string const &prg_;
    std::optional<std::pair<V, bool>> val_;
    std::string fractional_;
};

//! Solve a mixed integer program checking that integer variables are assigned integer values.
template <typename V>
auto run_mip(std::string const &prg, Options const &opts, size_t threads) -> std::optional<std::pair<V, bool>> {
    auto lib = Clingo::Library{};
    auto ctl = Clingo::Control{lib};
    register_theory(ctl);
    auto &prp = ctl.register_propagator(std::make_unique<Propagator<V>>(lib, opts));
    ctl.config()["solve"]["models"] = "0";
    ctl.config()["solve"]["parallel_mode"] = std::to_string(threads);
    ctl.parse_string(prg);
    ctl.ground();
    auto handler = MIPHandler<V>{prp, prg};
    if (!ctl.solve({}, std::ref(handler)).satisfiable()) {
        return std::nullopt;
    }
    INFO("fractional value " << handler.fractional());
    REQUIRE(handler.fractional().empty());
    return handler.objective();
}

} // namespace

// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
TEST_CASE("order-literals") {
    // p3 -> x <= 3, p5 -> x <= 5, q2 -> x >= 2, q6 -> x >= 6
    Clingo::SolverLiteral const p3 = 1;
    Clingo::SolverLiteral const p5 = 2;
    Clingo::SolverLiteral const q2 = 3;
    Clingo::SolverLiteral const q6 = 4;
    auto order = OrderLiterals{};
    REQUIRE(!order.initialized());
    order.initialize({{Integer{5}, p5}, {Integer{3}, p3}}, {{Integer{6}, q6}, {Integer{2}, q2}});
    REQUIRE(order.initialized());
    std::vector<Implication> imps;
    // the first literal is connected to all bounds of the program
    order.insert(Integer{4}, 10, true, imps);
    REQUIRE(imps == std::vector<Implication>{{p3, 10}, {q6, -10}});
    // lower values only take over bounds in between
    imps.clear();
    order.insert(Integer{1}, 11, true, imps);
    REQUIRE(imps == std::vector<Implication>{{q2, -11}, {11, 10}});
    imps.clear();
    order.insert(Integer{5}, 12, true, imps);
    REQUIRE(imps == std::vector<Implication>{{p5, 12}, {q6, -12}, {10, 12}});
    // a literal between two existing ones is connected to both
    imps.clear();
    order.insert(Integer{2}, 13, true, imps);
    REQUIRE(imps == std::vector<Implication>{{11, 13}, {13, 10}});
    REQUIRE(order.find(Integer{4}) == 10);
    REQUIRE(order.find(Integer{2}) == 13);
    REQUIRE(!order.find(Integer{3}).has_value());
    // without linking, no implications are generated
    imps.clear();
    order.insert(Integer{7}, 14, false, imps);
    REQUIRE(imps.empty());
    REQUIRE(order.find(Integer{7}) == 14);
}

// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
TEST_CASE("cut-helpers") {
    auto q = [](fixed_int a, fixed_int b) -> Rational { return Rational{Integer{a}, Integer{b}}; };
    SECTION("gomory") {
        REQUIRE(gomory_coefficient(q(1, 4), q(1, 2), true) == q(1, 2));
        REQUIRE(gomory_coefficient(q(3, 4), q(1, 2), true) == q(1, 2));
        REQUIRE(gomory_coefficient(q(-1, 4), q(1, 2), true) == q(1, 2));
        REQUIRE(gomory_coefficient(q(1, 2), q(1, 3), true) == q(3, 4));
        REQUIRE(gomory_coefficient(Rational{2}, q(1, 3), true) == 0);
        REQUIRE(gomory_coefficient(Rational{1}, q(1, 2), false) == 2);
        REQUIRE(gomory_coefficient(Rational{-1}, q(1, 3), false) == q(3, 2));
    }
    SECTION("round") {
        auto g = q(1, 3);
        round_up(g, 2);
        REQUIRE(g == q(1, 2));
        g = q(1, 2);
        round_up(g, 2);
        REQUIRE(g == q(1, 2));
        g = q(5, 7);
        round_up(g, 1024);
        REQUIRE(g == q(5, 7));
    }
    SECTION("finish") {
        // t0 = x0 - 0 <= 1, t1 = 5 - x1 unbounded, t2 = x2 - 2 <= 1
        std::vector<CutTerm> terms;
        terms.emplace_back(0, q(1, 4), Rational{0}, Rational{1}, 1, 2, true);
        terms.emplace_back(1, q(1, 2), Rational{5}, std::nullopt, 3, 0, false);
        terms.emplace_back(2, Rational{1}, Rational{2}, Rational{1}, 4, 5, true);
        Cut cut;
        REQUIRE(finish_cut(terms, q(1, 2), cut));
        // t0 is dropped: 1/2 t1 + t2 >= 3/4 gives -1/2 x1 + x2 >= 1/4
        REQUIRE(cut.row == std::vector<std::pair<index_t, Rational>>{{1, q(-1, 2)}, {2, Rational{1}}});
        REQUIRE(cut.rhs == q(1, 4));
        REQUIRE(cut.clause == std::vector<Clingo::SolverLiteral>{-1, -2, -4, -3});
        // nothing is dropped with a margin of 1
        terms.clear();
        terms.emplace_back(0, q(1, 4), Rational{0}, Rational{1}, 1, 2, true);
        REQUIRE(finish_cut(terms, Rational{1}, cut));
        REQUIRE(cut.row == std::vector<std::pair<index_t, Rational>>{{0, q(1, 4)}});
        REQUIRE(cut.rhs == 1);
        // coefficients that do not fit into machine integers
        terms.clear();
        terms.emplace_back(0, Rational{Integer{"100000000000000000000", 10}, Integer{1}}, Rational{0}, std::nullopt, 1,
                           0, true);
        REQUIRE(!finish_cut(terms, q(1, 2), cut));
    }
}

// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
TEST_CASE("integer") {
    SECTION("rounding") {
        REQUIRE(!run("&int { x }. &sum { 2*x } = 3.\n"));
        REQUIRE(!run("&int { x }. &sum { x } > 1/2. &sum { x } < 1.\n"));
        REQUIRE(run("&int { x }. &sum { x } > 1/2. &sum { x } < 2.\n"));
        REQUIRE(!run("&int { x; y }. &sum { 2*x; 2*y } = 3.\n"));
        REQUIRE(!run("&int { x; y }. &sum { x; y } > 1. &sum { x; y } < 2.\n"));
        REQUIRE_THROWS(run("&int { x }. &sum { x; y } > 1.\n"));
    }
    SECTION("branch") {
        // the relaxation has the vertex x = y = 1/2
        REQUIRE(!run("&int { x; y }.\n"
                     "&sum { x; y } = 1.\n"
                     "&sum { x; -y } = 0.\n"));
        REQUIRE(run("&int { x }.\n"
                    "&sum { 3*x; -2*y } = 1.\n"
                    "&sum { y } >= 1/3.\n"));
        auto opts = options;
        opts.global_objective = RationalQ{Rational{1}};
        REQUIRE(run_mip<Rational>("&int { x; y }.\n"
                                  "&sum { -x; y } <= 1.\n"
                                  "&sum { 3*x; 2*y } <= 12.\n"
                                  "&sum { 2*x; 3*y } <= 12.\n"
                                  "&sum { x } >= 0.\n"
                                  "&sum { y } >= 0.\n"
                                  "&maximize { y }.\n",
                                  opts, 1) == std::make_pair(Rational{2}, true));
    }
    SECTION("conditional") {
        REQUIRE(!run("&int { x; y }.\n"
                     "a.\n"
                     "&sum { 2*x : a; 2*y : a } = 1.\n"));
    }
}

// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
TEST_CASE("integer-random") {
    // Compare against a reference implementation on random mixed integer
    // problems varying the solver options.
    auto rng = Random{7};
    for (size_t it = 0; it < 600; ++it) {
        auto mip = random_mip(rng);
        auto prg = to_program(mip);
        INFO("iteration " << it << "\n" << prg);
        auto exp = mip_optimum(mip);
        auto opts = options;
        opts.cut_period = cut_periods.at(it % cut_periods.size());
        opts.cut_limit = cut_limits.at((it / 4) % cut_limits.size());
        opts.store_sat_assignment = stores.at((it / 12) % stores.size());
        opts.propagate_mode = propagate_modes.at(it % propagate_modes.size());
        opts.order_branches = it % 7 != 0;
        opts.gcd_test = it % 11 != 0;
        opts.fix_reduced_costs = it % 13 != 0;
        opts.branch_polarity = static_cast<BranchPolarity>(it % 4);
        auto threads = it % 5 == 0 ? 3 : 1;
        INFO("cut_period=" << opts.cut_period << " cut_limit=" << opts.cut_limit << " threads=" << threads);
        opts.global_objective = RationalQ{0};
        auto res = run_mip<Rational>(prg, opts, threads);
        opts.global_objective = RationalQ{Rational{0}, Rational{1}};
        auto res_q = run_mip<RationalQ>(prg, opts, threads);
        REQUIRE(res.has_value() == exp.has_value());
        REQUIRE(res_q.has_value() == exp.has_value());
        if (exp.has_value()) {
            REQUIRE(res->second);
            REQUIRE(res_q->second);
            REQUIRE(res->first == *exp);
            REQUIRE(res_q->first == RationalQ{*exp});
        }
    }
}

// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
TEST_CASE("integer-equalities-random") {
    // Like integer-random but for integer problems dominated by equalities.
    auto rng = Random{11};
    for (size_t it = 0; it < 400; ++it) {
        auto mip = random_mip(rng, true);
        auto prg = to_program(mip);
        INFO("iteration " << it << "\n" << prg);
        auto exp = mip_optimum(mip);
        auto opts = options;
        opts.cut_period = cut_periods.at(it % cut_periods.size());
        opts.cut_limit = cut_limits.at((it / 4) % cut_limits.size());
        opts.store_sat_assignment = stores.at((it / 12) % stores.size());
        opts.propagate_mode = propagate_modes.at(it % propagate_modes.size());
        opts.order_branches = it % 7 != 0;
        opts.gcd_test = it % 11 != 0;
        opts.fix_reduced_costs = it % 13 != 0;
        opts.branch_polarity = static_cast<BranchPolarity>(it % 4);
        auto threads = it % 5 == 0 ? 3 : 1;
        INFO("cut_period=" << opts.cut_period << " cut_limit=" << opts.cut_limit << " threads=" << threads);
        opts.global_objective = RationalQ{0};
        auto res = run_mip<Rational>(prg, opts, threads);
        opts.global_objective = RationalQ{Rational{0}, Rational{1}};
        auto res_q = run_mip<RationalQ>(prg, opts, threads);
        REQUIRE(res.has_value() == exp.has_value());
        REQUIRE(res_q.has_value() == exp.has_value());
        if (exp.has_value()) {
            REQUIRE(res->second);
            REQUIRE(res_q->second);
            REQUIRE(res->first == *exp);
            REQUIRE(res_q->first == RationalQ{*exp});
        }
    }
}

} // namespace ClingoLPX::Test
