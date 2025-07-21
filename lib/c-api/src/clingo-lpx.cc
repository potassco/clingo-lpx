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

#include <clingo-lpx/parsing.hh>
#include <clingo-lpx/solving.hh>

#include <clingo.h>
#include <clingo/propagate.hh>

#include <optional>
#include <sstream>
#include <stdexcept>

using namespace ClingoLPX;

namespace {

using Clingo::Detail::handle_error;

//! C initialization callback for the LPX propagator.
template <typename Value> auto init(clingo_assignment_t const *ass, clingo_propagate_init_t *init, void *data) -> bool {
    CLINGO_TRY { static_cast<Propagator<Value> *>(data)->init(Clingo::Assignment{ass}, Clingo::PropagateInit{init}); }
    CLINGO_CATCH;
}

//! C propagation callback for the LPX propagator.
template <typename Value>
auto propagate(clingo_assignment_t const *ass, clingo_propagate_control_t *ctl, clingo_literal_t const *changes,
               size_t size, void *data) -> bool {
    CLINGO_TRY {
        static_cast<Propagator<Value> *>(data)->propagate(Clingo::Assignment{ass}, Clingo::PropagateControl{ctl},
                                                          {changes, size});
    }
    CLINGO_CATCH;
}

//! C undo callback for the LPX propagator.
template <typename Value>
void undo(clingo_assignment_t const *ass, clingo_literal_t const *changes, size_t size, void *data) {
    static_cast<Propagator<Value> *>(data)->undo(Clingo::Assignment{ass}, {changes, size});
}

//! C check callback for the LPX propagator.
template <typename Value>
auto check(clingo_assignment_t const *ass, clingo_propagate_control_t *init, void *data) -> bool {
    CLINGO_TRY {
        static_cast<Propagator<Value> *>(data)->check(Clingo::Assignment{ass}, Clingo::PropagateControl{init});
    }
    CLINGO_CATCH;
}

//! C decide callback for the LPX propagator.
template <typename Value>
auto decide(clingo_assignment_t const *assignment, clingo_literal_t fallback, void *data, clingo_literal_t *decision)
    -> bool {
    CLINGO_TRY { *decision = static_cast<Propagator<Value> *>(data)->decide(Clingo::Assignment{assignment}, fallback); }
    CLINGO_CATCH;
}

//! High level interface to use the LPX propagator hiding the value type.
class PropagatorFacade {
  public:
    PropagatorFacade() = default;
    PropagatorFacade(PropagatorFacade const &other) = delete;
    virtual ~PropagatorFacade() = default;

