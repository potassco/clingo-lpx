#pragma once

#include "integer.hh"
#include "parsing.hh"
#include "problem.hh"
#include "tableau.hh"

#include <clingo/control.hh>
#include <clingo/core.hh>

#include <deque>
#include <optional>
#include <shared_mutex>
#include <unordered_map>

namespace ClingoLPX {

using SymbolMap = std::unordered_map<Clingo::Symbol, index_t>;
using SymbolVec = std::vector<Clingo::Symbol>;

enum class StoreSATAssignments : uint8_t {
    No = 0,
    Partial = 1,
    Total = 2,
};

enum class SelectionHeuristic : uint8_t {
    None = 0,
    Match = 1,
    Conflict = 2,
};

enum class PropagateMode : uint8_t {
    None = 0,
    Changed = 1,
    Full = 2,
};

//! Rule to select the conflicting basic variable when searching for a solution.
enum class BasicRule : uint8_t {
    Bland = 0,     //!< Select the variable with the smallest index.
    Violation = 1, //!< Select the variable violating its bound the most.
};

//! Rule to select the non-basic variable when searching for a solution.
enum class NonBasicRule : uint8_t {
    Bland = 0,  //!< Select the variable with the smallest index.
    Sparse = 1, //!< Select the variable with the fewest non-zero elements in its column.
};

//! Rule to select the entering variable when optimizing.
enum class EnteringRule : uint8_t {
    Bland = 0,   //!< Select the variable with the smallest index.
    Dantzig = 1, //!< Select the variable with the largest coefficient in the objective.
};

//! Rule to select which side of a branch x <= k or x >= k + 1 is tried first.
enum class BranchPolarity : uint8_t {
    Nearest = 0,  //!< Try the side closer to the current value.
    Farthest = 1, //!< Try the side farther from the current value.
    Up = 2,       //!< Try x >= k + 1 first.
    Down = 3,     //!< Try x <= k first.
};

//! Rule to select the sign when deciding a branch literal.
enum class BranchPhase : uint8_t {
    Saved = 0, //!< Keep the sign chosen by the solver.
    Value = 1, //!< Select the side containing the current value or the nearer side.
};

static constexpr auto default_bland_threshold = 128;
static constexpr auto default_cut_period = 0;
static constexpr auto default_cut_limit = 64;

struct Options {
    SelectionHeuristic select = SelectionHeuristic::None;
    StoreSATAssignments store_sat_assignment = StoreSATAssignments::No;
    std::optional<RationalQ> global_objective = std::nullopt;
    PropagateMode propagate_mode = PropagateMode::Changed;
    bool propagate_conflicts = false;
    BasicRule basic_rule = BasicRule::Bland;
    NonBasicRule non_basic_rule = NonBasicRule::Sparse;
    EnteringRule entering_rule = EnteringRule::Dantzig;
    //! Number of pivots without progress after which Bland's rule is used.
    //!
    //! The actual threshold is the maximum of this value and the number of rows.
    size_t bland_threshold = default_bland_threshold;
    //! Try a Gomory cut instead of branching every n-th time an integer
    //! variable has a fractional value (0 disables cuts).
    size_t cut_period = default_cut_period;
    //! The maximum number of cut rows per solver thread.
    //!
    //! If the limit is reached, inactive cut rows are replaced by new ones.
    size_t cut_limit = default_cut_limit;
    //! Connect the literals of branches and bounds on the same variable like in
    //! an order encoding.
    bool order_branches = true;
    //! Which side of a branch to try first.
    BranchPolarity branch_polarity = BranchPolarity::Nearest;
    //! Check rows over integer variables for divisibility before branching.
    bool gcd_test = true;
    //! Tighten bounds of integer variables using reduced costs when optimizing globally.
    bool fix_reduced_costs = false;
    //! Whether strict constraints are supported (which selects the number type).
    bool strict = false;
    //! The sign when deciding literals introduced during solving.
    BranchPhase branch_phase = BranchPhase::Value;
};

struct Statistics {
    void reset();

