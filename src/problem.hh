#pragma once

#include <gmpxx.h>
#include <clingo.hh>

using Number = mpq_class;

enum class Operator {
    LessEqual = 0,
    GreaterEqual = 1,
    Equal = 2,
};

std::ostream &operator<<(std::ostream &out, Operator const &op);

struct Term {
    Number co;
    Clingo::Symbol var;
};

std::ostream &operator<<(std::ostream &out, Term const &term);

struct Equation {
    std::vector<Term> lhs;
    Number rhs;
    Operator op;
};

std::ostream &operator<<(std::ostream &out, Equation const &eq);
