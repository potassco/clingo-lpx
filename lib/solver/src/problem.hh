#pragma once

#include <clingo.hh>

#include "number.hh"

enum class Relation {
    LessEqual = 0,
    GreaterEqual = 1,
    Equal = 2,
    Less = 3,
    Greater = 4,
};

[[nodiscard]] auto invert(Relation rel) -> Relation;

auto operator<<(std::ostream &out, Relation const &rel) -> std::ostream &;

struct Term {
    Rational co;
    Clingo::Symbol var;
};

auto operator<<(std::ostream &out, Term const &term) -> std::ostream &;

struct Inequality {
    std::vector<Term> lhs;
    Rational rhs;
    Relation rel;
    Clingo::literal_t lit;
};

auto operator<<(std::ostream &out, Inequality const &x) -> std::ostream &;
