// {{{ MIT License
//
// Copyright Roland Kaminski
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//
// }}}

#include "clingo-lpx.h"
#include "parsing.hh"
#include "solving.hh"

#include <clingo.h>
#include <clingo.hh>

#include <optional>
#include <sstream>
#include <stdexcept>

#define CLINGOLPX_TRY try // NOLINT
#define CLINGOLPX_CATCH                                                                                                \
    catch (...) {                                                                                                      \
        Clingo::Detail::handle_cxx_error();                                                                            \
        return false;                                                                                                  \
    }                                                                                                                  \
    return true // NOLINT

namespace {

using Clingo::Detail::handle_error;

//! C initialization callback for the LPX propagator.
template <typename Value> auto init(clingo_propagate_init_t *i, void *data) -> bool {
    CLINGOLPX_TRY {
        Clingo::PropagateInit in(i);
        static_cast<Propagator<Value> *>(data)->init(in);
    }
    CLINGOLPX_CATCH;
}

//! C propagation callback for the LPX propagator.
template <typename Value>
auto propagate(clingo_propagate_control_t *i, const clingo_literal_t *changes, size_t size, void *data) -> bool {
    CLINGOLPX_TRY {
        Clingo::PropagateControl in(i);
        static_cast<Propagator<Value> *>(data)->propagate(in, {changes, size});
    }
    CLINGOLPX_CATCH;
}

//! C undo callback for the LPX propagator.
template <typename Value>
void undo(clingo_propagate_control_t const *i, const clingo_literal_t *changes, size_t size, void *data) {
    Clingo::PropagateControl in(const_cast<clingo_propagate_control_t *>(i)); // NOLINT
    static_cast<Propagator<Value> *>(data)->undo(in, {changes, size});
}

//! C check callback for the LPX propagator.
template <typename Value> auto check(clingo_propagate_control_t *i, void *data) -> bool {
    CLINGOLPX_TRY {
        Clingo::PropagateControl in(i);
        static_cast<Propagator<Value> *>(data)->check(in);
    }
    CLINGOLPX_CATCH;
}

//! C decide callback for the LPX propagator.
template <typename Value>
auto decide(clingo_id_t thread_id, clingo_assignment_t const *assignment, clingo_literal_t fallback, void *data,
            clingo_literal_t *decision) -> bool {
    CLINGOLPX_TRY {
        Clingo::Assignment assign(const_cast<clingo_assignment_t *>(assignment)); // NOLINT
        *decision = static_cast<Propagator<Value> *>(data)->decide(thread_id, assign, fallback);
    }
    CLINGOLPX_CATCH;
}

//! High level interface to use the LPX propagator hiding the value type.
class PropagatorFacade {
  public:
    PropagatorFacade() = default;
    PropagatorFacade(PropagatorFacade const &other) = default;
    PropagatorFacade(PropagatorFacade &&other) = default;
    auto operator=(PropagatorFacade const &other) -> PropagatorFacade & = default;
    auto operator=(PropagatorFacade &&other) noexcept -> PropagatorFacade & = default;
    virtual ~PropagatorFacade() = default;

