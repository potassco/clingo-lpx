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

#include "clingolpx_git_hash.h"

#include <clingo.h>
#include <clingo/propagate.hh>

#include <algorithm>
#include <array>
#include <charconv>
#include <cstring>
#include <deque>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <system_error>

using namespace ClingoLPX;

namespace {

constexpr auto dec_radix = 10;

using namespace std::string_view_literals;

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
    LPXPropagatorFacade(Clingo::Library const &lib, clingo_control_t *control, Options const &options)
        : prop_{lib, options} {
        static clingo_propagator_t prp = {
            init<Value>, nullptr, propagate<Value>, undo<Value>, check<Value>, decide<Value>, nullptr,
        };
        static clingo_propagator_t heu = {
            init<Value>, nullptr, propagate<Value>, undo<Value>, check<Value>, nullptr, nullptr,
        };
        handle_error(clingo_control_register_propagator(
            control,
            options.select != SelectionHeuristic::None || options.branch_phase != BranchPhase::Saved ? &prp : &heu,
            &prop_));
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
        // Creating symbols is expensive. Hence, the symbols of the last model
        // of each thread are cached and reused for unchanged values.
        std::vector<Clingo::Symbol> symbols;
        auto thread_id = model.thread_id();
        if (cache_.size() <= thread_id) {
            cache_.resize(thread_id + 1);
        }
        auto &cache = cache_[thread_id];

        for (size_t i = 0; next(thread_id, i);) {
            auto value = prop_.get_value(thread_id, static_cast<index_t>(i - 1));
            if (cache.size() < i) {
                cache.resize(i);
            }
            auto &entry = cache[i - 1];
            if (!entry.has_value() || entry->first != value) {
                ss_.str("");
                ss_ << value;
                auto sym = Clingo::Function(
                    lib, "__lpx", {prop_.get_symbol(static_cast<index_t>(i - 1)), Clingo::String(lib, ss_.view())});
                entry.emplace(std::move(value), sym);
            }
            symbols.emplace_back(entry->second);
        }
        auto objective = prop_.get_objective(thread_id);
        if (objective.has_value()) {
            ss_.str("");
            ss_ << objective->first;
            symbols.emplace_back(Clingo::Function(
                lib, "__lpx_objective",
                {Clingo::String(lib, ss_.view()), Clingo::String(lib, objective->second ? "bounded" : "unbounded")}));
        }
        model.extend(symbols);
        prop_.on_model(model);
    }

    void on_statistics(Clingo::Stats step, Clingo::Stats accu) override { prop_.on_statistics(step, accu); }

  private:
    Propagator<Value> prop_; //!< The underlying LPX propagator.
    std::ostringstream ss_;
    //! The values and symbols of the last model per thread.
    std::vector<std::vector<std::optional<std::pair<Value, Clingo::Symbol>>>> cache_;
};

//! Ascii tolower conversion.
constexpr auto tolower(char c) -> char { return (c >= 'A' && c <= 'Z') ? static_cast<char>(c + ('a' - 'A')) : c; }

//! Check if b is a lower case prefix of a returning a string_view to the remainder of a.
auto iequals_pre(std::string_view a, std::string_view b) -> std::optional<std::string_view> {
    if (a.size() < b.size()) {
        return std::nullopt;
    }
    auto cmp = [](char ac, char bc) -> bool { return tolower(ac) == tolower(bc); };
    auto res = std::ranges::mismatch(b, a, cmp);
    return res.in1 == b.end() ? std::optional{a.substr(b.size())} : std::nullopt;
}

//! Check if two strings are lower case equal.
auto iequals(std::string_view a, std::string_view b) -> bool {
    auto res = iequals_pre(a, b);
    return res && res->empty();
}

template <class T> auto c_parse(const char *value, size_t size, void *data) -> bool {
    CLINGO_TRY { T{*static_cast<Options *>(data)}.set({value, size}); }
    CLINGO_CATCH;
}

//! A value of an enumeration with its name and description.
template <typename Enum> struct EnumEntry {
    std::string_view name;
    Enum value;
    std::string_view desc;
};

