#pragma once

#include <clingo.hh>
#include "util.hh"

enum class Relation {
    LessEqual = 0,
    GreaterEqual = 1,
    Equal = 2,
    Less = 3,
    Greater = 4,
};

[[nodiscard]] Relation invert(Relation rel);

std::ostream &operator<<(std::ostream &out, Relation const &rel);

struct Term {
    Number co;
    Clingo::Symbol var;
};

std::ostream &operator<<(std::ostream &out, Term const &term);

struct Inequality {
    std::vector<Term> lhs;
    Number rhs;
    Relation rel;
    Clingo::literal_t lit;
};

std::ostream &operator<<(std::ostream &out, Inequality const &x);
