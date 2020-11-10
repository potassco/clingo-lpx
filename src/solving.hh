#pragma once

#include <problem.hh>
#include <util.hh>

#include <queue>

struct Statistics {
    void reset();

    size_t pivots_{0};
};

//! A solver for finding an assignment satisfying a set of inequalities.
class Solver {
private:
    //! Helper class to prepare the inequalities for solving.
    struct Prepare;
    //! Capture the current state of a variable.
    struct Variable {
        //! Adjusts the lower bound and returns if the bounds are consistent.
        [[nodiscard]] bool update_lower(Number const &v);
        //! Adjusts the upper bound and returns if the bounds are consistent.
        [[nodiscard]] bool update_upper(Number const &v);
        //! Adjusts the bounds based on the relation and returns if the bounds are consistent.
        [[nodiscard]] bool update(Relation rel, Number const &v);
        //! Check if te value of the variable conflicts with the bounds;
        [[nodiscard]] bool has_conflict();

        //! The lower bound of a variable.
        std::optional<Number> lower;
        //! The upper bound of a variable.
        std::optional<Number> upper;
        //! The value of the variable.
        Number value{0};
        //! Helper index for pivoting variables.
        index_t index{0};
        index_t reserve_index{0};
        //! Whether this variales is in the queue of conflicting variables.
        bool queued{false};
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
    //! Check if basic variables with unsatisfied bounds are enqueued.
    [[nodiscard]] bool check_basic_();
    //! Check if bounds of non-basic variables are satisfied.
    [[nodiscard]] bool check_non_basic_();
    //! Check if the current assignment is a solution.
    [[nodiscard]] bool check_solution_();

    //! Enqueue basic variable `x_i` if it is conflicting.
    void enqueue_(index_t i);

    //! Set the value of non-basic `x_j` variable to `v`.
    void update_(index_t j, Number v);

    //! Pivots basic variable `x_i` and non-basic variable `x_j`.
    void pivot_(index_t i, index_t j, Number const &v);

    //! Select pivot point using Bland's rule.
    State select_(index_t &ret_i, index_t &ret_j, Number &ret_v);

    //! Get basic variable associated with row `i`.
    Variable &basic_(index_t i);
    //! Get non-basic variable associated with column `j`.
    Variable &non_basic_(index_t j);

    //! The set of inequalities.
    std::vector<Inequality> inequalities_;
    //! Mapping from symbols to their indices in the assignment.
    std::unordered_map<Clingo::Symbol, index_t> indices_;
    //! The tableau of coefficients.
    Tableau tableau_;
    //! The non-basic and basic variables.
    std::vector<Variable> variables_;
    //! The set of conflicting variables.
    std::priority_queue<index_t, std::vector<index_t>, std::greater<index_t>> conflicts_;
    //! Problem and solving statistics.
    Statistics statistics_;
    //! The number of non-basic variables.
    index_t n_non_basic_{0};
    //! The number of basic variables.
    index_t n_basic_{0};
};
