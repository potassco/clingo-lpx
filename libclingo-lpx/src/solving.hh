#pragma once

#include "parsing.hh"
#include "problem.hh"
#include "tableau.hh"
#include "util.hh"

#include <clingo.hh>

#include <deque>
#include <mutex>
#include <optional>
#include <queue>
#include <shared_mutex>
#include <unordered_map>

using SymbolMap = std::unordered_map<Clingo::Symbol, index_t>;
using SymbolVec = std::vector<Clingo::Symbol>;

enum class StoreSATAssignments : int {
    No = 0,
    Partial = 1,
    Total = 2,
};

enum class SelectionHeuristic : int {
    None = 0,
    Match = 1,
    Conflict = 2,
};

enum class PropagateMode : int {
    None = 0,
    Changed = 1,
    Full = 2,
};

struct Options {
    SelectionHeuristic select = SelectionHeuristic::None;
    StoreSATAssignments store_sat_assignment = StoreSATAssignments::No;
    std::optional<RationalQ> global_objective = std::nullopt;
    PropagateMode propagate_mode = PropagateMode::None;
    bool propagate_conflicts = false;
};

struct Statistics {
    void reset();

    size_t pivots{0};
    size_t propagated_bounds{0};
};

//! Helper to distribute current best objective to solver threads.
template <typename Value> class ObjectiveState {
  public:
    void reset();
    void update(std::pair<Value, bool> value);
    auto value(size_t &generation) -> std::optional<std::pair<Value, bool>>;

  private:
#ifndef CLINGOLPX_NO_SHARED_MUTEX
    std::shared_mutex mutex_;
#else
    std::mutex mutex_;
#endif
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
    enum class BoundType : uint32_t {
        Lower = 0,
        Upper = 1,
        // Equal = 2,
    };
    enum class BoundRelation : uint32_t {
        LessEqual = 0,
        GreaterEqual = 1,
        Equal = 2,
    };
    template <typename V> friend auto bound_rel(Relation rel) -> typename Solver<V>::BoundRelation;
    //! The bounds associated with a Variable.
    //!
    //! In practice, there should be a lot of variables with just one bound.
    struct Bound {
        Value value;
        index_t variable{0};
        Clingo::literal_t lit{0};
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
    };
    struct TrailOffset {
        index_t level;
        index_t bound;
        index_t assignment;
    };
    //! Captures what is know about of the satisfiability of a problem while
    //! solving.
    enum class State { Satisfiable = 0, Unsatisfiable = 1, Unknown = 2 };
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
    Solver(Options const &options);

    //! Prepare inequalities for solving.
    [[nodiscard]] auto prepare(Clingo::PropagateInit &init, SymbolMap const &symbols,
                               std::vector<Inequality> const &inequalities, std::vector<Term> const &objective,
                               bool master) -> bool;

    //! Solve the (previously prepared) problem.
    [[nodiscard]] auto solve(Clingo::PropagateControl &ctl, Clingo::LiteralSpan lits) -> bool;

    //! Undo assignments on the current level.
    void undo();

    //! Get the currently assigned value.
    [[nodiscard]] auto get_value(index_t i) const -> Value;

    //! Get the currently assigned objective value.
    [[nodiscard]] auto get_objective() const -> std::optional<std::pair<Value, bool>>;

    //! Compute the optimal value for the objective function.
    void optimize();

    //! Integrate the objective into this solver.
    auto integrate_objective(Clingo::PropagateControl &ctl, ObjectiveState<Value> &state) -> bool;

    //! Discard bounded solutions (if necessary).
    auto discard_bounded(Clingo::PropagateControl &ctl) -> bool;

    //! Ensure that the current (SAT) assignment will not be backtracked.
    void store_sat_assignment();

    //! Return the solve statistics.
    [[nodiscard]] auto statistics() const -> Statistics const &;

    //! Adjust the sign of the given literal so that it does not conflict with
    //! the current tableau.
    [[nodiscard]] auto adjust(Clingo::Assignment const &assign, Clingo::literal_t lit) const -> Clingo::literal_t;

  private:
    //! Check if the tableau.
    [[nodiscard]] auto check_tableau_() -> bool;
    //! Check if basic variables with unsatisfied bounds are enqueued.
    [[nodiscard]] auto check_basic_() -> bool;
    //! Check if bounds of non-basic variables are satisfied.
    [[nodiscard]] auto check_non_basic_() -> bool;
    //! Check if the current assignment is a solution.
    [[nodiscard]] auto check_solution_() -> bool;
    //! Print a readable representation of the internal problem to stderr.
    void debug_();
    //! Propagate (some) bounds.
    [[nodiscard]] auto propagate_(Clingo::PropagateControl &ctl) -> bool;

    //! Apply the given bound.
    [[nodiscard]] auto update_bound_(Clingo::PropagateControl &ctl, Bound const &bound) -> bool;

    //! Insert a new bound dynamically.
    [[nodiscard]] auto assert_bound_(Clingo::PropagateControl &ctl, Value value) -> bool;

    //! Enqueue basic variable `x_i` if it is conflicting.
    void enqueue_(index_t i);

    //! Set the value of non-basic `x_j` variable to `v`.
    void update_(index_t level, index_t j, Value v);

    //! Pivots basic variable `x_i` and non-basic variable `x_j`.
    void pivot_(index_t level, index_t i, index_t j, Value const &v);

    //! Select pivot point using Bland's rule.
    auto select_(index_t &ret_i, index_t &ret_j, Value const *&ret_v) -> State;

    //! Get basic variable associated with row `i`.
    auto basic_(index_t i) -> Variable &;
    //! Get non-basic variable associated with column `j`.
    auto non_basic_(index_t j) -> Variable &;

    //! Options configuring the algorithms.
    Options const &options_;
    //! Mapping from literals to bounds.
    std::unordered_multimap<Clingo::literal_t, Bound> bounds_;
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
    //! The set of conflicting variables.
    std::priority_queue<index_t, std::vector<index_t>, std::greater<>> conflicts_;
    //! The conflict clause.
    std::vector<Clingo::literal_t> conflict_clause_;
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
};

