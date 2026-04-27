#include <clingo-lpx/parsing.hh>

#include <clingo/control.hh>
#include <clingo/core.hh>

#include <catch2/catch_test_macros.hpp>

#include <sstream>

namespace ClingoLPX::Test {

namespace {

template <typename T> auto str(T &&x) -> std::string {
    std::ostringstream oss;
    oss << x;
    return oss.str();
}

struct Fixture {
    Fixture() { ctl.parse_string(THEORY); }
    auto evaluate(std::string_view prg) {
        ctl.parse_string(prg);
        ctl.ground();
        evaluate_theory(lib, ctl.base().theory(), Fixture::mapper, vars, eqs, objective);
    }
    static auto mapper(std::optional<Clingo::ProgramLiteral> lit) -> Clingo::ProgramLiteral {
        static_cast<void>(lit);
        return 1;
    };

    Clingo::Library lib;
    Clingo::Control ctl{lib};
    VarMap vars;
    std::vector<Inequality> eqs;
    std::vector<Term> objective;
};

} // namespace

// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
TEST_CASE_METHOD(Fixture, "parsing simple") {
    evaluate("&sum { x2; x3 } >= 10.\n");
    REQUIRE(eqs.size() == 1);
    REQUIRE(objective.empty());
    REQUIRE(str(eqs.front()) == "x2 + x3 >= 10");
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
    std::sort(objective.begin(), objective.end(),
              [](auto &a, auto &b) { return std::make_pair(a.var, a.co) < std::make_pair(b.var, b.co); });
    REQUIRE(str(Inequality{objective, 0, Relation::Equal, 0}) == "-3*x + -y = 0");
}

TEST_CASE_METHOD(Fixture, "parsing complex") {
    evaluate("&sum { 2*(x+3*y)/7; 10 } = x*3-z.\n");
    REQUIRE(eqs.size() == 1);
    REQUIRE(objective.empty());
    REQUIRE(str(eqs.front()) == "-19/7*x + 6/7*y + z = -10");
}

TEST_CASE_METHOD(Fixture, "parsing string rational") {
    evaluate(R"(&sum { x } = "123.0".)");
    REQUIRE(eqs.size() == 1);
    REQUIRE(objective.empty());
    REQUIRE(str(eqs.front()) == "x = 123");
}

} // namespace ClingoLPX::Test
