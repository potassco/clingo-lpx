#pragma once

//! @file
//! Building blocks for integer variables that do not depend on the solver.

#include <clingo-lpx/number.hh>
#include <clingo-lpx/tableau.hh>

#include <clingo/core.hh>

#include <optional>
#include <utility>
#include <vector>

namespace ClingoLPX {

//! An implication a -> b between two literals.
using Implication = std::pair<Clingo::SolverLiteral, Clingo::SolverLiteral>;

//! The order literals of an integer variable x.
//!
//! An order literal l satisfies l -> x <= k and -l -> x >= k + 1. Like in an
//! order encoding, the order literals are connected with each other and with
//! the literals of the bounds in the program by implications.
class OrderLiterals {
  public:
    //! Literals associated with values sorted by value.
    using Lits = std::vector<std::pair<Integer, Clingo::SolverLiteral>>;

    //! Check if the bounds of the program have been set.
    [[nodiscard]] auto initialized() const -> bool { return init_; }

    //! Set the bounds of the program.
    //!
    //! A literal p in uppers satisfies p -> x <= k and a literal q in lowers
    //! satisfies q -> x >= k. The vectors do not have to be sorted.
    void initialize(Lits uppers, Lits lowers);

    //! Find the order literal for value k.
    [[nodiscard]] auto find(Integer const &k) const -> std::optional<Clingo::SolverLiteral>;

    //! Insert order literal lit for value k, which must not exist yet.
    //!
    //! If link is set, implications connecting the literal to the existing
    //! literals are appended to implications:
    //!
    //! - order literals l' and l'' with the nearest values k' < k < k'' via
    //!   l' -> lit and lit -> l'',
    //! - literals p of the program with p -> x <= a via p -> lit if k' < a <=
    //!   k, and
    //! - literals q of the program with q -> x >= b via q -> -lit if k < b <=
    //!   k''.
    //!
    //! Here a missing k' or k'' imposes no restriction. Together with the
    //! implications of earlier insertions, this captures all implications
    //! between these literals.
    void insert(Integer const &k, Clingo::SolverLiteral lit, bool link, std::vector<Implication> &implications);

  private:
    Lits branches_;
    Lits uppers_;
    Lits lowers_;
    bool init_{false};
};

//! A term g*t of a cut sum_j g_j t_j >= 1 with t >= 0.
//!
//! The variable t corresponds to x - v if x is at its lower bound v and to v
//! - x if x is at its upper bound v.
struct CutTerm {
    //! The column of the non-basic variable x.
    index_t col;
    //! The coefficient of the term.
    Rational g;
    //! The value of the bound of x.
    Rational v;
    //! The maximum value of t if x has an opposite bound.
    std::optional<Rational> range;
    //! The literal of the bound.
    Clingo::SolverLiteral lit;
    //! The literal of the opposite bound (if any).
    Clingo::SolverLiteral other;
    //! Whether x is at its lower bound.
    bool lower;
};

//! Compute the coefficient g of a Gomory mixed integer cut.
//!
//! Given the coefficient c of term t in row y + sum_j c_j t_j = b and the
//! fractional part f_0 of b, this computes
//!
//! - f/f_0 if t is integer and f <= f_0,
//! - (1-f)/(1-f_0) if t is integer and f > f_0,
//! - c/f_0 if t is continuous and c >= 0, and
//! - -c/(1-f_0) if t is continuous and c < 0
//!
//! where f is the fractional part of c.
[[nodiscard]] auto gomory_coefficient(Rational const &c, Rational const &f_0, bool integer) -> Rational;

//! Round up the given coefficient to a multiple of 1/den if its denominator is larger than den.
void round_up(Rational &g, fixed_int den);

//! A cut sum_j a_j x_j >= rhs over the non-basic variables.
struct Cut {
    //! The coefficients a_j sorted by column.
    std::vector<std::pair<index_t, Rational>> row;
    //! The right-hand side.
    Rational rhs;
    //! The negated literals of the bounds the cut depends on.
    std::vector<Clingo::SolverLiteral> clause;
};

//! Weaken the cut sum_j g_j t_j >= 1 and substitute back the variables x.
//!
//! The terms of variables bounded on both sides are dropped starting with the
//! smallest g_j r_j for range r_j as long as the right-hand side stays at
//! least margin. Returns false if the resulting coefficients do not fit into
//! machine integers. The terms are reordered.
[[nodiscard]] auto finish_cut(std::vector<CutTerm> &terms, Rational const &margin, Cut &cut) -> bool;

} // namespace ClingoLPX