//! Format the values of an option for the help output.
//!
//! This gives a line like `<arg>: {a|b}` followed by one line per value with
//! its description, which is how clasp describes its options.
template <class Entries> auto describe_values(std::string_view arg, Entries const &entries) -> std::string {
    size_t width = 0;
    for (auto const &entry : entries) {
        width = std::max(width, entry.name.size());
    }
    std::string res = "\n      ";
    res.append(arg);
    res.append(": {");
    for (auto const &entry : entries) {
        if (res.back() != '{') {
            res += '|';
        }
        res.append(entry.name);
    }
    res += '}';
    for (auto const &entry : entries) {
        res.append("\n        ");
        res.append(entry.name);
        res.append(width - entry.name.size(), ' ');
        res.append(": ");
        res.append(entry.desc);
    }
    return res;
}

//! Map between the names and values of an enumeration.
template <typename Enum, size_t N> class EnumStringMap {
  public:
    constexpr explicit EnumStringMap(const std::array<EnumEntry<Enum>, N> &map) : map_{map} {}

    [[nodiscard]] auto from_string(std::string_view value) const -> Enum {
        for (const auto &entry : map_) {
            if (iequals(value, entry.name)) {
                return entry.value;
            }
        }
        throw std::invalid_argument("invalid enum string");
    }

    [[nodiscard]] auto to_string(Enum value) const -> std::string_view {
        for (const auto &entry : map_) {
            if (value == entry.value) {
                return entry.name;
            }
        }
        throw std::invalid_argument("invalid enum value");
    }

    //! Describe the values for the help output.
    [[nodiscard]] auto describe(std::string_view arg) const -> std::string { return describe_values(arg, map_); }

  private:
    std::array<EnumEntry<Enum>, N> map_;
};

template <typename Enum, size_t N>
// NOLINTNEXTLINE(modernize-avoid-c-arrays)
constexpr auto make_enum_string_map(EnumEntry<Enum> const (&map)[N]) {
    return EnumStringMap<Enum, N>(std::to_array(map));
}

constexpr auto propagate_modes = make_enum_string_map<PropagateMode>({
    {"none", PropagateMode::None, "Disable bound propagation"},
    {"changed", PropagateMode::Changed, "Propagate constraints with changed variables"},
    {"full", PropagateMode::Full, "Propagate all constraints"},
});
constexpr auto selection_heuristics = make_enum_string_map<SelectionHeuristic>({
    {"none", SelectionHeuristic::None, "Keep the sign chosen by the solver"},
    {"match", SelectionHeuristic::Match, "Prefer bounds satisfied by the assignment"},
    {"conflict", SelectionHeuristic::Conflict, "Prefer bounds violated by the assignment"},
});
constexpr auto store_modes = make_enum_string_map<StoreSATAssignments>({
    {"no", StoreSATAssignments::No, "Restore assignments"},
    {"partial", StoreSATAssignments::Partial, "Keep assignments after propagation"},
    {"total", StoreSATAssignments::Total, "Keep only total assignments"},
});
constexpr auto basic_rules = make_enum_string_map<BasicRule>({
    {"bland", BasicRule::Bland, "Select the smallest index"},
    {"violation", BasicRule::Violation, "Select the largest bound violation"},
});
constexpr auto non_basic_rules = make_enum_string_map<NonBasicRule>({
    {"bland", NonBasicRule::Bland, "Select the smallest index"},
    {"sparse", NonBasicRule::Sparse, "Select the sparsest column"},
});
constexpr auto entering_rules = make_enum_string_map<EnteringRule>({
    {"bland", EnteringRule::Bland, "Select the smallest index"},
    {"dantzig", EnteringRule::Dantzig, "Select the largest objective coefficient"},
});
constexpr auto branch_polarities = make_enum_string_map<BranchPolarity>({
    {"nearest", BranchPolarity::Nearest, "Try the side closer to the value first"},
    {"farthest", BranchPolarity::Farthest, "Try the side farther from the value first"},
    {"up", BranchPolarity::Up, "Try increasing the variable first"},
    {"down", BranchPolarity::Down, "Try decreasing the variable first"},
});

constexpr auto branch_phases = make_enum_string_map<BranchPhase>({
    {"saved", BranchPhase::Saved, "Keep the sign chosen by the solver"},
    {"value", BranchPhase::Value, "Select the side containing the value or the nearer one"},
});

// The classes below configure a member of the options. They are used to
// parse command line options and to provide clingo's configuration.

