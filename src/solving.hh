#pragma once

#include <problem.hh>

#include <util.hh>

struct Statistics {
    void reset();

    size_t pivots_{0};
};

//! A solver for finding an assignment satisfying a set of inequalities.
class Solver {
private:
    struct Bounds {
        std::optional<Number> lower;
        std::optional<Number> upper;
    };
    enum class State {
        Satisfiable = 0,
        Unsatisfiable = 1,
        Unknown = 2
    };

public:
    Solver(std::vector<Equation> &&equations);

    //! Prepare equations for solving.
    void prepare();

    //! Solve the (previously prepared) problem.
    std::optional<std::vector<std::pair<Clingo::Symbol, Number>>> solve();

    Statistics const &statistics() const;

private:
    //! Return the variables occuring in the equations.
    std::vector<Clingo::Symbol> vars_();

    //! Check if the state invariants hold.
    bool check_();

    //! Pivots basic variable `x_i` and non-basic variable `x_j`.
    void pivot_(index_t i, index_t j, Number const &v);

    // Select pivot point using Bland's rule.
    State select_(index_t &ret_i, index_t &ret_j, Number &ret_v);

    std::vector<Equation> equations_;
    Tableau tableau_;
    std::vector<Bounds> bounds_;
    std::vector<Number> assignment_;
    std::vector<index_t> variables_;
    Statistics statistics_;
    index_t n_non_basic_{0};
    index_t n_basic_{0};
};