    //! Look up the index of a symbol.
    //!
    //! The function returns false if the symbol could not be found.
    virtual auto lookup_symbol(Clingo::Symbol const &name, size_t &index) -> bool = 0;
    //! Get the symbol associated with an index.
    virtual auto get_symbol(size_t index) -> Clingo::Symbol = 0;
    //! Check if a symbol has a value in a thread.
    virtual auto has_value(uint32_t thread_id, size_t index) -> bool = 0;
    //! Get the value of a symbol in a thread.
    virtual auto get_value(Clingo::Library const &lib, uint32_t thread_id, size_t index) -> Clingo::Symbol = 0;
    //! Function to iterate over the thread specific assignment of symbols and values.
    //!
    //! Argument current should initially be set to 0. The function returns
    //! false if no more values are available.
    virtual auto next(uint32_t thread_id, size_t &current) -> bool = 0;
    //! Extend the given model with the assignment stored in the propagator.
    virtual void extend_model(Clingo::Library const &lib, Clingo::Model m) = 0;
    //! Add the propagator statistics to clingo's statistics.
    virtual void on_statistics(Clingo::Stats step, Clingo::Stats accu) = 0;
};

//! High level interface to use the LPX propagator.
template <typename Value> class LPXPropagatorFacade : public PropagatorFacade {
  public:
    LPXPropagatorFacade(Clingo::Library const &lib, clingo_control_t *control, std::string_view theory,
                        Options const &options)
        : prop_{lib, options} {
        handle_error(clingo_control_parse_string(control, theory.data(), theory.size()));
        static clingo_propagator_t prp = {
            init<Value>, nullptr, propagate<Value>, undo<Value>, check<Value>, decide<Value>, nullptr,
        };
        static clingo_propagator_t heu = {
            init<Value>, nullptr, propagate<Value>, undo<Value>, check<Value>, nullptr, nullptr,
        };
        handle_error(clingo_control_register_propagator(
            control, options.select != SelectionHeuristic::None ? &prp : &heu, &prop_));
    }

    auto lookup_symbol(Clingo::Symbol const &name, size_t &index) -> bool override {
        if (auto ret = prop_.lookup_symbol(name); ret) {
            index = *ret + 1;
            return true;
        }
        return false;
    }

    auto get_symbol(size_t index) -> Clingo::Symbol override {
        return prop_.get_symbol(static_cast<index_t>(index - 1));
    }

    auto has_value(uint32_t thread_id, size_t index) -> bool override {
        return index > 0 && prop_.has_value(thread_id, static_cast<index_t>(index - 1));
    }

    auto get_value(Clingo::Library const &lib, uint32_t thread_id, size_t index) -> Clingo::Symbol override {
        ss_.str("");
        ss_ << prop_.get_value(thread_id, static_cast<index_t>(index - 1));
        return Clingo::String(lib, ss_.view());
    }

    auto next(uint32_t thread_id, size_t &current) -> bool override {
        while (current < prop_.n_values(thread_id)) {
            ++current;
            if (prop_.get_symbol(static_cast<index_t>(current - 1)).type() != Clingo::SymbolType::number) {
                return true;
            }
        }
        return false;
    }

    void extend_model(Clingo::Library const &lib, Clingo::Model model) override {
        std::vector<Clingo::Symbol> symbols;
        auto thread_id = model.thread_id();

        for (size_t i = 0; next(thread_id, i);) {
            ss_.str("");
            ss_ << prop_.get_value(thread_id, static_cast<index_t>(i - 1));
            symbols.emplace_back(Clingo::Function(
                lib, "__lpx", {prop_.get_symbol(static_cast<index_t>(i - 1)), Clingo::String(lib, ss_.view())}));
        }
        auto objective = prop_.get_objective(thread_id);
        if (objective.has_value()) {
            ss_.str("");
            ss_ << objective->first;
            symbols.emplace_back(Clingo::Function(
                lib, "__lpx_objective", {Clingo::String(lib, ss_.view()), Clingo::Number(objective->second ? 1 : 0)}));
        }
        model.extend(symbols);
        prop_.on_model(model);
    }

    void on_statistics(Clingo::Stats step, Clingo::Stats accu) override { prop_.on_statistics(step, accu); }

  private:
    Propagator<Value> prop_; //!< The underlying LPX propagator.
    std::ostringstream ss_;
};

//! Ascii tolower conversion.
constexpr auto tolower(char c) -> char { return (c >= 'A' && c <= 'Z') ? c + ('a' - 'A') : c; }

//! Check if b is a lower case prefix of a returning a string_view to the remainder of a.
auto iequals_pre(std::string_view a, std::string_view b) -> std::optional<std::string_view> {
    if (a.size() < b.size()) {
        return std::nullopt;
    }
    auto cmp = [](char ac, char bc) { return tolower(ac) == tolower(bc); };
    auto res = std::ranges::mismatch(b, a, cmp);
    return res.in1 == b.end() ? std::optional{a.substr(b.size())} : std::nullopt;
}

//! Check if two strings are lower case equal.
auto iequals(std::string_view a, std::string_view b) -> bool {
    auto res = iequals_pre(a, b);
    return res && res->empty();
}

auto parse_bool(char const *value, size_t size, void *data, bool *result) -> bool {
    CLINGO_TRY {
        auto str = std::string_view{value, size};
        auto &target = *static_cast<bool *>(data);
        *result = true;
        if (iequals(str, "no") || iequals(str, "off") || iequals(str, "0")) {
            target = false;
        } else if (iequals(str, "yes") || iequals(str, "on") || iequals(str, "1")) {
            target = true;
        } else {
            *result = false;
        }
    }
    CLINGO_CATCH;
}

//! Parse value for phase selection heuristic.
auto parse_select(char const *value, size_t size, void *data, bool *result) -> bool {
    CLINGO_TRY {
        auto str = std::string_view{value, size};
        auto &options = *static_cast<Options *>(data);
        *result = true;
        if (iequals(str, "none")) {
            options.select = SelectionHeuristic::None;
        } else if (iequals(str, "match")) {
            options.select = SelectionHeuristic::Match;
        } else if (iequals(str, "conflict")) {
            options.select = SelectionHeuristic::Conflict;
        } else {
            *result = false;
        }
    }
    CLINGO_CATCH;
}

//! Parse value for propagate mode.
auto parse_propagate(char const *value, size_t size, void *data, bool *result) -> bool {
    CLINGO_TRY {
        auto str = std::string_view{value, size};
        auto &options = *static_cast<Options *>(data);
        *result = true;
        if (iequals(str, "none")) {
            options.propagate_mode = PropagateMode::None;
        } else if (iequals(str, "changed")) {
            options.propagate_mode = PropagateMode::Changed;
        } else if (iequals(str, "full")) {
            options.propagate_mode = PropagateMode::Full;
        } else {
            *result = false;
        }
    }
    CLINGO_CATCH;
}

//! Parse value for store SAT assignment configuration.
auto parse_store(char const *value, size_t size, void *data, bool *result) -> bool {
    CLINGO_TRY {
        auto str = std::string_view{value, size};
        auto &options = *static_cast<Options *>(data);
        *result = true;
        if (iequals(str, "no")) {
            options.store_sat_assignment = StoreSATAssignments::No;
        } else if (iequals(str, "partial")) {
            options.store_sat_assignment = StoreSATAssignments::Partial;
        } else if (iequals(str, "total")) {
            options.store_sat_assignment = StoreSATAssignments::Total;
        } else {
            *result = false;
        }
    }
    CLINGO_CATCH;
}

//! Parse how objective function is treated.
auto parse_objective(char const *value, size_t size, void *data, bool *result) -> bool {
    CLINGO_TRY {
        auto str = std::string_view{value, size};
        auto &options = *static_cast<Options *>(data);
        *result = true;
        if (iequals(str, "local")) {
            options.global_objective = std::nullopt;
        } else if (auto res = iequals_pre(str, "global")) {
            auto str = *res;
            if (str.empty()) {
                options.global_objective = RationalQ{0};
            } else if (auto res = iequals_pre(str, ",")) {
                str = *res;
                if (iequals(str, "e")) {
                    options.global_objective = RationalQ{Rational{0}, Rational{1}};
                } else {
                    options.global_objective = RationalQ{Rational{value, 10}};
                }
            } else {
                *result = false;
            }
        } else {
            *result = false;
        }
    }
    CLINGO_CATCH;
}

//! Set the given error message if the Boolean is false.
//!
//! Return false if there is a parse error.
template <class F, class... As> auto check_parse(char const *key, F fun, As &&...as) -> bool {
    CLINGO_TRY {
        bool res = false;
        if (!fun(std::forward<As>(as)..., &res)) {
            return false;
        }
        if (!res) {
            auto msg = std::ostringstream{};
            msg << "invalid value for '" << key << "'";
            clingo_set_error(clingo_result_invalid, msg.view().data(), msg.view().size());
        }
    }
    CLINGO_CATCH;
}

struct clingolpx_theory {
    Clingo::Library lib;
    std::unique_ptr<PropagatorFacade> clingolpx{nullptr};
    Options options;
    bool strict{false};