    //! Look up the index of a symbol.
    //!
    //! The function returns false if the symbol could not be found.
    virtual auto lookup_symbol(clingo_symbol_t name, size_t *index) -> bool = 0;
    //! Get the symbol associated with an index.
    virtual auto get_symbol(size_t index) -> clingo_symbol_t = 0;
    //! Check if a symbol has a value in a thread.
    virtual auto has_value(uint32_t thread_id, size_t index) -> bool = 0;
    //! Get the value of a symbol in a thread.
    virtual void get_value(uint32_t thread_id, size_t index, clingolpx_value_t *value) = 0;
    //! Function to iterate over the thread specific assignment of symbols and values.
    //!
    //! Argument current should initially be set to 0. The function returns
    //! false if no more values are available.
    virtual auto next(uint32_t thread_id, size_t *current) -> bool = 0;
    //! Extend the given model with the assignment stored in the propagator.
    virtual void extend_model(Clingo::Model &m) = 0;
    //! Add the propagator statistics to clingo's statistics.
    virtual void on_statistics(Clingo::UserStatistics &step, Clingo::UserStatistics &accu) = 0;
};

//! High level interface to use the LPX propagator.
template <typename Value> class LPXPropagatorFacade : public PropagatorFacade {
  public:
    LPXPropagatorFacade(clingo_control_t *control, char const *theory, Options const &options) : prop_{options} {
        handle_error(clingo_control_add(control, "base", nullptr, 0, theory));
        static clingo_propagator_t prp = {
            init<Value>, propagate<Value>, undo<Value>, check<Value>, decide<Value>,
        };
        static clingo_propagator_t heu = {
            init<Value>, propagate<Value>, undo<Value>, check<Value>, nullptr,
        };
        handle_error(clingo_control_register_propagator(
            control, options.select != SelectionHeuristic::None ? &prp : &heu, &prop_, false));
    }

    auto lookup_symbol(clingo_symbol_t name, size_t *index) -> bool override {
        if (auto ret = prop_.lookup_symbol(Clingo::Symbol{name}); ret) {
            *index = *ret + 1;
            return true;
        }
        return false;
    }

    auto get_symbol(size_t index) -> clingo_symbol_t override { return prop_.get_symbol(index - 1).to_c(); }

    auto has_value(uint32_t thread_id, size_t index) -> bool override {
        return index > 0 && prop_.has_value(thread_id, index - 1);
    }

    void get_value(uint32_t thread_id, size_t index, clingolpx_value_t *value) override {
        ss_.str("");
        ss_ << prop_.get_value(thread_id, index - 1);
        value->type = clingolpx_value_type_symbol;
        value->symbol = Clingo::String(ss_.str().c_str()).to_c(); // NOLINT
    }

    auto next(uint32_t thread_id, size_t *current) -> bool override {
        while (*current < prop_.n_values(thread_id)) {
            ++*current;
            if (prop_.get_symbol(*current - 1).type() != Clingo::SymbolType::Number) {
                return true;
            }
        }
        return false;
    }

    void extend_model(Clingo::Model &model) override {
        std::vector<Clingo::Symbol> symbols;
        auto thread_id = model.thread_id();

        for (size_t i = 0; next(thread_id, &i);) {
            ss_.str("");
            ss_ << prop_.get_value(thread_id, i - 1);
            symbols.emplace_back(
                Clingo::Function("__lpx", {prop_.get_symbol(i - 1), Clingo::String(ss_.str().c_str())}));
        }
        auto objective = prop_.get_objective(thread_id);
        if (objective.has_value()) {
            ss_.str("");
            ss_ << objective->first;
            symbols.emplace_back(Clingo::Function(
                "__lpx_objective", {Clingo::String(ss_.str().c_str()), Clingo::Number(objective->second ? 1 : 0)}));
        }
        model.extend(symbols);
        prop_.on_model(model);
    }

    void on_statistics(Clingo::UserStatistics &step, Clingo::UserStatistics &accu) override {
        prop_.on_statistics(step, accu);
    }

  private:
    Propagator<Value> prop_; //!< The underlying LPX propagator.
    std::ostringstream ss_;
};

//! Check if b is a lower case prefix of a returning a pointer to the remainder of a.
auto iequals_pre(char const *a, char const *b) -> char const * {
    for (; *a && *b; ++a, ++b) { // NOLINT
        if (tolower(*a) != tolower(*b)) {
            return nullptr;
        }
    }
    return *b != '\0' ? nullptr : a;
}

//! Check if two strings are lower case equal.
auto iequals(char const *a, char const *b) -> bool {
    a = iequals_pre(a, b);
    return a != nullptr && *a == '\0';
}

//! Parse a Boolean and store it in data.
//!
//! Return false if there is a parse error.
auto parse_bool(const char *value, void *data) -> bool {
    auto &result = *static_cast<bool *>(data);
    if (iequals(value, "no") || iequals(value, "off") || iequals(value, "0")) {
        result = false;
        return true;
    }
    if (iequals(value, "yes") || iequals(value, "on") || iequals(value, "1")) {
        result = true;
        return true;
    }
    return false;
}

//! Parse value for phase selection heuristic.
auto parse_select(const char *value, void *data) -> bool {
    auto &options = *static_cast<Options *>(data);
    if (iequals(value, "none")) {
        options.select = SelectionHeuristic::None;
        return true;
    }
    if (iequals(value, "match")) {
        options.select = SelectionHeuristic::Match;
        return true;
    }
    if (iequals(value, "conflict")) {
        options.select = SelectionHeuristic::Conflict;
        return true;
    }
    return false;
}

//! Parse value for propagate mode.
auto parse_propagate(const char *value, void *data) -> bool {
    auto &options = *static_cast<Options *>(data);
    if (iequals(value, "none")) {
        options.propagate_mode = PropagateMode::None;
        return true;
    }
    if (iequals(value, "changed")) {
        options.propagate_mode = PropagateMode::Changed;
        return true;
    }
    if (iequals(value, "full")) {
        options.propagate_mode = PropagateMode::Full;
        return true;
    }
    return false;
}

//! Parse value for store SAT assignment configuration.
auto parse_store(const char *value, void *data) -> bool {
    auto &options = *static_cast<Options *>(data);
    if (iequals(value, "no")) {
        options.store_sat_assignment = StoreSATAssignments::No;
        return true;
    }
    if (iequals(value, "partial")) {
        options.store_sat_assignment = StoreSATAssignments::Partial;
        return true;
    }
    if (iequals(value, "total")) {
        options.store_sat_assignment = StoreSATAssignments::Total;
        return true;
    }
    return false;
}

//! Parse how objective function is treated.
auto parse_objective(const char *value, void *data) -> bool {
    auto &options = *static_cast<Options *>(data);
    if (iequals(value, "local")) {
        options.global_objective = std::nullopt;
        return true;
    }
    value = iequals_pre(value, "global");
    if (value == nullptr) {
        return false;
    }
    if (*value == '\0') {
        options.global_objective = RationalQ{0};
        return true;
    }
    value = iequals_pre(value, ",");
    if (value == nullptr) {
        return false;
    }
    if (iequals(value, "e")) {
        options.global_objective = RationalQ{Rational{0}, Rational{1}};
        return true;
    }
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
    options.global_objective = RationalQ{Rational{value, 10}};
    return true;
}

//! Set the given error message if the Boolean is false.
//!
//! Return false if there is a parse error.
auto check_parse(char const *key, bool ret) -> bool {
    if (!ret) {
        std::ostringstream msg;
        msg << "invalid value for '" << key << "'";
        clingo_set_error(clingo_error_runtime, msg.str().c_str());
    }
    return ret;
}

} // namespace

