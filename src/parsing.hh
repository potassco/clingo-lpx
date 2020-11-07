#pragma once

#include <problem.hh>

constexpr char const *THEORY = R"(
#theory lp {
    sum_term {
    -  : 3, unary;
    *  : 1, binary, left;
    /  : 1, binary, left
    };
    &sum/0 : sum_term, {<=,=,>=}, sum_term, any
}.
)";

[[nodiscard]] std::vector<Equation> evaluate_theory(Clingo::TheoryAtoms const &theory);