//! Configure an option holding an enumeration.
template <auto Member, auto const &Map> class ConfigEnum {
  public:
    explicit ConfigEnum(Options &opts) : opts_{&opts} {}

    void set(std::string_view value) { opts_->*Member = map_.from_string(value); }

    [[nodiscard]] auto get() const -> std::optional<std::string_view> { return map_.to_string(opts_->*Member); }

    [[nodiscard]] static auto describe(std::string_view arg) -> std::string { return map_.describe(arg); }

  private:
    // clang-tidy 23 mistakes the template parameter for a parenthesized
    // expression and its fix removes it
    // NOLINTNEXTLINE(readability-redundant-parentheses)
    static constexpr auto const &map_ = Map;
    Options *opts_;
};

//! Parse a non-negative integer.
auto parse_size(std::string_view value) -> size_t {
    size_t res = 0;
    auto const *begin = value.data();
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    auto const *end = value.data() + value.size();
    auto [ptr, ec] = std::from_chars(begin, end, res);
    if (ec != std::errc{} || ptr != end) {
        throw std::invalid_argument{"invalid number"};
    }
    return res;
}

//! Configure an option holding a non-negative integer.
template <size_t Options::*Member> class ConfigSize {
  public:
    explicit ConfigSize(Options &opts) : opts_{&opts} {}

    void set(std::string_view value) { opts_->*Member = parse_size(value); }

    [[nodiscard]] auto get() const -> std::optional<std::string> { return std::to_string(opts_->*Member); }

    [[nodiscard]] static auto describe(std::string_view arg) -> std::string_view {
        static_cast<void>(arg);
        return {};
    }

  private:
    Options *opts_;
};

//! Configure a Boolean option, which is a flag on the command line.
template <bool Options::*Member> class ConfigFlag {
  public:
    explicit ConfigFlag(Options &opts) : opts_{&opts} {}

    void set(std::string_view value) {
        if (iequals(value, "no") || iequals(value, "off") || iequals(value, "0")) {
            opts_->*Member = false;
        } else if (iequals(value, "yes") || iequals(value, "on") || iequals(value, "1")) {
            opts_->*Member = true;
        } else {
            throw std::invalid_argument("invalid boolean value");
        }
    }

    [[nodiscard]] auto get() const -> std::optional<std::string_view> { return opts_->*Member ? "yes" : "no"; }

    [[nodiscard]] auto target() const -> bool & { return opts_->*Member; }

  private:
    Options *opts_;
};

//! Configure the objective.
class ConfigObjective {
  public:
    explicit ConfigObjective(Options &opts) : opts_{&opts} {}

    void set(std::string_view value) {
        if (iequals(value, "local")) {
            opts_->global_objective = std::nullopt;
        } else if (auto res = iequals_pre(value, "global")) {
            value = *res;
            if (value.empty()) {
                opts_->global_objective = RationalQ{0};
            } else if (auto res = iequals_pre(value, ",")) {
                value = *res;
                if (iequals(value, "e")) {
                    opts_->global_objective = RationalQ{Rational{0}, Rational{1}};
                } else {
                    opts_->global_objective = RationalQ{Rational{std::string{value}, dec_radix}};
                }
            } else {
                throw std::invalid_argument{"invalid global objective"};
            }
        } else {
            throw std::invalid_argument{"invalid objective"};
        }
    }

    [[nodiscard]] auto get() const -> std::optional<std::string> {
        if (opts_->global_objective) {
            std::ostringstream oss;
            oss << "global";
            if (opts_->global_objective != 0) {
                oss << "," << *opts_->global_objective;
            }
            return std::move(oss).str();
        }
        return "local";
    }

    [[nodiscard]] static auto describe(std::string_view arg) -> std::string {
        static constexpr auto entries = std::to_array<EnumEntry<int>>({
            {"local", 0, "Optimize each stable model separately"},
            {"global", 1, "Optimize across stable models"},
        });
        auto res = describe_values(arg, entries);
        res.insert(res.find('}') + 1, "[,<step>]");
        res.append("\n        <step>: Improve by at least <step> ({<n>|e}, e needs --strict)");
        return res;
    }

  private:
    Options *opts_;
};

//! An option with its name on the command line, the name of its argument,
//! and a short description.
struct OptionInfo {
    std::string_view name;
    std::string_view arg;
    std::string_view desc;
};

