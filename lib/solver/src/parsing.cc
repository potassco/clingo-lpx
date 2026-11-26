#include <algorithm>
#include <clingo-lpx/parsing.hh>
#include <clingo-lpx/util.hh>

#include <clingo/core.hh>
#include <clingo/symbol.hh>

#include <iterator>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <variant>

namespace ClingoLPX {

namespace {

constexpr auto dec_radix = 10;

template <typename T = void> [[nodiscard]] auto throw_syntax_error(char const *message = "Invalid Syntax") -> T {
    throw std::runtime_error(message);
}

void check_syntax(bool condition, char const *message = "Invalid Syntax") {
    if (!condition) {
        throw_syntax_error(message);
    }
}

[[nodiscard]] auto match(Clingo::TheoryTerm const &term, std::string_view name, size_t arity) -> bool {
    return (term.type() == Clingo::TheoryTermType::symbol && term.name() == name && arity == 0) ||
           (term.type() == Clingo::TheoryTermType::function && term.name() == name && term.arguments().size() == arity);
}

[[nodiscard]] auto is_string(std::string_view name) -> bool {
    return name.size() >= 2 && name.starts_with('"') && name.ends_with('"');
}

[[nodiscard]] auto is_string(Clingo::TheoryTerm const &term) -> bool {
    return term.type() == Clingo::TheoryTermType::symbol && is_string(term.name());
}

[[nodiscard]] auto evaluate_cmp(std::string_view rel) -> Relation {
    if (rel == "<=") {
        return Relation::LessEqual;
    }
    if (rel == ">=") {
        return Relation::GreaterEqual;
    }
    if (rel == "=") {
        return Relation::Equal;
    }
    if (rel == ">") {
        return Relation::Greater;
    }
    if (rel == "<") {
        return Relation::Less;
    }
    return throw_syntax_error<Relation>();
}

template <class F> struct EB {
    explicit EB(F &f) : f{f} {}
    auto operator()(Rational &&a, Rational &&b) -> Rational { return f(std::move(a), std::move(b)); }
    template <class A, class B> auto operator()([[maybe_unused]] A &&a, [[maybe_unused]] B &&b) -> Rational {
        return throw_syntax_error<Rational>();
    }
    F &f;
};

struct EU {
    auto operator()(Rational &&a) -> std::variant<Rational, Clingo::Symbol> { return -std::move(a); }
    auto operator()(Clingo::Symbol const &a) -> std::variant<Rational, Clingo::Symbol> {
        if (a.type() == Clingo::SymbolType::function) {
            return Clingo::Function(lib, a.name(), a.arguments(), !a.is_positive());
        }
        return throw_syntax_error<Clingo::Symbol>();
    }
    Clingo::Library const &lib;
};

struct AS {
    auto operator()(Rational const &a) -> Clingo::Symbol {
        if (auto v = a.num().as_int(); a.den() == 1 && v) {
            return Clingo::Number(*v);
        }
        return throw_syntax_error<Clingo::Symbol>();
    }
    auto operator()(Clingo::Symbol a) -> Clingo::Symbol { return a; }
};

[[nodiscard]] auto as_sym(std::variant<Rational, Clingo::Symbol> &&x) -> Clingo::Symbol {
    return std::visit(AS{}, std::move(x));
}

[[nodiscard]] auto as_num(std::string_view name) -> std::optional<Rational> {
    assert(is_string(name));
    name = name.substr(1, name.size() - 2);
    bool negative = name.starts_with('-');
    if (negative) {
        name = name.substr(1);
    }
    auto dot = name.find('.');
    auto lhs = (dot != std::string_view::npos) ? name.substr(0, dot) : name;
    auto rhs = (dot != std::string_view::npos) ? name.substr(dot + 1) : "0";
    auto is_digit = [](char c) -> bool { return c >= '0' && c <= '9'; };
    if (lhs.empty() || rhs.empty() || !std::ranges::all_of(lhs, is_digit) || !std::ranges::all_of(rhs, is_digit)) {
        return std::nullopt;
    }
    std::string a = {lhs.begin(), lhs.end()};
    if (auto dot = rhs.find_last_not_of('0'); dot != std::string_view::npos) {
        rhs = rhs.substr(0, dot + 1);
    }
    if (!rhs.empty()) {
        a.append(rhs.begin(), rhs.end());
        a += "/1";
        a.append(rhs.size(), '0');
    }
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
    Rational n{a, dec_radix};
    if (negative) {
        n.neg();
    }
    n.canonicalize();
    return n;
}

[[nodiscard]] inline auto unquote(std::string_view str) -> std::string {
    auto res = std::string{};
    auto slash = false;
    if (is_string(str)) {
        str = str.substr(1, str.size() - 2);
    }
    for (char it : str) {
        if (slash) {
            switch (it) {
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
        } else if (it == '\\') {
            slash = true;
        } else {
            res.push_back(it);
        }
    }
    return res;
}

[[nodiscard]] auto evaluate(Clingo::Library const &lib, bool num_str, Clingo::TheoryTerm const &term)
    -> std::variant<Rational, Clingo::Symbol>;

template <class F>
[[nodiscard]] auto evaluate(Clingo::Library const &lib, bool num_str, Clingo::TheoryTerm const &a,
                            Clingo::TheoryTerm const &b, F f) -> Rational {
    return std::visit(EB{f}, evaluate(lib, num_str, a), evaluate(lib, num_str, b));
}

auto evaluate(Clingo::Library const &lib, bool num_str, Clingo::TheoryTerm const &term)
    -> std::variant<Rational, Clingo::Symbol> {
    if (term.type() == Clingo::TheoryTermType::symbol) {
        auto name = term.name();
        if (is_string(term)) {
            if (num_str) {
                if (auto res = as_num(name); res) {
                    return *std::move(res);
                }
            }
            return Clingo::String(lib, unquote(name));
        }
        return Clingo::Function(lib, name, {});
    }

    if (term.type() == Clingo::TheoryTermType::number) {
        return Rational(term.number());
    }

    if (match(term, "+", 2)) {
        return evaluate(lib, num_str, term.arguments().front(), term.arguments().back(),
                        [](Rational &&a, Rational const &b) -> Rational { return std::move(a) + b; });
    }
    if (match(term, "-", 2)) {
        return evaluate(lib, num_str, term.arguments().front(), term.arguments().back(),
                        [](Rational &&a, Rational const &b) -> Rational { return std::move(a) - b; });
    }
    if (match(term, "*", 2)) {
        return evaluate(lib, num_str, term.arguments().front(), term.arguments().back(),
                        [](Rational &&a, Rational const &b) -> Rational { return std::move(a) * b; });
    }
    if (match(term, "/", 2)) {
        return evaluate(lib, num_str, term.arguments().front(), term.arguments().back(),
                        [](Rational &&a, Rational const &b) -> Rational { return std::move(a) / b; });
    }

    if (match(term, "-", 1)) {
        return std::visit(EU{lib}, evaluate(lib, num_str, term.arguments().front()));
    }

    check_syntax(!match(term, "..", 2));

    if (term.type() == Clingo::TheoryTermType::tuple || term.type() == Clingo::TheoryTermType::function) {
        auto args = std::vector<Clingo::Symbol>{};
        args.reserve(term.arguments().size());
        for (auto const &arg : term.arguments()) {
            args.emplace_back(as_sym(evaluate(lib, false, arg)));
        }
        return term.type() == Clingo::TheoryTermType::function ? Clingo::Function(lib, term.name(), args)
                                                               : Clingo::Tuple(lib, args);
    }

    return throw_syntax_error<Clingo::Symbol>();
}

struct EvaluateVar {
    auto operator()([[maybe_unused]] Rational const &a) -> Clingo::Symbol {
        return throw_syntax_error<Clingo::Symbol>();
    }
    auto operator()(Clingo::Symbol a) -> Clingo::Symbol { return a; }
};

[[nodiscard]] auto evaluate_var(Clingo::Library const &lib, Clingo::TheoryTerm const &term) -> Clingo::Symbol {
    return std::visit(EvaluateVar{}, evaluate(lib, false, term));
}

struct EvaluateNum {
    auto operator()(Rational &&a) -> Rational { return std::move(a); }
    auto operator()([[maybe_unused]] Clingo::Symbol &&a) -> Rational { return throw_syntax_error<Rational>(); }
};

[[nodiscard]] auto evaluate_num(Clingo::Library const &lib, Clingo::TheoryTerm const &term) -> Rational {
    return std::visit(EvaluateNum{}, evaluate(lib, true, term));
}

//! Negate the coefficients of the terms starting at the given position.
void negate_from(std::vector<Term> &terms, size_t pos) {
    for (auto it = terms.begin() + static_cast<ptrdiff_t>(pos), ie = terms.end(); it != ie; ++it) {
        it->co.neg();
    }
}

//! Add a number or a variable with coefficient 1 or -1 to the terms.
void add_term(std::vector<Term> &terms, std::variant<Rational, Clingo::Symbol> x, bool negative) {
    if (auto *num = std::get_if<Rational>(&x)) {
        if (negative) {
            num->neg();
        }
        terms.emplace_back(Term{std::move(*num), constant_symbol()});
    } else {
        terms.emplace_back(Term{Rational{negative ? -1 : 1}, std::get<Clingo::Symbol>(x)});
    }
}

auto parse_diff_elem(Clingo::Library const &lib, Clingo::TheoryTerm const &term) -> std::vector<Term> {
    check_syntax(match(term, "-", 2), "Invalid Syntax: invalid difference constraint");
    std::vector<Term> res;
    auto args = term.arguments();
    add_term(res, evaluate(lib, true, args.front()), false);
    add_term(res, evaluate(lib, true, args.back()), true);
    return res;
}

void parse_sum_elem(Clingo::Library const &lib, Clingo::TheoryTerm const &term, std::vector<Term> &res) {
    if (term.type() == Clingo::TheoryTermType::number) {
        res.emplace_back(Term{term.number(), constant_symbol()});
    } else if (match(term, "+", 2)) {
        auto args = term.arguments();
        parse_sum_elem(lib, args.front(), res);
        parse_sum_elem(lib, args.back(), res);
    } else if (match(term, "-", 2)) {
        auto args = term.arguments();
        parse_sum_elem(lib, args.front(), res);
        auto pos = res.size();
        parse_sum_elem(lib, args.back(), res);
        negate_from(res, pos);
    } else if (match(term, "-", 1)) {
        auto pos = res.size();
        parse_sum_elem(lib, term.arguments().front(), res);
        negate_from(res, pos);
    } else if (match(term, "+", 1)) {
        parse_sum_elem(lib, term.arguments().front(), res);
    } else if (match(term, "*", 2)) {
        auto args = term.arguments();
        std::vector<Term> lhs;
        std::vector<Term> rhs;
        parse_sum_elem(lib, args.front(), lhs);
        parse_sum_elem(lib, args.back(), rhs);
        for (auto &[l_co, l_var] : lhs) {
            for (auto &[r_co, r_var] : rhs) {
                check_syntax(is_constant(l_var) || is_constant(r_var));
                auto var = is_constant(l_var) ? r_var : l_var;
                res.emplace_back(Term{l_co * r_co, var});
            }
        }
    } else if (match(term, "/", 2)) {
        auto args = term.arguments();
        std::vector<Term> lhs;
        parse_sum_elem(lib, args.front(), lhs);
        auto rhs = evaluate_num(lib, args.back());
        check_syntax(rhs != 0);
        for (auto &[co, var] : lhs) {
            res.emplace_back(Term{std::move(co) / rhs, var});
        }
    } else if (is_string(term)) {
        if (auto num = as_num(term.name())) {
            res.emplace_back(Term{*num, constant_symbol()});
        } else {
            res.emplace_back(Term{1, evaluate_var(lib, term)});
        }
    } else if (term.type() == Clingo::TheoryTermType::symbol || term.type() == Clingo::TheoryTermType::function ||
               term.type() == Clingo::TheoryTermType::tuple) {
        res.emplace_back(Term{1, evaluate_var(lib, term)});
    } else {
        throw_syntax_error("Invalid Syntax: invalid sum constraint");
    }
}

[[nodiscard]] auto parse_sum_elems(Clingo::Library const &lib, LitMapper const &mapper, VarMap &var_map,
                                   std::vector<Inequality> &iqs, std::span<Clingo::TheoryElement const> elements)
    -> std::vector<Term> {
    std::vector<Term> lhs;
    for (auto &&elem : elements) {
        auto tup = elem.tuple();
        check_syntax(tup.size() == 1);
        auto term = tup.front();
        size_t n = lhs.size();
        parse_sum_elem(lib, term, lhs);
        if (auto cond_id = elem.condition_id(); cond_id) {
            auto cond_lit = mapper(cond_id);
            for (auto it = lhs.begin() + static_cast<ptrdiff_t>(n), ie = lhs.end(); it != ie; ++it) {
                auto res = var_map.try_emplace(std::make_pair(it->var, cond_lit),
                                               Clingo::Number(safe_cast<int>(var_map.size() + 1)));
                if (res.second) {
                    iqs.emplace_back(Inequality{{{1, res.first->second}}, 0, Relation::Equal, -cond_lit});
                    if (!is_constant(it->var)) {
                        iqs.emplace_back(
                            Inequality{{{1, res.first->second}, {-1, it->var}}, 0, Relation::Equal, cond_lit});
                    } else {
                        iqs.emplace_back(Inequality{{{1, res.first->second}}, 1, Relation::Equal, cond_lit});
                    }
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
    std::for_each(ib, ie, [&cos, &rhs](Term &term) -> void {
        if (is_constant(term.var)) {
            rhs -= term.co;
            term.co = 0;
        } else if (auto [jt, res] = cos.emplace(term.var, term); !res) {
            jt->second.co += term.co;
            term.co = 0;
        }
    });

    // remove terms with zero coeffcients
    terms.erase(std::remove_if(ib, ie, [](Term const &term) -> bool { return term.co == 0; }), ie);

    return rhs;
}

[[nodiscard]] auto negate_cmp(std::string_view rel) -> std::string_view {
    if (rel == "<=") {
        return ">";
    }
    if (rel == ">=") {
        return "<";
    }
    if (rel == "<") {
        return ">=";
    }
    if (rel == ">") {
        return "<=";
    }
    if (rel == "=") {
        return "!=";
    }
    if (rel == "!=") {
        return "=";
    }
    return throw_syntax_error<std::string_view>();
}

//! Add the constraint lhs rel rhs implied by lit to the inequalities.
void add_constraint(LitSplitter const &split, std::vector<Inequality> &iqs, std::vector<Term> lhs, Rational rhs,
                    std::string_view rel, Clingo::ProgramLiteral lit) {
    if (rel == "!=") {
        check_syntax(split != nullptr, "relation != not enabled (try option --rewrite-constraints)");
        auto [lt, gt] = split(lit);
        iqs.emplace_back(Inequality{lhs, rhs, Relation::Less, lt});
        iqs.emplace_back(Inequality{std::move(lhs), std::move(rhs), Relation::Greater, gt});
    } else {
        iqs.emplace_back(Inequality{std::move(lhs), std::move(rhs), evaluate_cmp(rel), lit});
    }
}

//! Add the inequality lhs rel rhs to the inequalities.
//!
//! The terms of the right-hand side are moved to the left and the constants
//! to the right. Constraints in bodies are equivalent to their literal.
void add_inequality(Clingo::Library const &lib, std::unordered_map<Clingo::Symbol, Term &> &cos,
                    LitSplitter const &split, std::vector<Inequality> &iqs, std::vector<Term> lhs, std::string_view rel,
                    Clingo::TheoryTerm const &rhs, Clingo::ProgramLiteral lit, bool body) {
    auto pos = lhs.size();
    parse_sum_elem(lib, rhs, lhs);
    negate_from(lhs, pos);
    auto bound = simplify(cos, lhs);
    if (body) {
        check_syntax(split != nullptr, "constraints in rule bodies not enabled (try option --rewrite-constraints)");
        add_constraint(split, iqs, lhs, bound, negate_cmp(rel), -lit);
    }
    add_constraint(split, iqs, std::move(lhs), std::move(bound), rel, lit);
}

// NOLINTBEGIN(bugprone-unchecked-optional-access)

void parse_theory(Clingo::Library const &lib, Clingo::TheoryBase const &theory, LitMapper const &mapper,
                  LitSplitter const &split, VarMap &var_map, std::vector<Inequality> &iqs, std::vector<Term> &objective,
                  std::vector<Clingo::Symbol> &integers) {
    std::unordered_map<Clingo::Symbol, Term &> cos;
    for (auto &&atom : theory) {
        auto name = atom.name();
        if (match(name, "dom", 0)) {
            auto elems = atom.elements();
            check_syntax(elems.size() == 1);
            auto elem = elems.front();
            auto tuple = elem.tuple();
            check_syntax(tuple.size() == 1 && elem.condition().empty());
            auto term = tuple.front();
            auto guard = atom.guard();
            check_syntax(guard.has_value(), "&dom constraints need guards");
            check_syntax(match(term, "..", 2), "interval in &dom constraint expected");
            auto var = evaluate_var(lib, guard->second);
            auto lit = mapper(atom.literal());
            auto args = term.arguments();
            iqs.emplace_back(Inequality{{{1, var}}, evaluate_num(lib, args.back()), Relation::LessEqual, lit});
            iqs.emplace_back(Inequality{{{1, var}}, evaluate_num(lib, args.front()), Relation::GreaterEqual, lit});
        } else if (match(name, "diff", 0) || match(name, "__diff_b", 0)) {
            auto guard = atom.guard();
            auto elems = atom.elements();
            check_syntax(guard.has_value() && elems.size() == 1, "&diff invalid difference constraint");
            auto tuple = elems.front().tuple();
            check_syntax(tuple.size() == 1 && elems.front().condition().empty(), "&diff invalid difference constraint");
            add_inequality(lib, cos, split, iqs, parse_diff_elem(lib, tuple.front()), guard->first, guard->second,
                           mapper(atom.literal()), match(name, "__diff_b", 0));
        } else if (match(name, "sum", 0) || match(name, "__sum_b", 0)) {
            auto guard = atom.guard();
            check_syntax(guard.has_value(), "&sum constraints need guards");
            auto lhs = parse_sum_elems(lib, mapper, var_map, iqs, atom.elements());
            add_inequality(lib, cos, split, iqs, std::move(lhs), guard->first, guard->second, mapper(atom.literal()),
                           match(name, "__sum_b", 0));
        } else if (match(name, "minimize", 0) || match(name, "maximize", 0)) {
            auto lhs = parse_sum_elems(lib, mapper, var_map, iqs, atom.elements());
            if (match(name, "minimize", 0)) {
                negate_from(lhs, 0);
            }
            std::ranges::move(lhs, std::back_inserter(objective));
        } else if (match(name, "int", 0)) {
            for (auto &&elem : atom.elements()) {
                auto tuple = elem.tuple();
                check_syntax(tuple.size() == 1 && elem.condition().empty(), "&int expects variables");
                integers.emplace_back(evaluate_var(lib, tuple.front()));
            }
        }
    }
    auto rhs = simplify(cos, objective);
    check_syntax(rhs == 0, "objective must not contain constants");
}

// NOLINTEND(bugprone-unchecked-optional-access)

} // namespace

void evaluate_theory(Clingo::Library const &lib, Clingo::TheoryBase const &theory, LitMapper const &mapper,
                     LitSplitter const &split, VarMap &var_map, std::vector<Inequality> &iqs,
                     std::vector<Term> &objective, std::vector<Clingo::Symbol> &integers) {
    parse_theory(lib, theory, mapper, split, var_map, iqs, objective, integers);
}

void rewrite(Clingo::Library const &lib, Clingo::AST::Node const &ast, NodeCallback const &cb) {
    using namespace Clingo::AST;
    auto tagged = [](std::string_view name) -> std::optional<std::string> {
        if (name == "sum" || name == "diff") {
            return "__" + std::string{name} + "_b";
        }
        return std::nullopt;
    };
    auto tag = [&](Node const &name) -> std::optional<Node> {
        if (name.type() == NodeType::term_symbolic) {
            auto sym = name.symbol(Attribute::symbol);
            if (sym.type() != Clingo::SymbolType::function || !sym.arguments().empty()) {
                return std::nullopt;
            }
            if (auto res = tagged(sym.name()); res.has_value()) {
                return Node::create<NodeType::term_symbolic>(lib, name.location(Attribute::location),
                                                             Clingo::Function(lib, *res, {}));
            }
        }
        if (name.type() == NodeType::term_function && name.number(Attribute::external) == 0) {
            auto pool = name.nodes(Attribute::pool);
            if (auto res = tagged(name.string(Attribute::name));
                res.has_value() && pool.size() == 1 && pool.front().nodes(Attribute::arguments).empty()) {
                return name.update<NodeType::term_function>(lib, [&]<Attribute Attr>() -> auto {
                    if constexpr (Attr == Attribute::name) {
                        return *res;
                    }
                });
            }
        }
        return std::nullopt;
    };
    Transformer trans = [&](Node const &node) -> std::optional<Node> {
        if (node.type() == NodeType::body_theory_atom) {
            if (auto name = tag(node.node(Attribute::name)); name.has_value()) {
                return node.update<NodeType::body_theory_atom>(lib, [&]<Attribute Attr>() -> auto {
                    if constexpr (Attr == Attribute::name) {
                        return *name;
                    }
                });
            }
            return std::nullopt;
        }
        return node.accept(lib, trans);
    };
    auto res = trans(ast);
    cb(res.has_value() ? *res : ast);
}

} // namespace ClingoLPX