struct clingolpx_theory {
    std::unique_ptr<PropagatorFacade> clingolpx{nullptr};
    Options options;
    bool strict{false};
};

extern "C" void clingolpx_version(int *major, int *minor, int *patch) {
    if (major != nullptr) {
        *major = CLINGOLPX_VERSION_MAJOR;
    }
    if (minor != nullptr) {
        *minor = CLINGOLPX_VERSION_MINOR;
    }
    if (patch != nullptr) {
        *patch = CLINGOLPX_VERSION_REVISION;
    }
}

extern "C" auto clingolpx_create(clingolpx_theory_t **theory) -> bool {
    CLINGOLPX_TRY { *theory = new clingolpx_theory{}; } // NOLINT
    CLINGOLPX_CATCH;
}

extern "C" auto clingolpx_register(clingolpx_theory_t *theory, clingo_control_t *control) -> bool {
    CLINGOLPX_TRY {
        if (!theory->strict) {
            theory->clingolpx = std::make_unique<LPXPropagatorFacade<Rational>>(control, THEORY, theory->options);
        } else {
            theory->clingolpx = std::make_unique<LPXPropagatorFacade<RationalQ>>(control, THEORY_Q, theory->options);
        }
    }
    CLINGOLPX_CATCH;
}

extern "C" auto clingolpx_rewrite_ast(clingolpx_theory_t *theory, clingo_ast_t *ast, clingolpx_ast_callback_t add,
                                      void *data) -> bool {
    static_cast<void>(theory);
    return add(ast, data);
}

extern "C" auto clingolpx_prepare(clingolpx_theory_t *theory, clingo_control_t *control) -> bool {
    static_cast<void>(theory);
    static_cast<void>(control);
    return true;
}

extern "C" auto clingolpx_destroy(clingolpx_theory_t *theory) -> bool {
    CLINGOLPX_TRY { delete theory; } // NOLINT
    CLINGOLPX_CATCH;
}