//! Call f(info, config) for each option.
//!
//! The config is one of the classes above. Flags are listed first and have
//! no argument.
template <typename F> void for_each_option(Options &opts, F const &f) {
    f(OptionInfo{"strict", "", "Enable strict constraints using < and >"}, ConfigFlag<&Options::strict>{opts});
    f(OptionInfo{"propagate-conflicts", "", "Add binary clauses for conflicting bounds"},
      ConfigFlag<&Options::propagate_conflicts>{opts});
    f(OptionInfo{"order-branches", "", "Link branches and bounds like an order encoding"},
      ConfigFlag<&Options::order_branches>{opts});
    f(OptionInfo{"gcd-test", "", "Check integer rows for divisibility before branching"},
      ConfigFlag<&Options::gcd_test>{opts});
    f(OptionInfo{"fix-reduced-costs", "", "Tighten integer bounds via reduced costs"},
      ConfigFlag<&Options::fix_reduced_costs>{opts});
    f(OptionInfo{"objective", "<mode>", "Configure optimization"}, ConfigObjective{opts});
    f(OptionInfo{"propagate-bounds", "<mode>", "Propagate bounds of constraints"},
      ConfigEnum<&Options::propagate_mode, propagate_modes>{opts});
    f(OptionInfo{"select", "<sign>", "Configure sign heuristic for bound literals"},
      ConfigEnum<&Options::select, selection_heuristics>{opts});
    f(OptionInfo{"store", "<mode>", "Keep satisfying assignments when backtracking"},
      ConfigEnum<&Options::store_sat_assignment, store_modes>{opts});
    f(OptionInfo{"pivot-basic", "<rule>", "Select the conflicting basic variable"},
      ConfigEnum<&Options::basic_rule, basic_rules>{opts});
    f(OptionInfo{"pivot-non-basic", "<rule>", "Select the non-basic variable"},
      ConfigEnum<&Options::non_basic_rule, non_basic_rules>{opts});
    f(OptionInfo{"pivot-entering", "<rule>", "Select the entering variable when optimizing"},
      ConfigEnum<&Options::entering_rule, entering_rules>{opts});
    f(OptionInfo{"bland-threshold", "<n>", "Use Bland's rule after max(<n>,rows) stalled pivots"},
      ConfigSize<&Options::bland_threshold>{opts});
    f(OptionInfo{"branch-polarity", "<sign>", "Select the side of a branch to try first"},
      ConfigEnum<&Options::branch_polarity, branch_polarities>{opts});
    f(OptionInfo{"branch-phase", "<mode>", "Select the sign when deciding branch literals"},
      ConfigEnum<&Options::branch_phase, branch_phases>{opts});
    f(OptionInfo{"cut-period", "<n>", "Cut instead of branching every <n>-th time (0=off)"},
      ConfigSize<&Options::cut_period>{opts});
    f(OptionInfo{"cut-limit", "<n>", "Keep at most <n> cuts per thread"}, ConfigSize<&Options::cut_limit>{opts});
}

struct clingolpx_theory {
    Clingo::Library lib;
    std::unique_ptr<PropagatorFacade> clingolpx{nullptr};
    Options options;
    //! Storage for strings passed to clingo (a deque does not move its elements).
    std::deque<std::string> strings;

