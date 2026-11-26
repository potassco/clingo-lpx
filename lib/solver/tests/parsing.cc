#include <algorithm>
#include <clingo-lpx/parsing.hh>

#include <clingo/ast.hh>
#include <clingo/control.hh>
#include <clingo/core.hh>

#include <catch2/catch_test_macros.hpp>

#include <sstream>

namespace ClingoLPX::Test {

namespace {

template <typename T> auto str(T &&x) -> std::string {
    std::ostringstream oss;
    oss << std::forward<T>(x);
    return oss.str();
}

struct Fixture {
    Fixture() { ctl.parse_string(THEORY); }
    auto evaluate(std::string_view prg) {
        ctl.parse_string(prg);
        ctl.ground();
        evaluate_theory(lib, ctl.base().theory(), Fixture::mapper, Fixture::split, vars, eqs, objective, integers);
    }
    static auto mapper(std::optional<Clingo::ProgramLiteral> lit) -> Clingo::ProgramLiteral {
        static_cast<void>(lit);
        return 1;
    };
    static auto split(Clingo::ProgramLiteral lit) -> std::pair<Clingo::ProgramLiteral, Clingo::ProgramLiteral> {
        static_cast<void>(lit);
        return {2, 3};
    };

    Clingo::Library lib;
    Clingo::Control ctl{lib};
    VarMap vars;
    std::vector<Inequality> eqs;
    std::vector<Term> objective;
    std::vector<Clingo::Symbol> integers;
};

} // namespace

// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
TEST_CASE_METHOD(Fixture, "parsing simple") {
    evaluate("&sum { x2; x3 } >= 10.\n");
    REQUIRE(eqs.size() == 1);
    REQUIRE(objective.empty());
    REQUIRE(str(eqs.front()) == "x2 + x3 >= 10");
}

TEST_CASE_METHOD(Fixture, "parsing disequality") {
    evaluate("&sum { x } != 1.\n");
    REQUIRE(eqs.size() == 2);
    REQUIRE(str(eqs[0]) == "x < 1");
    REQUIRE(eqs[0].lit == 2);
    REQUIRE(str(eqs[1]) == "x > 1");
    REQUIRE(eqs[1].lit == 3);
}

TEST_CASE_METHOD(Fixture, "parsing sign") {
    evaluate("&sum { -x } <= 0.\n");
    REQUIRE(eqs.size() == 1);
    REQUIRE(objective.empty());
    REQUIRE(str(eqs.front()) == "-x <= 0");
}

TEST_CASE_METHOD(Fixture, "parsing rational") {
    evaluate("&sum {  -x; -2/(-3)*y } = -1.\n");
    REQUIRE(eqs.size() == 1);
    REQUIRE(objective.empty());
    REQUIRE(str(eqs.front()) == "-x + 2/3*y = -1");
}

TEST_CASE_METHOD(Fixture, "parsing objective") {
    evaluate("&minimize { 3*x }. &maximize { -y }.");
    REQUIRE(eqs.empty());
    REQUIRE(objective.size() == 2);
    std::ranges::sort(
        objective, [](auto &a, auto &b) -> auto { return std::make_pair(a.var, a.co) < std::make_pair(b.var, b.co); });
    REQUIRE(str(Inequality{objective, 0, Relation::Equal, 0}) == "-3*x + -y = 0");
}

TEST_CASE_METHOD(Fixture, "parsing complex") {
    evaluate("&sum { 2*(x+3*y)/7; 10 } = x*3-z.\n");
    REQUIRE(eqs.size() == 1);
    REQUIRE(objective.empty());
    REQUIRE(str(eqs.front()) == "-19/7*x + 6/7*y + z = -10");
}

TEST_CASE_METHOD(Fixture, "parsing integer") {
    evaluate("&int { x; y(1) }. &int { x }. &sum { x; z } >= 1.");
    REQUIRE(eqs.size() == 1);
    REQUIRE(integers.size() == 3);
    std::ranges::sort(integers);
    REQUIRE(str(integers[0]) == "x");
    REQUIRE(str(integers[1]) == "x");
    REQUIRE(str(integers[2]) == "y(1)");
    REQUIRE_THROWS(evaluate("&int { 1 }."));
}

TEST_CASE_METHOD(Fixture, "parsing string rational") {
    evaluate(R"(&sum { x } = "123.0".)");
    REQUIRE(eqs.size() == 1);
    REQUIRE(objective.empty());
    REQUIRE(str(eqs.front()) == "x = 123");
}

} // namespace ClingoLPX::Test

namespace ClingoLPX::Test {

TEST_CASE("rewrite body") {
    using namespace Clingo::AST;
    auto lib = Clingo::Library{};
    auto rewritten = [&](Node const &stm) -> std::string {
        auto res = std::string{};
        rewrite(lib, stm, [&](Node const &ast) -> void { res = ast.to_string(); });
        return res;
    };
    auto stm = parse(lib, "a :- &sum { x } >= 1; &dom { 0..1 } = y.");
    REQUIRE(rewritten(stm) == "a :- &__sum_b { x } >= 1; &dom { (0 .. 1) } = y.");
    REQUIRE(rewritten(parse(lib, "&sum { x } >= 1.")) == "&sum { x } >= 1.");

    // the name of a theory atom can also be a symbolic term
    Transformer trans = [&](Node const &ast) -> std::optional<Node> {
        if (ast.type() == NodeType::body_theory_atom) {
            return ast.update<NodeType::body_theory_atom>(lib, [&]<Attribute Attr>() -> auto {
                if constexpr (Attr == Attribute::name) {
                    auto name = ast.node(Attribute::name);
                    return Node::create<NodeType::term_symbolic>(lib, name.location(Attribute::location),
                                                                 Clingo::Function(lib, "diff", {}));
                }
            });
        }
        return ast.accept(lib, trans);
    };
    auto sym = trans(parse(lib, "a :- &sum { x } >= 1."));
    REQUIRE(sym.has_value());
    REQUIRE(rewritten(*sym) == "a :- &__diff_b { x } >= 1.");
}

} // namespace ClingoLPX::Test
