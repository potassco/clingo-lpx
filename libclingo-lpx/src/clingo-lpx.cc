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

#include <clingo-lpx.h>
#include "solving.hh"
#include "parsing.hh"

#include <clingo.h>
#include <clingo.hh>
#include <sstream>

#define CLINGOLPX_TRY try // NOLINT
#define CLINGOLPX_CATCH catch (...){ Clingo::Detail::handle_cxx_error(); return false; } return true // NOLINT

namespace {

using Clingo::Detail::handle_error;

//! C initialization callback for the LPX propagator.
template <typename Factor, typename Value>
bool init(clingo_propagate_init_t* i, void* data) {
    CLINGOLPX_TRY {
        Clingo::PropagateInit in(i);
        static_cast<Propagator<Factor, Value>*>(data)->init(in);
    }
    CLINGOLPX_CATCH;
}

//! C propagation callback for the LPX propagator.
template <typename Factor, typename Value>
bool propagate(clingo_propagate_control_t* i, const clingo_literal_t *changes, size_t size, void* data) {
    CLINGOLPX_TRY {
        Clingo::PropagateControl in(i);
        static_cast<Propagator<Factor, Value>*>(data)->propagate(in, {changes, size});
    }
    CLINGOLPX_CATCH;
}

//! C undo callback for the LPX propagator.
template <typename Factor, typename Value>
void undo(clingo_propagate_control_t const* i, const clingo_literal_t *changes, size_t size, void* data) {
    Clingo::PropagateControl in(const_cast<clingo_propagate_control_t *>(i)); // NOLINT
    static_cast<Propagator<Factor, Value>*>(data)->undo(in, {changes, size});
}

//! C check callback for the LPX propagator.
template <typename Factor, typename Value>
bool check(clingo_propagate_control_t* i, void* data) {
    CLINGOLPX_TRY {
        Clingo::PropagateControl in(i);
        static_cast<Propagator<Factor, Value>*>(data)->check(in);
    }
    CLINGOLPX_CATCH;
}

//! C decide callback for the LPX propagator.
template <typename Factor, typename Value>
bool decide(clingo_id_t thread_id, clingo_assignment_t const *assignment, clingo_literal_t fallback, void *data, clingo_literal_t *decision) {
    CLINGOLPX_TRY {
        Clingo::Assignment assign(const_cast<clingo_assignment_t *>(assignment)); // NOLINT
        *decision = static_cast<Propagator<Factor, Value>*>(data)->decide(thread_id, assign, fallback);
    }
    CLINGOLPX_CATCH;
}

//! High level interface to use the LPX propagator hiding the value type.
class PropagatorFacade {
public:
    PropagatorFacade() = default;
    PropagatorFacade(PropagatorFacade const &other) = default;
    PropagatorFacade(PropagatorFacade &&other) = default;
    PropagatorFacade &operator=(PropagatorFacade const &other) = default;
    PropagatorFacade &operator=(PropagatorFacade &&other) noexcept = default;
    virtual ~PropagatorFacade() = default;

    //! Look up the index of a symbol.
    //!
    //! The function returns false if the symbol could not be found.
    virtual bool lookup_symbol(clingo_symbol_t name, size_t *index) = 0;
    //! Get the symbol associated with an index.
    virtual clingo_symbol_t get_symbol(size_t index) = 0;
    //! Check if a symbol has a value in a thread.
    virtual bool has_value(uint32_t thread_id, size_t index) = 0;
    //! Get the value of a symbol in a thread.
    virtual void get_value(uint32_t thread_id, size_t index, clingolpx_value_t *value) = 0;
    //! Function to iterate over the thread specific assignment of symbols and values.
    //!
    //! Argument current should initially be set to 0. The function returns
    //! false if no more values are available.
    virtual bool next(uint32_t thread_id, size_t *current) = 0;
    //! Extend the given model with the assignment stored in the propagator.
    virtual void extend_model(Clingo::Model &m) = 0;
    //! Add the propagator statistics to clingo's statistics.
    virtual void on_statistics(Clingo::UserStatistics& step, Clingo::UserStatistics &accu) = 0;
};

//! High level interface to use the LPX propagator.
template <typename Factor, typename Value>
class LPXPropagatorFacade : public PropagatorFacade {
public:
    LPXPropagatorFacade(clingo_control_t *control, char const *theory, SelectionHeuristic heuristic, bool propagate_conflicts)
    : prop_{heuristic, propagate_conflicts} {
        handle_error(clingo_control_add(control, "base", nullptr, 0, theory));
        static clingo_propagator_t prp = {
            init<Factor, Value>,
            propagate<Factor, Value>,
            undo<Factor, Value>,
            check<Factor, Value>,
            decide<Factor, Value>,
        };
        static clingo_propagator_t heu = {
            init<Factor, Value>,
            propagate<Factor, Value>,
            undo<Factor, Value>,
            check<Factor, Value>,
            nullptr,
        };
        handle_error(clingo_control_register_propagator(control, heuristic != SelectionHeuristic::None ? &prp : &heu, &prop_, false));
    }