    static auto info([[maybe_unused]] void *self, clingo_string_t *name, int *major, int *minor, int *patch,
                     clingo_string_t *hash) -> bool {
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
            if (hash != nullptr) {
#ifdef CLINGOLPX_GIT_HASH
                constexpr auto str = std::string_view{CLINGOLPX_GIT_HASH};
#else
                constexpr auto str = ""sv;
#endif
                hash->data = str.data();
                hash->size = str.size();
            }
        }
        CLINGO_CATCH;
    }

    static auto register_(void *self, clingo_control_t *control) -> bool {
        CLINGO_TRY {
            auto *theory = static_cast<clingolpx_theory *>(self);
            handle_error(clingo_control_parse_string(control, THEORY, std::strlen(THEORY)));
            auto cfg = Clingo::Control{control, true}.config();
            cfg.add("lpx", "Clingo.LPX configuration");
            for_each_option(theory->options, [&](OptionInfo const &info, auto config) -> void {
                // the configuration uses underscores instead of dashes
                auto &key = theory->strings.emplace_back("lpx.");
                key.append(info.name);
                std::ranges::replace(key, '-', '_');
                cfg.add(key, info.desc, std::move(config));
            });
        }

        CLINGO_CATCH;
    }

    static auto rewrite_ast([[maybe_unused]] void *self, clingo_ast_t *ast, clingo_theory_ast_callback_t add,
                            void *data) -> bool {
        return add(ast, data);
    }

    static auto prepare(void *self, clingo_control_t *control) -> bool {
        CLINGO_TRY {
            auto *theory = static_cast<clingolpx_theory *>(self);
            if (theory->clingolpx == nullptr) {
                if (!theory->options.strict) {
                    theory->clingolpx =
                        std::make_unique<LPXPropagatorFacade<Rational>>(theory->lib, control, theory->options);
                } else {
                    theory->clingolpx =
                        std::make_unique<LPXPropagatorFacade<RationalQ>>(theory->lib, control, theory->options);
                }
            }
        }
        CLINGO_CATCH;
    }

    static void destroy(void *self) {
        auto *theory = static_cast<clingolpx_theory *>(self);
        std::unique_ptr<clingolpx_theory>{theory};
    }

    static auto register_options(void *self, clingo_options_t *options) -> bool {
        CLINGO_TRY {
            using namespace std::string_view_literals;
            auto *theory = static_cast<clingolpx_theory *>(self);
            auto group = "Clingo.LPX Options"sv;
            auto opt = [&](std::string_view name, std::string_view desc, clingo_option_parser_t parser,
                           bool multi = false, std::string_view arg = {}) -> void {
                handle_error(clingo_options_add(options, group.data(), group.size(), name.data(), name.size(),
                                                desc.data(), desc.size(), parser, &theory->options, multi,
                                                arg.empty() ? nullptr : arg.data(), arg.size()));
            };
            auto flag = [&](std::string_view name, std::string_view desc, bool &target) -> void {
                handle_error(clingo_options_add_flag(options, group.data(), group.size(), name.data(), name.size(),
                                                     desc.data(), desc.size(), &target));
            };
            auto def = [&](std::string_view name, std::string_view value) -> void {
                handle_error(
                    clingo_options_set_default_value(options, name.data(), name.size(), value.data(), value.size()));
            };

            // the help shows the default values and the details of the arguments
            auto defaults = Options{};
            for_each_option(theory->options, [&](OptionInfo const &info, auto config) -> void {
                using Config = decltype(config);
                if constexpr (requires { config.target(); }) {
                    flag(info.name, info.desc, config.target());
                } else {
                    auto &desc = theory->strings.emplace_back(info.desc);
                    desc.append(" [");
                    desc.append(*Config{defaults}.get());
                    desc.append("]");
                    desc.append(Config::describe(info.arg));
                    opt(info.name, desc, c_parse<Config>, false, info.arg);
                }
            });

            def("out-assign", "__lpx/2");
            def("out-cost", "Objective:,__lpx_objective/2:%0 [%1]");
        }
        CLINGO_CATCH;
    }

    static auto validate_options(void *self) -> bool {
        CLINGO_TRY {
            auto *theory = static_cast<clingolpx_theory *>(self);
            if (!theory->options.strict && theory->options.global_objective.has_value() &&
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
            auto *theory = static_cast<clingolpx_theory *>(self);
            *found = theory->clingolpx->lookup_symbol(Clingo::Symbol{symbol, true}, *index);
        }
        CLINGO_CATCH;
    }

    static auto assignment_next(void *self, uint32_t thread_id, bool *init, size_t *index, bool *has_value) -> bool {
        CLINGO_TRY {
            auto *theory = static_cast<clingolpx_theory *>(self);
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
            auto *theory = static_cast<clingolpx_theory *>(self);
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
                    // NOLINTBEGIN(cppcoreguidelines-pro-type-union-access)
                    value->symbol = *c_cast(&sym);
                    clingo_symbol_acquire(value->symbol);
                    // NOLINTEND(cppcoreguidelines-pro-type-union-access)
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

extern "C" auto clingolpx_create(clingo_lib_t *lib, clingo_theory_t *theory) -> bool {
    CLINGO_TRY {
        *theory = clingo_theory_t{
            clingolpx_theory::info,
            clingolpx_theory::destroy,
            clingolpx_theory::register_,
            clingolpx_theory::rewrite_ast,
            clingolpx_theory::prepare,
            clingolpx_theory::register_options,
            clingolpx_theory::validate_options,
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