    static auto info([[maybe_unused]] void *self, clingo_string_t *name, int *major, int *minor, int *patch) -> bool {
        using namespace std::string_view_literals;
        CLINGO_TRY {
            if (name != nullptr) {
                constexpr auto str = "clingo-lpx"sv;
                name->data = str.data();
                name->size = str.size();
            }
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
        CLINGO_CATCH;
    }

    static auto register_(void *self, clingo_control_t *control) -> bool {
        CLINGO_TRY {
            auto *theory = static_cast<clingolpx_theory *>(self);
            if (!theory->strict) {
                theory->clingolpx =
                    std::make_unique<LPXPropagatorFacade<Rational>>(theory->lib, control, THEORY, theory->options);
            } else {
                theory->clingolpx =
                    std::make_unique<LPXPropagatorFacade<RationalQ>>(theory->lib, control, THEORY_Q, theory->options);
            }
        }
        CLINGO_CATCH;
    }

    static auto rewrite_ast([[maybe_unused]] void *self, clingo_ast_t *ast, clingo_theory_ast_callback_t add,
                            void *data) -> bool {
        return add(ast, data);
    }

    static auto prepare([[maybe_unused]] void *self, [[maybe_unused]] clingo_control_t *control) -> bool {
        return true;
    }

    static void destroy(void *self) {
        auto theory = static_cast<clingolpx_theory *>(self);
        std::unique_ptr<clingolpx_theory>{theory};
    }

    static auto configure(void *self, char const *key, size_t key_size, char const *value, size_t value_size) -> bool {
        CLINGO_TRY {
            auto theory = static_cast<clingolpx_theory *>(self);
            auto sv_key = std::string_view{key, key_size};
            if (sv_key == "strict") {
                return check_parse("strict", parse_bool, value, value_size, &theory->strict);
            }
            if (sv_key == "objective") {
                return check_parse("objective", parse_bool, value, value_size, &theory->options.global_objective);
            }
            if (sv_key == "propagate-conflicts") {
                return check_parse("propagate-conflicts", parse_bool, value, value_size,
                                   &theory->options.propagate_conflicts);
            }
            if (sv_key == "propagate-bounds") {
                return check_parse("propagate-bounds", parse_propagate, value, value_size, &theory->options);
            }
            if (sv_key == "select") {
                return check_parse("select", parse_select, value, value_size, &theory->options);
            }
            if (sv_key == "store") {
                return check_parse("select", parse_store, value, value_size, &theory->options);
            }
            auto msg = std::ostringstream{};
            msg << "invalid configuration key '" << sv_key << "'";
            clingo_set_error(clingo_result_runtime, msg.view().data(), msg.view().size());
            return false;
        }
        CLINGO_CATCH;
    }

    static auto register_options(void *self, clingo_options_t *options) -> bool {
        CLINGO_TRY {
            using namespace std::string_view_literals;
            auto theory = static_cast<clingolpx_theory *>(self);
            auto group = "Clingo.LPX Options"sv;
            auto opt = [&](std::string_view name, std::string_view desc, clingo_option_parser_t parser,
                           bool multi = false, std::string_view arg = {}) {
                handle_error(clingo_options_add(options, group.data(), group.size(), name.data(), name.size(),
                                                desc.data(), desc.size(), parser, &theory->options, multi,
                                                arg.empty() ? nullptr : arg.data(), arg.size()));
            };
            auto flag = [&](std::string_view name, std::string_view desc, bool &target) {
                handle_error(clingo_options_add_flag(options, group.data(), group.size(), name.data(), name.size(),
                                                     desc.data(), desc.size(), &target));
            };
            flag("strict", "Enable support for strict constraints", theory->strict);
            flag("propagate-conflicts", "Propagate conflicting bounds", theory->options.propagate_conflicts);
            opt("propagate-bounds", "Propagate bounds", parse_propagate, false, "{none,changed,full}");
            opt("objective", "Choose how to treat objective function", parse_objective, false, "{local,global[,step]}");
            opt("select", "Choose phase selection heuristic", parse_select, false, "{none,match,conflict}");
            opt("store", "Whether to store SAT assignments", parse_store, false, "{no,partial,total}");
        }
        CLINGO_CATCH;
    }

    static auto validate_options(void *self) -> bool {
        CLINGO_TRY {
            auto *theory = static_cast<clingolpx_theory *>(self);
            if (!theory->strict && theory->options.global_objective.has_value() &&
                !theory->options.global_objective->is_rational()) {
                throw std::invalid_argument{"objective step value requires strict mode"};
            }
        }
        CLINGO_CATCH;
    }

    static auto on_model(void *self, clingo_model_t *model) -> bool {
        CLINGO_TRY {
            auto *theory = static_cast<clingolpx_theory *>(self);
            Clingo::Model m(model);
            theory->clingolpx->extend_model(theory->lib, m);
        }
        CLINGO_CATCH;
    }

    static auto lookup_symbol(void *self, clingo_symbol_t symbol, size_t *index, bool *found) -> bool {
        CLINGO_TRY {
            auto theory = static_cast<clingolpx_theory *>(self);
            *found = theory->clingolpx->lookup_symbol(Clingo::Symbol{symbol, true}, *index);
        }
        CLINGO_CATCH;
    }

    static auto assignment_next(void *self, uint32_t thread_id, bool *init, size_t *index, bool *has_value) -> bool {
        CLINGO_TRY {
            auto theory = static_cast<clingolpx_theory *>(self);
            if (std::exchange(*init, false)) {
                *index = 0;
            }
            *has_value = theory->clingolpx->next(thread_id, *index);
        }
        CLINGO_CATCH;
    }

    static auto assignment_get_value(void *self, uint32_t thread_id, size_t index, clingo_symbol_t *symbol,
                                     clingo_theory_value_t *value, bool *has_value) -> bool {
        CLINGO_TRY {
            auto theory = static_cast<clingolpx_theory *>(self);
            bool hv = theory->clingolpx->has_value(thread_id, index);
            if (has_value != nullptr) {
                *has_value = hv;
            }
            if (hv) {
                if (symbol != nullptr) {
                    *symbol = c_cast(theory->clingolpx->get_symbol(index));
                    clingo_symbol_acquire(*symbol);
                }
                if (value != nullptr) {
                    auto sym = theory->clingolpx->get_value(theory->lib, thread_id, index);
                    value->type = clingo_theory_value_type_symbol;
                    value->symbol = *c_cast(&sym);
                    clingo_symbol_acquire(value->symbol);
                }
            }
        }
        CLINGO_CATCH;
    }

    static auto on_statistics(void *self, clingo_stats_t *stats) -> bool {
        CLINGO_TRY {
            auto *theory = static_cast<clingolpx_theory *>(self);
            uint64_t root = 0;
            handle_error(clingo_stats_root(stats, &root));
            auto cpp_stats = Clingo::Stats{stats, root};
            theory->clingolpx->on_statistics(cpp_stats["user_step"], cpp_stats["user_accu"]);
        }
        CLINGO_CATCH;
    }
};

} // namespace

extern "C" bool clingolpx_create(clingo_lib_t *lib, clingo_theory_t *theory) {
    CLINGO_TRY {
        *theory = clingo_theory_t{
            clingolpx_theory::info,
            clingolpx_theory::destroy,
            clingolpx_theory::register_,
            clingolpx_theory::rewrite_ast,
            clingolpx_theory::prepare,
            clingolpx_theory::register_options,
            clingolpx_theory::validate_options,
            clingolpx_theory::configure,
            clingolpx_theory::on_model,
            clingolpx_theory::on_statistics,
            clingolpx_theory::lookup_symbol,
            clingolpx_theory::assignment_next,
            clingolpx_theory::assignment_get_value,
            nullptr,
        };
        theory->self = std::make_unique<clingolpx_theory>(Clingo::Library{lib, true}).release();
    }
    CLINGO_CATCH;
}
