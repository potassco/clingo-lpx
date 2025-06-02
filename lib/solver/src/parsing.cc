#include "parsing.hh"
#include "util.hh"

#include <clingo.hh>

#include <algorithm>
#include <iterator>
#include <optional>
#include <regex>
#include <stdexcept>
#include <unordered_map>
#include <variant>

namespace {

#define MATCHES(a, b) std::is_same_v<std::remove_reference_t<std::remove_cv_t<decltype(a)>>, b>
#define FWD(a) std::forward<decltype(a)>(a)

template <typename T = void> [[nodiscard]] auto throw_syntax_error(char const *message = "Invalid Syntax") -> T {
    throw std::runtime_error(message);
}

void check_syntax(bool condition, char const *message = "Invalid Syntax") {
    if (!condition) {
        throw_syntax_error(message);
    }
}

[[nodiscard]] auto match(Clingo::TheoryTerm const &term, char const *name, size_t arity) -> bool {
    return (term.type() == Clingo::TheoryTermType::Symbol && std::strcmp(term.name(), name) == 0 && arity == 0) ||
           (term.type() == Clingo::TheoryTermType::Function && std::strcmp(term.name(), name) == 0 &&
            term.arguments().size() == arity);
}

[[nodiscard]] auto is_string(Clingo::TheoryTerm const &term) -> bool {
    if (term.type() != Clingo::TheoryTermType::Symbol) {
        return false;
    }
    char const *name = term.name();
    size_t len = std::strlen(term.name());
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    return len >= 2 && name[0] == '"' && name[len - 1] == '"';
}

[[nodiscard]] auto is_invalid(Clingo::Symbol const &sym) -> bool {
    return sym.type() == Clingo::SymbolType::Number && sym.number() == 0;
}

[[nodiscard]] auto evaluate_cmp(char const *rel) -> Relation {
    if (std::strcmp(rel, "<=") == 0) {
        return Relation::LessEqual;
    }
    if (std::strcmp(rel, ">=") == 0) {
        return Relation::GreaterEqual;
    }
    if (std::strcmp(rel, "=") == 0) {
        return Relation::Equal;
    }
    if (std::strcmp(rel, ">") == 0) {
        return Relation::Greater;
    }
    if (std::strcmp(rel, "<") == 0) {
        return Relation::Less;
    }
    return throw_syntax_error<Relation>();
}

template <class F> struct EB {
    EB(F &f) : f{f} {}
    auto operator()(Rational &&a, Rational &&b) -> Rational { return f(std::move(a), std::move(b)); }
    template <class A, class B> auto operator()([[maybe_unused]] A &&a, [[maybe_unused]] B &&b) -> Rational {
        return throw_syntax_error<Rational>();
    }
    F &f;
};

struct EU {
    auto operator()(Rational &&a) -> std::variant<Rational, Clingo::Symbol> { return -std::move(a); }
    auto operator()(Clingo::Symbol &&a) -> std::variant<Rational, Clingo::Symbol> {
        if (a.type() == Clingo::SymbolType::Function && std::strlen(a.name()) > 0) {
            return Clingo::Function(a.name(), a.arguments(), !a.is_positive());
        }
        return throw_syntax_error<Clingo::Symbol>();
    }
};

struct AS {
    auto operator()(Rational &&a) -> Clingo::Symbol {
        if (auto v = a.num().as_int(); a.den() == 1 && v) {
            return Clingo::Number(*v);
        }
        return throw_syntax_error<Clingo::Symbol>();
    }
    auto operator()(Clingo::Symbol &&a) -> Clingo::Symbol { return std::move(a); }
};

[[nodiscard]] auto as_sym(std::variant<Rational, Clingo::Symbol> &&x) -> Clingo::Symbol {
    return std::visit(AS{}, std::move(x));
}

[[nodiscard]] auto as_num(char const *name) -> std::optional<Rational> {
    size_t len = std::strlen(name);
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    if (len <= 2 || name[0] != '"' || name[len - 1] != '"') {
        return std::nullopt;
    }
    std::regex const rgx{"(-)?([0-9]+)(\\.([0-9]+))?"};
    std::cmatch match;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    if (!std::regex_match(name + 1, name + len - 1, match, rgx)) {
        return std::nullopt;
    }
    std::string a = match[2];
    if (match[4].matched) {
        auto const *ib = match[4].first;
        auto const *it = match[4].second;
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        for (; it != ib && *(it - 1) == '0'; --it) {
        }
        a.append(ib, it);
        a.append("/1");
        a.append(it - ib, '0');
    }
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
    Rational n{a, 10};
    if (match[1].matched) {
        n.neg();
    }
    n.canonicalize();
    return n;
}

[[nodiscard]] auto unquote(Clingo::Span<char> str) -> std::string {
    std::string res;
    bool slash = false;
    for (auto c : str) {
        if (slash) {
            switch (c) {
                case 'n': {
                    res.push_back('\n');
                    break;
                }
                case '\\': {
                    res.push_back('\\');
                    break;
                }
                case '"': {
                    res.push_back('"');
                    break;
                }
                default: {
                    assert(false);
                    break;
                }
            }
            slash = false;
        } else if (c == '\\') {
            slash = true;
        } else {
            res.push_back(c);
        }
    }
    return res;
}

[[nodiscard]] auto evaluate(bool num_str, Clingo::TheoryTerm const &term) -> std::variant<Rational, Clingo::Symbol>;

template <class F>
[[nodiscard]] auto evaluate(bool num_str, Clingo::TheoryTerm const &a, Clingo::TheoryTerm const &b, F f) -> Rational {
    return std::visit(EB{f}, evaluate(num_str, a), evaluate(num_str, b));
}

auto evaluate(bool num_str, Clingo::TheoryTerm const &term) -> std::variant<Rational, Clingo::Symbol> {
    if (term.type() == Clingo::TheoryTermType::Symbol) {
        const auto *cname = term.name();
        Clingo::Span<char> name{cname, std::strlen(cname)};
        if (is_string(term)) {
            if (num_str) {
                if (auto res = as_num(cname); res) {
                    return *std::move(res);
                }
            }
            return Clingo::String(unquote({name.begin() + 1, name.end() - 1}).c_str());
        }
        return Clingo::Function(cname, {});
    }

    if (term.type() == Clingo::TheoryTermType::Number) {
        return Rational(term.number());
    }

    if (match(term, "+", 2)) {
        return evaluate(num_str, term.arguments().front(), term.arguments().back(),
                        [](Rational &&a, Rational &&b) { return std::move(a) + std::move(b); });
    }
    if (match(term, "-", 2)) {
        return evaluate(num_str, term.arguments().front(), term.arguments().back(),
                        [](Rational &&a, Rational &&b) { return std::move(a) - std::move(b); });
    }
    if (match(term, "*", 2)) {
        return evaluate(num_str, term.arguments().front(), term.arguments().back(),
                        [](Rational &&a, Rational &&b) { return std::move(a) * std::move(b); });
    }
    if (match(term, "/", 2)) {
        return evaluate(num_str, term.arguments().front(), term.arguments().back(),
                        [](Rational &&a, Rational &&b) { return std::move(a) / std::move(b); });
    }

    if (match(term, "-", 1)) {
        return std::visit(EU{}, evaluate(num_str, term.arguments().front()));
    }

    check_syntax(!match(term, "..", 2));

    if (term.type() == Clingo::TheoryTermType::Tuple || term.type() == Clingo::TheoryTermType::Function) {
        std::vector<Clingo::Symbol> args;
        args.reserve(term.arguments().size());
        for (auto const &arg : term.arguments()) {
            args.emplace_back(as_sym(evaluate(false, arg)));
        }
        return Clingo::Function(term.type() == Clingo::TheoryTermType::Function ? term.name() : "", args);
    }

    return throw_syntax_error<Clingo::Symbol>();
}

struct EvaluateVar {
    auto operator()([[maybe_unused]] Rational &&a) -> Clingo::Symbol { return throw_syntax_error<Clingo::Symbol>(); }
    auto operator()(Clingo::Symbol &&a) -> Clingo::Symbol { return std::move(a); }
};

[[nodiscard]] auto evaluate_var(Clingo::TheoryTerm const &term) -> Clingo::Symbol {
    return std::visit(EvaluateVar{}, evaluate(false, term));
}

struct EvaluateNum {
    auto operator()(Rational &&a) -> Rational { return std::move(a); }
    auto operator()([[maybe_unused]] Clingo::Symbol &&a) -> Rational { return throw_syntax_error<Rational>(); }
};

[[nodiscard]] auto evaluate_num(Clingo::TheoryTerm const &term) -> Rational {
    return std::visit(EvaluateNum{}, evaluate(true, term));
}

auto parse_diff_elem(Clingo::TheoryTerm const &term) -> std::vector<Term> {
    std::vector<Term> res;
    if (match(term, "-", 2)) {
        auto args = term.arguments();
        std::visit(
            [&res](auto &&a) {
                if constexpr (MATCHES(a, Rational)) {
                    res.emplace_back(Term{FWD(a), Clingo::Number(0)});
                }
                if constexpr (MATCHES(a, Clingo::Symbol)) {
                    res.emplace_back(Term{Rational(1), FWD(a)});
                }
            },
            evaluate(true, args.front()));
        std::visit(
            [&res](auto &&a) {
                if constexpr (MATCHES(a, Rational)) {
                    res.emplace_back(Term{-FWD(a), Clingo::Number(0)});
                }
                if constexpr (MATCHES(a, Clingo::Symbol)) {
                    res.emplace_back(Term{Rational(-1), FWD(a)});
                }
            },
            evaluate(true, args.back()));
    } else {
        throw_syntax_error("Invalid Syntax: invalid difference constraint");
    }
    return res;
}

void parse_sum_elem(Clingo::TheoryTerm const &term, std::vector<Term> &res) {
    if (term.type() == Clingo::TheoryTermType::Number) {
        res.emplace_back(Term{FWD(term.number()), Clingo::Number(0)});
    } else if (match(term, "+", 2)) {
        auto args = term.arguments();
        parse_sum_elem(args.front(), res);
        parse_sum_elem(args.back(), res);
    } else if (match(term, "-", 2)) {
        auto args = term.arguments();
        parse_sum_elem(args.front(), res);
        auto pos = res.size();
        parse_sum_elem(args.back(), res);
        for (auto it = res.begin() + pos, ie = res.end(); it != ie; ++it) {
            it->co = -it->co;
        }
    } else if (match(term, "-", 1)) {
        auto pos = res.size();
        parse_sum_elem(term.arguments().front(), res);
        for (auto it = res.begin() + pos, ie = res.end(); it != ie; ++it) {
            it->co = -it->co;
        }
    } else if (match(term, "+", 1)) {
        parse_sum_elem(term.arguments().front(), res);
    } else if (match(term, "*", 2)) {
        auto args = term.arguments();
        std::vector<Term> lhs;
        std::vector<Term> rhs;
        parse_sum_elem(args.front(), lhs);
        parse_sum_elem(args.back(), rhs);
        for (auto &[l_co, l_var] : lhs) {
            for (auto &[r_co, r_var] : rhs) {
                check_syntax(is_invalid(l_var) || is_invalid(r_var));
                auto var = is_invalid(l_var) ? r_var : l_var;
                res.emplace_back(Term{l_co * r_co, var});
            }
        }
    } else if (match(term, "/", 2)) {
        auto args = term.arguments();
        std::vector<Term> lhs;
        parse_sum_elem(args.front(), lhs);
        auto rhs = evaluate_num(args.back());
        check_syntax(rhs != 0);
        for (auto &[co, var] : lhs) {
            res.emplace_back(Term{std::move(co) / rhs, var});
        }
    } else if (is_string(term)) {
        if (auto num = as_num(term.name())) {
            res.emplace_back(Term{*num, Clingo::Number(0)});
        } else {
            res.emplace_back(Term{1, evaluate_var(term)});
        }
    } else if (term.type() == Clingo::TheoryTermType::Symbol || term.type() == Clingo::TheoryTermType::Function ||
               term.type() == Clingo::TheoryTermType::Tuple) {
        res.emplace_back(Term{1, evaluate_var(term)});
    } else {
        throw_syntax_error("Invalid Syntax: invalid sum constraint");
    }
}

[[nodiscard]] auto parse_sum_elems(LitMapper const &mapper, VarMap &var_map, std::vector<Inequality> &iqs,
                                   Clingo::TheoryElementSpan elements) -> std::vector<Term> {
    std::vector<Term> lhs;
    for (auto &&elem : elements) {
        check_syntax(elem.tuple().size() == 1);
        auto &&term = elem.tuple().front();
        size_t n = lhs.size();
        parse_sum_elem(term, lhs);
        if (!elem.condition().empty()) {
            for (auto it = lhs.begin() + n, ie = lhs.end(); it != ie; ++it) {
                auto res = var_map.try_emplace(std::make_pair(it->var, elem.condition_id()),
                                               Clingo::Number(safe_cast<int>(var_map.size() + 1)));
                if (res.second) {
                    auto lit = mapper(elem.condition_id());
                    iqs.emplace_back(Inequality{{{1, res.first->second}}, 0, Relation::Equal, -lit});
                    iqs.emplace_back(Inequality{{{1, res.first->second}, {-1, it->var}}, 0, Relation::Equal, lit});
                }
                it->var = res.first->second;
            }
        }
    }
    return lhs;
}

auto simplify(std::unordered_map<Clingo::Symbol, Term &> &cos, std::vector<Term> &terms) -> Rational {
    auto ib = terms.begin();
    auto ie = terms.end();

    auto rhs = Rational{0};
    // combine cofficients
    cos.clear();
    std::for_each(ib, ie, [&cos, &rhs](Term &term) {
        if (is_invalid(term.var)) {
            rhs -= std::move(term.co);
            term.co = 0;
        } else if (auto [jt, res] = cos.emplace(term.var, term); !res) {
            jt->second.co += std::move(term.co);
            term.co = 0;
        }
    });

    // remove terms with zero coeffcients
    terms.erase(std::remove_if(ib, ie, [](Term const &term) { return term.co == 0; }), ie);

    return rhs;
}

void parse_theory(Clingo::TheoryAtoms const &theory, LitMapper const &mapper, VarMap &var_map,
                  std::vector<Inequality> &iqs, std::vector<Term> &objective) {
    std::unordered_map<Clingo::Symbol, Term &> cos;
    for (auto &&atom : theory) {
        if (match(atom.term(), "dom", 0)) {
            check_syntax(atom.elements().size() == 1);
            auto &&elem = atom.elements().front();
            check_syntax(elem.tuple().size() == 1 && elem.condition().empty());
            auto &&term = elem.tuple().front();
            check_syntax(atom.has_guard(), "&dom constraints need guards");
            check_syntax(match(term, "..", 2), "interval in &dom constraint expected");
            auto var = evaluate_var(atom.guard().second);
            auto lit = mapper(atom.literal());
            iqs.emplace_back(Inequality{{{1, var}}, evaluate_num(term.arguments().back()), Relation::LessEqual, lit});
            iqs.emplace_back(
                Inequality{{{1, var}}, evaluate_num(term.arguments().front()), Relation::GreaterEqual, lit});
        } else if (match(atom.term(), "diff", 0)) {
            check_syntax(atom.has_guard() && atom.elements().size() == 1 &&
                             atom.elements().front().tuple().size() == 1 && atom.elements().front().condition().empty(),
                         "&diff invalid difference constraint");
            auto lhs = parse_diff_elem(atom.elements().begin()->tuple().front());
            size_t n = lhs.size();
            parse_sum_elem(atom.guard().second, lhs);
            for (auto it = lhs.begin() + n, ie = lhs.end(); it != ie; ++it) {
                it->co.neg();
            }
            auto rhs = simplify(cos, lhs);
            auto lit = mapper(atom.literal());
            iqs.emplace_back(Inequality{std::move(lhs), std::move(rhs), evaluate_cmp(atom.guard().first), lit});
        } else if (match(atom.term(), "sum", 0)) {
            check_syntax(atom.has_guard(), "&sum constraints need guards");
            auto lhs = parse_sum_elems(mapper, var_map, iqs, atom.elements());
            size_t n = lhs.size();
            parse_sum_elem(atom.guard().second, lhs);
            for (auto it = lhs.begin() + n, ie = lhs.end(); it != ie; ++it) {
                it->co.neg();
            }
            auto rhs = simplify(cos, lhs);
            auto lit = mapper(atom.literal());
            iqs.emplace_back(Inequality{std::move(lhs), std::move(rhs), evaluate_cmp(atom.guard().first), lit});
        } else if (match(atom.term(), "minimize", 0) || match(atom.term(), "maximize", 0)) {
            auto lhs = parse_sum_elems(mapper, var_map, iqs, atom.elements());
            if (match(atom.term(), "minimize", 0)) {
                for (auto &term : lhs) {
                    term.co.neg();
                }
            }
            std::move(lhs.begin(), lhs.end(), std::back_inserter(objective));
        }
    }
    auto rhs = simplify(cos, objective);
    check_syntax(rhs == 0, "objective must not contain constants");
}

} // namespace

void evaluate_theory(Clingo::TheoryAtoms const &theory, LitMapper const &mapper, VarMap &var_map,
                     std::vector<Inequality> &iqs, std::vector<Term> &objective) {
    parse_theory(theory, mapper, var_map, iqs, objective);
}