template <typename Value> class Propagator : public Clingo::Heuristic {
  public:
    Propagator(Options options) : options_{std::move(options)} {}
    Propagator(Propagator const &) = default;
    Propagator(Propagator &&) noexcept = default;
    auto operator=(Propagator const &) -> Propagator & = default;
    auto operator=(Propagator &&) noexcept -> Propagator & = default;
    ~Propagator() override = default;
    void register_control(Clingo::Control &ctl);
    void on_statistics(Clingo::UserStatistics step, Clingo::UserStatistics accu);
    void on_model(Clingo::Model const &model);

    [[nodiscard]] auto lookup_symbol(Clingo::Symbol symbol) const -> std::optional<index_t>;
    [[nodiscard]] auto get_symbol(index_t i) const -> Clingo::Symbol;
    [[nodiscard]] auto has_value(index_t thread_id, index_t i) const -> bool;
    [[nodiscard]] auto get_value(index_t thread_id, index_t i) const -> Value;
    [[nodiscard]] auto get_objective(index_t thread_id) const -> std::optional<std::pair<Value, bool>>;
    [[nodiscard]] auto n_values(index_t thread_id) const -> index_t;

    void init(Clingo::PropagateInit &init) override;
    void check(Clingo::PropagateControl &ctl) override;
    void propagate(Clingo::PropagateControl &ctl, Clingo::LiteralSpan changes) override;
    void undo(Clingo::PropagateControl const &ctl, Clingo::LiteralSpan changes) noexcept override;

    auto decide(Clingo::id_t thread_id, Clingo::Assignment const &assign, Clingo::literal_t fallback)
        -> Clingo::literal_t override;

  private:
    VarMap aux_map_;
    SymbolMap var_map_;
    SymbolVec var_vec_;
    std::vector<Term> objective_;
    std::vector<Inequality> iqs_;
    size_t facts_offset_{0};
    std::vector<Clingo::literal_t> facts_;
    std::vector<std::pair<size_t, Solver<Value>>> slvs_;
    ObjectiveState<Value> objective_state_;
    Options options_;
};
