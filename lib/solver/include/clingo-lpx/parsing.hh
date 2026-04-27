#pragma once

#include <clingo-lpx/problem.hh>

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
    &sum/0      : sum_term, {<=,=,>=,<,>}, sum_term, head;
    &diff/0     : sum_term, {<=,=,>=,<,>}, sum_term, head;
    &dom/0      : dom_term, {=}, dom_term, head
}.
)";

using VarMap = std::map<std::pair<Clingo::Symbol, Clingo::ProgramLiteral>, Clingo::Symbol>;
using LitMapper = std::function<Clingo::ProgramLiteral(std::optional<Clingo::ProgramLiteral>)>;

void evaluate_theory(Clingo::Library const &lib, Clingo::TheoryBase const &theory, LitMapper const &mapper,
                     VarMap &var_map, std::vector<Inequality> &iqs, std::vector<Term> &objective);

} // namespace ClingoLPX
