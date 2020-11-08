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
    //! Helper class to prepare the inequalities for solving.
    struct Prepare;
    //! The upper and lower bounds of variables.
    struct Bounds {
        //! Adjusts the lower bound and returns if the bounds are consistent.
        [[nodiscard]] bool update_lower(Number const &v);
        //! Adjusts the upper bound and returns if the bounds are consistent.
        [[nodiscard]] bool update_upper(Number const &v);
        //! Adjusts the bounds based on the relation and returns if the bounds are consistent.
        [[nodiscard]] bool update(Relation rel, Number const &v);

        //! The lower bound of a variable.
        std::optional<Number> lower;
        //! The upper bound of a variable.
        std::optional<Number> upper;
    };
    //! Captures what is know about of the satisfiability of a problem while
    //! solving.
    enum class State {
        Satisfiable = 0,
        Unsatisfiable = 1,
        Unknown = 2
    };

public:
    //! Initializes the solver with the given set of inequalities.
    Solver(std::vector<Inequality> &&inequalities);

    //! Prepare inequalities for solving.
    [[nodiscard]] bool prepare();

    //! Solve the (previously prepared) problem.
    [[nodiscard]] std::optional<std::vector<std::pair<Clingo::Symbol, Number>>> solve();

    [[nodiscard]] Statistics const &statistics() const;

private:
    //! Return the variables occuring in the inequalities.
    std::vector<Clingo::Symbol> vars_();

    //! Check if the tableau.
    [[nodiscard]] bool check_tableau_();
    //! Check if bounds of basic variables are satisfied.
    [[nodiscard]] bool check_non_basic_();

    //! Set the value of non-basic `x_j` variable to `v`.
    void update_(index_t j, Number v);

    //! Pivots basic variable `x_i` and non-basic variable `x_j`.
    void pivot_(index_t i, index_t j, Number const &v);

    //! Select pivot point using Bland's rule.
    State select_(index_t &ret_i, index_t &ret_j, Number &ret_v);

    //! The set of inequalities.
    std::vector<Inequality> inequalities_;
    //! Mapping from symbols to their indices in the assignment.
    std::unordered_map<Clingo::Symbol, index_t> indices_;
    //! The tableau of coefficients.
    Tableau tableau_;
    //! The bounds of both basic and non-basic variables.
    std::vector<Bounds> bounds_;
    //! The assignment of both basic and non-basic variables.
    std::vector<Number> assignment_;
    //! The partition of non-basic and basic variables.
    std::vector<index_t> variables_;
    //! Problem and solving statistics.
    Statistics statistics_;
    //! The number of non-basic variables.
    index_t n_non_basic_{0};
    //! The number of basic variables.
    index_t n_basic_{0};
};
