#pragma once

#include <problem.hh>

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
    &sum/0 : sum_term, {<=,=,>=,<,>}, sum_term, head;
    &dom/0 : dom_term, {=}, sum_term, head
}.
)";

[[nodiscard]] std::vector<Inequality> evaluate_theory(Clingo::TheoryAtoms const &theory);