    bool lookup_symbol(clingo_symbol_t name, size_t *index) override {
        if (auto ret = prop_.lookup_symbol(Clingo::Symbol{name}); ret) {
            *index = *ret + 1;
            return true;
        }
        return false;
    }

    clingo_symbol_t get_symbol(size_t index) override {
        return prop_.get_symbol(index - 1).to_c();
    }

    bool has_value(uint32_t thread_id, size_t index) override {
        return index > 0 && prop_.has_value(thread_id, index - 1);
    }

    void get_value(uint32_t thread_id, size_t index, clingolpx_value_t *value) override {
        ss_.str();
        ss_ << prop_.get_value(thread_id, index - 1);
        value->type = clingolpx_value_type_symbol;
        value->symbol = Clingo::String(ss_.str().c_str()).to_c(); // NOLINT
    }

    bool next(uint32_t thread_id, size_t *current) override {
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
            symbols.emplace_back(Clingo::Function("__lpx", {prop_.get_symbol(i - 1), Clingo::String(ss_.str().c_str())}));
        }
        model.extend(symbols);
    }

    void on_statistics(Clingo::UserStatistics& step, Clingo::UserStatistics &accu) override {
        prop_.on_statistics(step, accu);
    }

private:
    Propagator<Factor, Value> prop_; //!< The underlying LPX propagator.
    std::ostringstream ss_;
};

//! Check if b is a lower case prefix of a returning a pointer to the remainder of a.
char const *iequals_pre(char const *a, char const *b) {
    for (; *a && *b; ++a, ++b) { // NOLINT
        if (tolower(*a) != tolower(*b)) { return nullptr; }
    }
    return *b != '\0' ? nullptr : a;
}

//! Check if two strings are lower case equal.
bool iequals(char const *a, char const *b) {
    a = iequals_pre(a, b);
    return a != nullptr && *a == '\0';
}