    size_t pivots{0};
    size_t propagated_bounds{0};
    size_t branches{0};
    size_t cuts{0};
    size_t gcd_conflicts{0};
    size_t fixed_bounds{0};
};

//! Helper to distribute current best objective to solver threads.
template <typename Value> class ObjectiveState {
  public:
    void reset();
    void update(std::pair<Value, bool> value);
    auto value(size_t &generation) -> std::optional<std::pair<Value, bool>>;

  private:
    std::shared_mutex mutex_;
    Value value_;
    size_t generation_ = 0;
    bool bounded_{true};
};

//! A solver for finding an assignment satisfying a set of inequalities.
template <typename Value> class Solver {
  private:
    //! Helper class to prepare the inequalities for solving.
    struct Prepare;
    //! The bound type.
    enum class BoundType : uint8_t {
        Lower = 0,
        Upper = 1,
        // Equal = 2,
    };
    enum class BoundRelation : uint8_t {
        LessEqual = 0,
        GreaterEqual = 1,
        Equal = 2,
    };
    template <typename V> friend auto bound_rel(Relation rel) -> Solver<V>::BoundRelation;
    //! The bounds associated with a Variable.
    //!
    //! In practice, there should be a lot of variables with just one bound.
    struct Bound {
        Value value;
        index_t variable{0};
        Clingo::SolverLiteral lit{0};
        BoundRelation rel{BoundRelation::LessEqual};
        //! Compare the given value with the value of the bound according to
        //! the relation of the bound.
        [[nodiscard]] auto compare(Value const &value) const -> bool;
        //! Check if the bound conflicts with the other one.
        [[nodiscard]] auto conflicts(Bound const &other) const -> bool;
    };
    //! Capture the current state of a variable.
    struct Variable {
        //! Adjusts the lower bound of the variable with the value of the given bound.
        [[nodiscard]] auto update_lower(Solver &s, Clingo::Assignment ass, Bound const &bound) -> bool;
        //! Adjusts the upper bound of the variable with the value of the given bound.
        [[nodiscard]] auto update_upper(Solver &s, Clingo::Assignment ass, Bound const &bound) -> bool;
        //! Adjusts the bounds of the variable w.r.t. to the relation of the bound.
        [[nodiscard]] auto update(Solver &s, Clingo::Assignment ass, Bound const &bound) -> bool;
        //! Check if te value of the variable conflicts with the bounds;
        [[nodiscard]] auto has_conflict() const -> bool;
        //! Check if the variable has a lower bound.
        [[nodiscard]] auto has_lower() const -> bool { return lower_bound != nullptr; }
        //! Check if the variable has an upper bound.
        [[nodiscard]] auto has_upper() const -> bool { return upper_bound != nullptr; }
        //! Return the value of the lower bound.
        [[nodiscard]] auto lower() const -> Value const & { return lower_bound->value; }
        //! Return thevalue of the upper bound.
        [[nodiscard]] auto upper() const -> Value const & { return upper_bound->value; }
        //! Set a new value or add to the existing one.
        void set_value(Solver &s, index_t lvl, Value const &val, bool add);

        //! The lower bound of a variable.
        Bound const *lower_bound{nullptr};
        //! The upper bound of a variable.
        Bound const *upper_bound{nullptr};
        //! The value of the variable.
        Value value{0};
        //! Helper index for pivoting variables.
        index_t index{0};
        //! Helper index to obtain row/column index of a variable.
        index_t reverse_index{0};
        //! The level the variable was assigned on.
        index_t level{0};
        //! The bounds associated with this variable.
        std::vector<Bound const *> bounds;
        //! Whether this variales is in the queue of conflicting variables.
        bool queued{false};
        //! Whether this variales is in the queue of (non-basic) variables to porpagate.
        bool propagate{false};
        //! Whether this variable has to take integer values.
        bool integer{false};
    };
    //! A row added to the tableau during search.
    struct CutRow {
        //! The slack variable of the cut.
        index_t var{0};
        //! The literal associated with the bound of the cut.
        Clingo::SolverLiteral lit{0};
        //! The definition of the slack variable in terms of variables that are
        //! not slack variables of cuts.
        std::vector<std::pair<index_t, Rational>> def;
    };
    //! The state of the cuts (see integer.cc).
    struct Cuts {
        //! The rows added during search.
        //!
        //! The slack variable of the i-th cut has index offset + i.
        std::vector<CutRow> rows;
        //! The index of the first slack variable of a cut.
        index_t offset{0};
        //! The next candidate cut to replace when the limit of cuts is reached.
        size_t next{0};
        //! Cuts whose values have to be recomputed when backtracking.
        //!
        //! The trail cannot restore their values for decision levels before
        //! they were added.
        std::vector<size_t> fresh;
        //! Buffer for the terms of a cut.
        std::vector<CutTerm> terms;
        //! Buffer for a cut.
        Cut cut;
        //! Buffer to compute the definition of a cut.
        std::vector<std::pair<index_t, Rational>> def;
    };
    //! The state for integer variables (see integer.cc).
    struct Integers {
        //! The order literals for each structural variable.
        std::vector<OrderLiterals> orders;
        //! Buffer for implications between order literals.
        std::vector<Implication> implications;
        //! Buffer for the columns, bound literals, and coefficients of the objective row.
        std::vector<std::tuple<index_t, Clingo::SolverLiteral, Integer>> fix_reasons;
        //! Number of times an integer variable with a fractional value was found.
        size_t fractional{0};
        //! Whether there are integer variables.
        bool active{false};
    };
    struct TrailOffset {
        index_t level;
        index_t bound;
        index_t assignment;
    };
    //! Captures what is know about of the satisfiability of a problem while
    //! solving.
    enum class State : uint8_t { Satisfiable = 0, Unsatisfiable = 1, Unknown = 2 };
    //! Captures the objective function.
    struct Objective {
        explicit operator bool() const { return active; }
        //! The index of the objective variable.
        index_t var{0};
        //! The bound for global optimization.
        index_t bound_var{0};
        //! The generation at which the last objective has been integrated.
        size_t generation{0};
        //! Whether there is an objective function.
        bool active{false};
        //! Whether the problem is bounded.
        bool discard_bounded{false};
        //! Whether the problem is bounded.
        bool bounded{true};
    };

  public:
    //! Construct a new solver object.
    explicit Solver(Options const &options);

    //! Prepare inequalities for solving.
    //!
    //! The (sorted) indices in integers refer to variables in symbols that
    //! have to take integer values.
    [[nodiscard]] auto prepare(Clingo::Assignment ass, Clingo::PropagateInit init, SymbolMap const &symbols,
                               std::vector<Inequality> const &inequalities, std::vector<Term> const &objective,
                               std::vector<index_t> const &integers, bool master) -> bool;

    //! Solve the (previously prepared) problem.
    [[nodiscard]] auto solve(Clingo::Assignment ass, Clingo::PropagateControl ctl, Clingo::SolverLiteralSpan lits)
        -> bool;

    //! Undo assignments on the current level.
    void undo();

    //! Get the currently assigned value.
    [[nodiscard]] auto get_value(index_t i) const -> Value;

    //! Get the currently assigned objective value.
    [[nodiscard]] auto get_objective() const -> std::optional<std::pair<Value, bool>>;

    //! Compute the optimal value for the objective function.
    void optimize();

    //! Integrate the objective into this solver.
    auto integrate_objective(Clingo::PropagateControl ctl, ObjectiveState<Value> &state) -> bool;

    //! Discard bounded solutions (if necessary).
    auto discard_bounded(Clingo::PropagateControl ctl) -> bool;

    //! Ensure that the current (SAT) assignment will not be backtracked.
    void store_sat_assignment();

    //! Check an optimal total assignment w.r.t. the integer variables.
    //!
    //! The function returns true if the assignment is accepted. Otherwise, it
    //! added literals or clauses, for example, bounds tightened via reduced
    //! costs, a conflict detected by the GCD test, a cut, or a branch.
    [[nodiscard]] auto check_integers(Clingo::Assignment ass, Clingo::PropagateControl ctl) -> bool;

    //! Return the solve statistics.
    [[nodiscard]] auto statistics() const -> Statistics const &;

    //! Adjust the sign of the given literal so that it does not conflict with
    //! the current tableau.
    [[nodiscard]] auto adjust(Clingo::SolverLiteral lit) const -> Clingo::SolverLiteral;

    //! Adjust the sign of a literal introduced during solving.
    [[nodiscard]] auto adjust_branch(Clingo::SolverLiteral lit) const -> Clingo::SolverLiteral;

  private:
    //! Check if the tableau.
    [[nodiscard]] auto check_tableau_() -> bool;
    //! Check if basic variables with unsatisfied bounds are enqueued.
    [[nodiscard]] auto check_basic_() -> bool;
    //! Check if bounds of non-basic variables are satisfied.
    [[nodiscard]] auto check_non_basic_() -> bool;
    //! Check if the current assignment is a solution.
    //!
    //! If cuts is false, the bounds of cuts are not checked, which might be
    //! violated after backtracking if they were added after the last
    //! satisfying assignment.
    [[nodiscard]] auto check_solution_(bool cuts = true) -> bool;
    //! Print a readable representation of the internal problem to stderr.
    void debug_();
    //! Propagate (some) bounds.
    [[nodiscard]] auto propagate_(Clingo::Assignment ass, Clingo::PropagateControl ctl) -> bool;

    //! Apply the given bound.
    [[nodiscard]] auto update_bound_(Clingo::Assignment ass, Clingo::PropagateControl ctl, Bound const &bound) -> bool;

    //! Insert a new bound dynamically.
    [[nodiscard]] auto assert_bound_(Clingo::PropagateControl ctl, Value value) -> bool;

    // The functions below for integer variables are implemented in
    // integer.cc. Functions returning bool return true if the assignment is
    // accepted and false if they added literals or clauses.

    //! Tighten bounds of integer variables using reduced costs.
    //!
    //! This requires an optimal assignment w.r.t. the objective and a bound on
    //! the objective from the global optimization.
    [[nodiscard]] auto fix_reduced_costs_(Clingo::PropagateControl ctl) -> bool;
    //! Check whether all integer variables have integer values and otherwise
    //! add a GCD conflict, a cut, or branch.
    [[nodiscard]] auto check_integrality_(Clingo::Assignment ass, Clingo::PropagateControl ctl) -> bool;
    //! Check if a row over integer variables has no integer solution.
    [[nodiscard]] auto gcd_test_(Clingo::PropagateControl ctl) -> bool;
    //! Branch on the fractional integer variable with the given index.
    void branch_(Clingo::PropagateControl ctl, index_t ii);
    //! Get the order literal l <-> x_ii <= k creating it if necessary.
    //!
    //! Returns std::nullopt if adding a clause failed.
    [[nodiscard]] auto order_lit_(Clingo::PropagateControl ctl, index_t ii, Integer k, bool prefer_upper)
        -> std::optional<Clingo::SolverLiteral>;
    //! Get the order literals of the given variable collecting the bounds of
    //! the program if necessary.
    auto orders_(index_t ii) -> OrderLiterals &;
    //! Try to add a Gomory cut.
    [[nodiscard]] auto cut_(Clingo::Assignment ass, Clingo::PropagateControl ctl) -> bool;
    //! Derive a Gomory cut from the given row into cuts_.cut.
    //!
    //! Returns false if no cut could be derived.
    [[nodiscard]] auto derive_cut_(index_t i) -> bool;
    //! Add the cut in cuts_.cut as a new row.
    [[nodiscard]] auto add_cut_(Clingo::Assignment ass, Clingo::PropagateControl ctl) -> bool;
    //! Get the slack variable for a new cut reusing an inactive one if the
    //! limit of cuts is reached.
    auto cut_slot_(Clingo::Assignment ass) -> std::optional<size_t>;
    //! Recompute values of cuts added since the last satisfying assignment.
    void refresh_cuts_();

    //! Enqueue basic variable `x_i` if it is conflicting.
    void enqueue_(index_t i);

    //! Set the value of non-basic `x_j` variable to `v`.
    void update_(index_t level, index_t j, Value v);

    //! Pivots basic variable `x_i` and non-basic variable `x_j`.
    void pivot_(index_t level, index_t i, index_t j, Value const &v);

    //! Select a conflicting basic variable using the given rule.
    auto select_basic_(BasicRule rule) -> std::optional<index_t>;
    //! Select a pivot point using the configured rules or Bland's rule.
    auto select_(index_t &ret_i, index_t &ret_j, Value const *&ret_v, bool bland) -> State;
    //! Get the number of pivots without progress after which Bland's rule is used.
    [[nodiscard]] auto bland_threshold_() const -> size_t;

    //! Get basic variable associated with row `i`.
    auto basic_(index_t i) -> Variable &;
    //! Get non-basic variable associated with column `j`.
    auto non_basic_(index_t j) -> Variable &;

    //! Options configuring the algorithms.
    Options const &options_;
    //! Mapping from literals to bounds.
    std::unordered_multimap<Clingo::SolverLiteral, Bound> bounds_;
    //! Trail of bound assignments (variable, relation, Value).
    std::vector<std::tuple<index_t, BoundRelation, Bound const *>> bound_trail_;
    //! Trail for assignments (level, variable, Value).
    std::vector<std::tuple<index_t, index_t, Value>> assignment_trail_;
    //! Trail offsets per level.
    std::vector<TrailOffset> trail_offset_;
    //! The tableau of coefficients.
    Tableau tableau_;
    //! The non-basic and basic variables.
    std::vector<Variable> variables_;
    //! The set of conflicting variables (organized as a min-heap).
    std::vector<index_t> conflicts_;
    //! The conflict clause.
    std::vector<Clingo::SolverLiteral> conflict_clause_;
    //! The vector of non-basic variables to propagate.
    std::deque<index_t> propagate_queue_;
    //! Problem and solving statistics.
    Statistics statistics_;
    //! The number of non-basic variables.
    index_t n_non_basic_{0};
    //! The number of basic variables.
    index_t n_basic_{0};
    //! The objective function.
    Objective objective_;
    //! The state of the cuts.
    Cuts cuts_;
    //! The state for integer variables.
    Integers ints_;
};

template <typename Value> class Propagator : public Clingo::Heuristic {
  public:
    Propagator(Clingo::Library const &lib, Options options) : lib_{&lib}, options_{std::move(options)} {}
    void on_statistics(Clingo::Stats step, Clingo::Stats accu);
    void on_model(Clingo::Model const &model);

    [[nodiscard]] auto lookup_symbol(const Clingo::Symbol &symbol) const -> std::optional<index_t>;
    [[nodiscard]] auto get_symbol(index_t i) const -> Clingo::Symbol;
    [[nodiscard]] auto has_value(index_t thread_id, index_t i) const -> bool;
    [[nodiscard]] auto get_value(index_t thread_id, index_t i) const -> Value;
    [[nodiscard]] auto get_objective(index_t thread_id) const -> std::optional<std::pair<Value, bool>>;
    [[nodiscard]] auto n_values(index_t thread_id) const -> index_t;

  private:
    void do_init(Clingo::Assignment ass, Clingo::PropagateInit init) override;
    void do_check(Clingo::Assignment ass, Clingo::PropagateControl ctl) override;
    void do_propagate(Clingo::Assignment ass, Clingo::PropagateControl ctl, Clingo::SolverLiteralSpan changes) override;
    void do_undo(Clingo::Assignment ass, Clingo::SolverLiteralSpan changes) override;
    auto do_decide(Clingo::Assignment ass, Clingo::SolverLiteral fallback) -> Clingo::SolverLiteral override;

    //! Collect the sorted indices of the integer variables.
    [[nodiscard]] auto collect_integers_() const -> std::vector<index_t>;

    Clingo::Library const *lib_;
    VarMap aux_map_;
    SymbolMap var_map_;
    SymbolVec var_vec_;
    std::vector<Term> objective_;
    std::vector<Inequality> iqs_;
    std::vector<Clingo::Symbol> integers_;
    size_t facts_offset_{0};
    //! The number of variables before solving (larger ones are solver local).
    size_t n_vars_{0};
    std::vector<Clingo::SolverLiteral> facts_;
    std::vector<std::pair<size_t, Solver<Value>>> slvs_;
    ObjectiveState<Value> objective_state_;
    Options options_;
};

inline void register_theory(Clingo::Control &ctl) { ctl.parse_string(THEORY); }

} // namespace ClingoLPX
