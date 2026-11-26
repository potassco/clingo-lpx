#pragma once

#include <clingo-lpx/problem.hh>

#include <clingo/ast.hh>
#include <clingo/base.hh>
#include <clingo/core.hh>

#include <map>

namespace ClingoLPX {

constexpr char const *THEORY = R"(
#theory lp {
    sum_term {
    +  : 1, binary, left;
    -  : 1, binary, left;
    *  : 2, binary, left;
    /  : 2, binary, left;
    -  : 3, unary
    };
    dom_term {
    .. : 0, binary, left;
    +  : 1, binary, left;
    -  : 1, binary, left;
    *  : 2, binary, left;
    /  : 2, binary, left;
    -  : 3, unary
    };
    &minimize/0 : sum_term, directive;
    &maximize/0 : sum_term, directive;
    &int/0      : sum_term, directive;
    &sum/0      : sum_term, {<=,=,>=,<,>,!=}, sum_term, head;
    &diff/0     : sum_term, {<=,=,>=,<,>,!=}, sum_term, head;
    &__sum_b/0  : sum_term, {<=,=,>=,<,>,!=}, sum_term, body;
    &__diff_b/0 : sum_term, {<=,=,>=,<,>,!=}, sum_term, body;
    &dom/0      : dom_term, {=}, dom_term, head
}.
)";

//! The symbol marking constant terms in linear expressions.
//!
//! Auxiliary variables introduced for conditional terms are positive numbers.
[[nodiscard]] inline auto constant_symbol() -> Clingo::Symbol { return Clingo::Number(0); }

//! Check if a symbol marks a constant term.
[[nodiscard]] inline auto is_constant(Clingo::Symbol const &sym) -> bool { return sym == constant_symbol(); }

using VarMap = std::map<std::pair<Clingo::Symbol, Clingo::ProgramLiteral>, Clingo::Symbol>;
using LitMapper = std::function<Clingo::ProgramLiteral(std::optional<Clingo::ProgramLiteral>)>;
//! Create two fresh literals of which exactly one is true if the given literal is true and none otherwise.
using LitSplitter = std::function<std::pair<Clingo::ProgramLiteral, Clingo::ProgramLiteral>(Clingo::ProgramLiteral)>;
using NodeCallback = std::function<void(Clingo::AST::Node const &ast)>;

//! Rename &sum and &diff constraints in rule bodies and pass the statement to the callback.
void rewrite(Clingo::Library const &lib, Clingo::AST::Node const &ast, NodeCallback const &cb);

//! Evaluate the theory atoms.
//!
//! Inequalities are added to iqs, the objective function to objective, and the
//! variables declared integer to integers (possibly with duplicates).
//!
//! Constraints in rule bodies are added together with their negation and
//! disequalities are split into two strict inequalities. Both are only
//! supported if split is set.
void evaluate_theory(Clingo::Library const &lib, Clingo::TheoryBase const &theory, LitMapper const &mapper,
                     LitSplitter const &split, VarMap &var_map, std::vector<Inequality> &iqs,
                     std::vector<Term> &objective, std::vector<Clingo::Symbol> &integers);

} // namespace ClingoLPX
