#pragma once

#include "problem.hh"

#include <map>

constexpr char const *THEORY = R"(
#theory lp {
    sum_term {
    -  : 3, unary;
    *  : 1, binary, left;
    /  : 1, binary, left
    };
    dom_term {
    .. : 0, binary, left;
    *  : 1, binary, left;
    /  : 1, binary, left;
    -  : 3, unary
    };
    &minimize/0 : sum_term, directive;
    &maximize/0 : sum_term, directive;
    &sum/0 : sum_term, {<=,=,>=}, sum_term, head;
    &dom/0 : dom_term, {=}, sum_term, head
}.
)";

constexpr char const *THEORY_Q = R"(
#theory lp {
    sum_term {
    -  : 3, unary;
    *  : 1, binary, left;
    /  : 1, binary, left
    };
    dom_term {
    .. : 0, binary, left;
    *  : 1, binary, left;
    /  : 1, binary, left;
    -  : 3, unary
    };
    &minimize/0 : sum_term, directive;
    &maximize/0 : sum_term, directive;
    &sum/0 : sum_term, {<=,=,>=,<,>}, sum_term, head;
    &dom/0 : dom_term, {=}, sum_term, head
}.
)";

using VarMap = std::map<std::pair<Clingo::Symbol, Clingo::literal_t>, Clingo::Symbol>;
using LitMapper = std::function<Clingo::literal_t(Clingo::literal_t)>;

void evaluate_theory(Clingo::TheoryAtoms const &theory, LitMapper const &mapper, VarMap &var_map,
                     std::vector<Inequality> &iqs, std::vector<Term> &objective);
