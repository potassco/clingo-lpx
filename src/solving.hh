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
    //! The bound type.
    enum class BoundType : uint32_t {
        Lower = 0,
        Upper = 1,
        //Equal = 2,
    };
    //! The bounds associated with a Variable.
    //!
    //! In practice, there should be a lot of variables with just one bound.
    struct Bound {
        Number value;
        index_t variable;
        Clingo::literal_t lit;
        Relation rel;
    };
    //! Capture the current state of a variable.
    struct Variable {
        //! Adjusts the lower bound of the variable with the value of the given bound.
        [[nodiscard]] bool update_lower(Solver &s, Clingo::Assignment ass, Bound const &bound);
        //! Adjusts the upper bound of the variable with the value of the given bound.
        [[nodiscard]] bool update_upper(Solver &s, Clingo::Assignment ass, Bound const &bound);
        //! Adjusts the bounds of the variable w.r.t. to the relation of the bound.
        [[nodiscard]] bool update(Solver &s, Clingo::Assignment ass, Bound const &bound);
        //! Check if te value of the variable conflicts with the bounds;
        [[nodiscard]] bool has_conflict() const;
        //! Check if the variable has a lower bound.
        [[nodiscard]] bool has_lower() const { return lower_bound != nullptr; }
        //! Check if the variable has an upper bound.
        [[nodiscard]] bool has_upper() const { return upper_bound != nullptr; }
        //! Return the value of the lower bound.
        [[nodiscard]] Number const &lower() const { return lower_bound->value; }
        //! Return thevalue of the upper bound.
        [[nodiscard]] Number const &upper() const { return upper_bound->value; }

        //! The lower bound of a variable.
        Bound const *lower_bound{nullptr};
        //! The upper bound of a variable.
        Bound const *upper_bound{nullptr};
        //! The value of the variable.
        Number value{0};
        //! Helper index for pivoting variables.
        index_t index{0};
        //! Helper index to obtain row/column index of a variable.
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
    //! Prepare inequalities for solving.
    [[nodiscard]] bool prepare(Clingo::PropagateInit &init, std::vector<Inequality> &&inequalities);

    //! Solve the (previously prepared) problem.
    [[nodiscard]] bool solve(Clingo::PropagateControl &ctl, Clingo::LiteralSpan lits);

    //! Undo assignments on the current level.
    void undo();

    //! Get the current assignment.
    std::vector<std::pair<Clingo::Symbol, Number>> assignment() const;

    //! Return the solve statistics.
    [[nodiscard]] Statistics const &statistics() const;

    //! Return the conflict clause.
    [[nodiscard]] Clingo::LiteralSpan reason() const { return conflict_clause_; }

private:
    //! Return the variables occuring in the inequalities.
    std::vector<Clingo::Symbol> vars_() const;

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

    //! Helper function to select pivot point.
    [[nodiscard]] bool select_(bool upper, Variable &x);
    //! Select pivot point using Bland's rule.
    State select_(index_t &ret_i, index_t &ret_j, Number const *&ret_v);

    //! Get basic variable associated with row `i`.
    Variable &basic_(index_t i);
    //! Get non-basic variable associated with column `j`.
    Variable &non_basic_(index_t j);

    //! The set of inequalities.
    std::vector<Inequality> inequalities_;
    //! Mapping from literals to bounds.
    std::unordered_multimap<Clingo::literal_t, Bound> bounds_;
    //! Trail of bound assignments.
    std::vector<std::tuple<index_t, Relation, Bound const *>> trail_;
    //! Trail offsets per level.
    std::vector<std::pair<index_t, index_t>> trail_offset_;
    //! Mapping from symbols to their indices in the assignment.
    std::unordered_map<Clingo::Symbol, index_t> indices_;
    //! The tableau of coefficients.
    Tableau tableau_;
    //! The non-basic and basic variables.
    std::vector<Variable> variables_;
    //! The set of conflicting variables.
    std::priority_queue<index_t, std::vector<index_t>, std::greater<>> conflicts_;
    //! The conflict clause.
    std::vector<Clingo::literal_t> conflict_clause_;
    //! Problem and solving statistics.
    Statistics statistics_;
    //! The number of non-basic variables.
    index_t n_non_basic_{0};
    //! The number of basic variables.
    index_t n_basic_{0};
};
