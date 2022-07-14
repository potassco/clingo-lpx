#pragma once

#include "problem.hh"
#include "parsing.hh"
#include "util.hh"

#include <clingo.hh>
#include <queue>
#include <map>
#include <optional>

using Value = Number;
using Factor = Number;
using CoeffcientQ = NumberQ;
using SymbolMap = std::unordered_map<Clingo::Symbol, index_t>;
using SymbolVec = std::vector<Clingo::Symbol>;

enum class SelectionHeuristic : int {
    None = 0,
    Match = 1,
    Conflict = 2,
};

struct Statistics {
    void reset();

    size_t pivots_{0};
};

//! A solver for finding an assignment satisfying a set of inequalities.
template <typename Factor, typename Value>
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
    enum class BoundRelation : uint32_t  {
        LessEqual = 0,
        GreaterEqual = 1,
        Equal = 2,
    };
    template<typename F, typename V>
    friend typename Solver<F, V>::BoundRelation bound_rel(Relation rel);
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
        bool compare(Value const &value) const;
        //! Check if the bound conflicts with the other one.
        bool conflicts(Bound const &other) const;
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
        [[nodiscard]] Value const &lower() const { return lower_bound->value; }
        //! Return thevalue of the upper bound.
        [[nodiscard]] Value const &upper() const { return upper_bound->value; }
        //! Set a new value or add to the existing one.
        void set_value(Solver &s, index_t lvl, Value const &num, bool add);

        //! The lower bound of a variable.
        Bound const *lower_bound{nullptr};
        //! The upper bound of a variable.
        Bound const *upper_bound{nullptr};
        //! The value of the variable.
        Value value{0};
        //! Helper index for pivoting variables.
        index_t index{0};
        //! Helper index to obtain row/column index of a variable.
        index_t reserve_index{0};
        //! The level the variable was assigned on.
        index_t level{0};
        //! The bounds associated with this variable.
        std::vector<Bound const *> bounds;
        //! Whether this variales is in the queue of conflicting variables.
        bool queued{false};
    };
    struct TrailOffset {
        index_t level;
        index_t bound;
        index_t assignment;
    };
    //! Captures what is know about of the satisfiability of a problem while
    //! solving.
    enum class State {
        Satisfiable = 0,
        Unsatisfiable = 1,
        Unknown = 2
    };

public:
    //! Construct a new solver object.
    Solver(std::vector<Inequality> const &inequalities);

    //! Prepare inequalities for solving.
    [[nodiscard]] bool prepare(Clingo::PropagateInit &init, SymbolMap const &symbols);

    //! Solve the (previously prepared) problem.
    [[nodiscard]] bool solve(Clingo::PropagateControl &ctl, Clingo::LiteralSpan lits, bool propagate_conflicts);

    //! Undo assignments on the current level.
    void undo();

    //! Get the currently assigned value.
    [[nodiscard]] Value get_value(index_t i) const;

    //! Return the solve statistics.
    [[nodiscard]] Statistics const &statistics() const;

    //! Adjust the sign of the given literal so that it does not conflict with
    //! the current tableau.
    [[nodiscard]] Clingo::literal_t adjust(SelectionHeuristic heuristic, Clingo::Assignment const &assign, Clingo::literal_t lit) const;

private:
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
    void update_(index_t level, index_t j, Value v);

    //! Pivots basic variable `x_i` and non-basic variable `x_j`.
    void pivot_(index_t level, index_t i, index_t j, Value const &v);

    //! Helper function to select pivot point.
    [[nodiscard]] bool select_(bool upper, Variable &x);
    //! Select pivot point using Bland's rule.
    State select_(index_t &ret_i, index_t &ret_j, Value const *&ret_v);

    //! Get basic variable associated with row `i`.
    Variable &basic_(index_t i);
    //! Get non-basic variable associated with column `j`.
    Variable &non_basic_(index_t j);

    //! The set of inequalities.
    std::vector<Inequality> const &inequalities_;
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
    //! Problem and solving statistics.
    Statistics statistics_;
    //! The number of non-basic variables.
    index_t n_non_basic_{0};
    //! The number of basic variables.
    index_t n_basic_{0};
};

template <typename Factor, typename Value>
class Propagator : public Clingo::Heuristic {
public:
    Propagator(SelectionHeuristic heuristic, bool propagate_conflicts)
    : heuristic_{heuristic}
    , propagate_conflicts_{propagate_conflicts} { }
    Propagator(Propagator const &) = default;
    Propagator(Propagator &&) noexcept = default;
    Propagator &operator=(Propagator const &) = default;
    Propagator &operator=(Propagator &&) noexcept = default;
    ~Propagator() override = default;
    void register_control(Clingo::Control &ctl);
    void on_statistics(Clingo::UserStatistics step, Clingo::UserStatistics accu);

    [[nodiscard]] std::optional<index_t> lookup_symbol(Clingo::Symbol symbol) const;
    [[nodiscard]] Clingo::Symbol get_symbol(index_t i) const;
    [[nodiscard]] bool has_value(index_t thread_id, index_t i) const;
    [[nodiscard]] Value get_value(index_t thread_id, index_t i) const;
    [[nodiscard]] index_t n_values(index_t thread_id) const;

    void init(Clingo::PropagateInit &init) override;
    void check(Clingo::PropagateControl &ctl) override;
    void propagate(Clingo::PropagateControl &ctl, Clingo::LiteralSpan changes) override;
    void undo(Clingo::PropagateControl const &ctl, Clingo::LiteralSpan changes) noexcept override;

    Clingo::literal_t decide(Clingo::id_t thread_id, Clingo::Assignment const &assign, Clingo::literal_t fallback) override;

private:
    VarMap aux_map_;
    SymbolMap var_map_;
    SymbolVec var_vec_;
    std::vector<Inequality> iqs_;
    size_t facts_offset_{0};
    std::vector<Clingo::literal_t> facts_;
    std::vector<std::pair<size_t, Solver<Factor, Value>>> slvs_;
    SelectionHeuristic heuristic_;
    bool propagate_conflicts_;
};