extern "C" auto clingolpx_configure(clingolpx_theory_t *theory, char const *key, char const *value) -> bool {
    CLINGOLPX_TRY {
        if (strcmp(key, "strict") == 0) {
            return check_parse("strict", parse_bool(value, &theory->strict));
        }
        if (strcmp(key, "objective") == 0) {
            return check_parse("objective", parse_bool(value, &theory->options.global_objective));
        }
        if (strcmp(key, "propagate-conflicts") == 0) {
            return check_parse("propagate-conflicts", parse_bool(value, &theory->options.propagate_conflicts));
        }
        if (strcmp(key, "propagate-bounds") == 0) {
            return check_parse("propagate-bounds", parse_propagate(value, &theory->options.propagate_mode));
        }
        if (strcmp(key, "select") == 0) {
            return check_parse("select", parse_select(value, &theory->options));
        }
        if (strcmp(key, "store") == 0) {
            return check_parse("select", parse_store(value, &theory->options));
        }
        std::ostringstream msg;
        msg << "invalid configuration key '" << key << "'";
        clingo_set_error(clingo_error_runtime, msg.str().c_str());
        return false;
    }
    CLINGOLPX_CATCH;
}

extern "C" auto clingolpx_register_options(clingolpx_theory_t *theory, clingo_options_t *options) -> bool {
    CLINGOLPX_TRY {
        char const *group = "Clingo.LPX Options";
        handle_error(clingo_options_add_flag(options, group, "strict", "Enable support for strict constraints",
                                             &theory->strict));
        handle_error(clingo_options_add_flag(options, group, "propagate-conflicts", "Propagate conflicting bounds",
                                             &theory->options.propagate_conflicts));
        handle_error(clingo_options_add(options, group, "propagate-bounds", "Propagate bounds", parse_propagate,
                                        &theory->options, false, "{none,changed,full}"));
        handle_error(clingo_options_add(options, group, "objective", "Choose how to treat objective function",
                                        parse_objective, &theory->options, false, "{local,global[,step]}"));
        handle_error(clingo_options_add(options, group, "select", "Choose phase selection heuristic", parse_select,
                                        &theory->options, false, "{none,match,conflict}"));
        handle_error(clingo_options_add(options, group, "store", "Whether to store SAT assignments", parse_store,
                                        &theory->options, false, "{no,partial,total}"));
    }
    CLINGOLPX_CATCH;
}

extern "C" auto clingolpx_validate_options(clingolpx_theory_t *theory) -> bool {
    CLINGOLPX_TRY {
        if (!theory->strict && theory->options.global_objective.has_value() &&
            !theory->options.global_objective->is_rational()) {
            throw std::runtime_error("objective step value requires strict mode");
        }
    }
    CLINGOLPX_CATCH;
}

extern "C" auto clingolpx_on_model(clingolpx_theory_t *theory, clingo_model_t *model) -> bool {
    CLINGOLPX_TRY {
        Clingo::Model m(model);
        theory->clingolpx->extend_model(m);
    }
    CLINGOLPX_CATCH;
}

extern "C" auto clingolpx_lookup_symbol(clingolpx_theory_t *theory, clingo_symbol_t symbol, size_t *index) -> bool {
    return theory->clingolpx->lookup_symbol(symbol, index);
}

extern "C" auto clingolpx_get_symbol(clingolpx_theory_t *theory, size_t index) -> clingo_symbol_t {
    return theory->clingolpx->get_symbol(index);
}

extern "C" void clingolpx_assignment_begin(clingolpx_theory_t *theory, uint32_t thread_id, size_t *index) {
    static_cast<void>(theory);
    static_cast<void>(thread_id);
    *index = 0;
}

extern "C" auto clingolpx_assignment_next(clingolpx_theory_t *theory, uint32_t thread_id, size_t *index) -> bool {
    return theory->clingolpx->next(thread_id, index);
}

extern "C" auto clingolpx_assignment_has_value(clingolpx_theory_t *theory, uint32_t thread_id, size_t index) -> bool {
    return theory->clingolpx->has_value(thread_id, index);
}

extern "C" void clingolpx_assignment_get_value(clingolpx_theory_t *theory, uint32_t thread_id, size_t index,
                                               clingolpx_value_t *value) {
    theory->clingolpx->get_value(thread_id, index, value);
}

extern "C" auto clingolpx_on_statistics(clingolpx_theory_t *theory, clingo_statistics_t *step,
                                        clingo_statistics_t *accu) -> bool {
    CLINGOLPX_TRY {
        uint64_t root_s{0};
        uint64_t root_a{0};
        handle_error(clingo_statistics_root(step, &root_s));
        handle_error(clingo_statistics_root(accu, &root_a));
        Clingo::UserStatistics s(step, root_s);
        Clingo::UserStatistics a(accu, root_a);
        theory->clingolpx->on_statistics(s, a);
    }
    CLINGOLPX_CATCH;
}

#undef CLINGOLPX_TRY
#undef CLINGOLPX_CATCH