//! Parse a Boolean and store it in data.
//!
//! Return false if there is a parse error.
bool parse_bool(const char *value, void *data) {
    auto &result = *static_cast<bool*>(data);
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

bool parse_select(const char *value, void *data) {
    auto &result = *static_cast<SelectionHeuristic*>(data);
    if (iequals(value, "none")) {
        result = SelectionHeuristic::None;
        return true;
    }
    if (iequals(value, "match")) {
        result = SelectionHeuristic::Match;
        return true;
    }
    if (iequals(value, "conflict")) {
        result = SelectionHeuristic::Conflict;
        return true;
    }
    return false;
}

//! Set the given error message if the Boolean is false.
//!
//! Return false if there is a parse error.
bool check_parse(char const *key, bool ret) {
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
    SelectionHeuristic select{SelectionHeuristic::None};
    bool propagate_conflicts{false};
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

extern "C" bool clingolpx_create(clingolpx_theory_t **theory) {
    CLINGOLPX_TRY { *theory = new clingolpx_theory{}; } // NOLINT
    CLINGOLPX_CATCH;
}

extern "C" bool clingolpx_register(clingolpx_theory_t *theory, clingo_control_t* control) {
    CLINGOLPX_TRY {
        if (!theory->strict) {
            theory->clingolpx = std::make_unique<LPXPropagatorFacade<Number, Number>>(control, THEORY, theory->select, theory->propagate_conflicts);
        }
        else {
            theory->clingolpx = std::make_unique<LPXPropagatorFacade<Number, NumberQ>>(control, THEORY_Q, theory->select, theory->propagate_conflicts);
        }
    }
    CLINGOLPX_CATCH;
}

extern "C" bool clingolpx_rewrite_ast(clingolpx_theory_t *theory, clingo_ast_t *ast, clingolpx_ast_callback_t add, void *data) {
    static_cast<void>(theory);
    return add(ast, data);
}

extern "C" bool clingolpx_prepare(clingolpx_theory_t *theory, clingo_control_t *control) {
    static_cast<void>(theory);
    static_cast<void>(control);
    return true;
}

extern "C" bool clingolpx_destroy(clingolpx_theory_t *theory) {
    CLINGOLPX_TRY { delete theory; } // NOLINT
    CLINGOLPX_CATCH;
}

extern "C" bool clingolpx_configure(clingolpx_theory_t *theory, char const *key, char const *value) {
    CLINGOLPX_TRY {
        if (strcmp(key, "strict") == 0) {
            return check_parse("strict", parse_bool(value, &theory->strict));
        }
        if (strcmp(key, "propagate-conflicts") == 0) {
            return check_parse("propagate-conflicts", parse_bool(value, &theory->propagate_conflicts));
        }
        if (strcmp(key, "select") == 0) {
            return check_parse("select", parse_select(value, &theory->select));
        }
        std::ostringstream msg;
        msg << "invalid configuration key '" << key << "'";
        clingo_set_error(clingo_error_runtime, msg.str().c_str());
        return false;
    }
    CLINGOLPX_CATCH;
}

extern "C" bool clingolpx_register_options(clingolpx_theory_t *theory, clingo_options_t* options) {
    CLINGOLPX_TRY {
        char const * group = "Clingo.LPX Options";
        handle_error(clingo_options_add_flag(options, group, "strict", "Enable support for strict constraints", &theory->strict));
        handle_error(clingo_options_add_flag(options, group, "propagate-conflicts", "Propagate conflicting bounds", &theory->propagate_conflicts));
        handle_error(clingo_options_add(options, group, "select", "Choose phase selection heuristic", parse_select, &theory->select, false, "{none,match,conflict}"));
    }
    CLINGOLPX_CATCH;
}

extern "C" bool clingolpx_validate_options(clingolpx_theory_t *theory) {
    static_cast<void>(theory);
    return true;
}

extern "C" bool clingolpx_on_model(clingolpx_theory_t *theory, clingo_model_t* model) {
    CLINGOLPX_TRY {
        Clingo::Model m(model);
        theory->clingolpx->extend_model(m);
    }
    CLINGOLPX_CATCH;
}

extern "C" bool clingolpx_lookup_symbol(clingolpx_theory_t *theory, clingo_symbol_t symbol, size_t *index) {
    return theory->clingolpx->lookup_symbol(symbol, index);
}

extern "C" clingo_symbol_t clingolpx_get_symbol(clingolpx_theory_t *theory, size_t index) {
    return theory->clingolpx->get_symbol(index);
}

extern "C" void clingolpx_assignment_begin(clingolpx_theory_t *theory, uint32_t thread_id, size_t *index) {
    static_cast<void>(theory);
    static_cast<void>(thread_id);
    *index = 0;
}

extern "C" bool clingolpx_assignment_next(clingolpx_theory_t *theory, uint32_t thread_id, size_t *index) {
    return theory->clingolpx->next(thread_id, index);
}

extern "C" bool clingolpx_assignment_has_value(clingolpx_theory_t *theory, uint32_t thread_id, size_t index) {
    return theory->clingolpx->has_value(thread_id, index);
}

extern "C" void clingolpx_assignment_get_value(clingolpx_theory_t *theory, uint32_t thread_id, size_t index, clingolpx_value_t *value) {
    theory->clingolpx->get_value(thread_id, index, value);
}

extern "C" bool clingolpx_on_statistics(clingolpx_theory_t *theory, clingo_statistics_t* step, clingo_statistics_t* accu) {
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
