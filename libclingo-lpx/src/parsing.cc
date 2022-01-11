#include "parsing.hh"

#include <map>
#include <regex>

namespace {

template <typename T=void>
[[nodiscard]] T throw_syntax_error(char const *message="Invalid Syntax") {
    throw std::runtime_error(message);
}

void check_syntax(bool condition, char const *message="Invalid Syntax") {
    if (!condition) {
        throw_syntax_error(message);
    }
}

[[nodiscard]] bool match(Clingo::TheoryTerm const &term, char const *name, size_t arity) {
    return (term.type() == Clingo::TheoryTermType::Symbol &&
        std::strcmp(term.name(), name) == 0 &&
        arity == 0) ||
        (term.type() == Clingo::TheoryTermType::Function &&
        std::strcmp(term.name(), name) == 0 &&
        term.arguments().size() == arity);
}

bool is_string(Clingo::TheoryTerm const &term) {
    if (term.type() != Clingo::TheoryTermType::Symbol) {
        return false;
    }
    char const *name = term.name();
    size_t len = std::strlen(term.name());
    return len >= 2 && name[0] == '"' && name[len - 1] == '"';
}

[[nodiscard]] Clingo::Symbol evaluate(Clingo::TheoryTerm const &term) {
    if (is_string(term)) {
        char const *name = term.name();
        size_t len = std::strlen(term.name());
        return Clingo::String(std::string{name + 1, name + len - 1}.c_str());
    }

    if (term.type() == Clingo::TheoryTermType::Symbol) {
        return Clingo::Function(term.name(), {});
    }

    if (term.type() == Clingo::TheoryTermType::Number) {
        return Clingo::Number(term.number());
    }

    if (match(term, "-", 1)) {
        auto arg = evaluate(term.arguments().back());
        if (arg.type() == Clingo::SymbolType::Number) {
            return Clingo::Number(-arg.number());
        }
        if (arg.type() == Clingo::SymbolType::Function) {
            return Clingo::Function(arg.name(), arg.arguments(), !arg.is_positive());
        }
        return throw_syntax_error<Clingo::Symbol>();
    }

    check_syntax(!match(term, "..", 2) && !match(term, "*", 2) && !match(term, "/", 2));

    if (term.type() == Clingo::TheoryTermType::Tuple || term.type() == Clingo::TheoryTermType::Function) {
        std::vector<Clingo::Symbol> args;
        args.reserve(term.arguments().size());
        for (auto const &arg : term.arguments()) {
            args.emplace_back(evaluate(arg));
        }
        return Clingo::Function(term.type() == Clingo::TheoryTermType::Function ? term.name() : "", args);
    }

    return throw_syntax_error<Clingo::Symbol>();
}

[[nodiscard]] Clingo::Symbol evaluate_var(Clingo::TheoryTerm const &term) {
    check_syntax(
        !match(term, "-", 1) &&
        !match(term, "..", 2) &&
        !match(term, "*", 2) &&
        !match(term, "/", 2));
    check_syntax(
        term.type() == Clingo::TheoryTermType::Tuple ||
        term.type() == Clingo::TheoryTermType::Function ||
        term.type() == Clingo::TheoryTermType::Symbol);

    return evaluate(term);
}

[[nodiscard]] Number evaluate_num(Clingo::TheoryTerm const &term) {
    if (is_string(term)) {
        auto const *name = term.name();
        std::regex const rgx{"(-)?([0-9]+)(\\.([0-9]+))?"};
        std::cmatch match;
        check_syntax(std::regex_match(name + 1, name + strlen(name) - 1, match, rgx));
        std::string a = match[2];
        if (match[4].matched) {
            auto const *ib = match[4].first;
            auto const *it = match[4].second;
            for (; it != ib && *(it - 1) == '0'; --it) { }
            a.append(ib, it);
            a.append("/1");
            a.append(it - ib, '0');
        }
        Number n{a, 10};
        if (match[1].matched) {
            n *= -1;
        }
        n.canonicalize();
        return n;
    }

    if (term.type() == Clingo::TheoryTermType::Number) {
        return {term.number()};
    }

    if (match(term, "-", 1)) {
        return -evaluate_num(term.arguments().front());
    }
    if (match(term, "*", 2)) {
        return evaluate_num(term.arguments().front()) * evaluate_num(term.arguments().back());
    }
    if (match(term, "/", 2)) {
        return evaluate_num(term.arguments().front()) / evaluate_num(term.arguments().back());
    }

    return throw_syntax_error<Number>();
}

[[nodiscard]] Relation evaluate_cmp(char const *rel) {
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

} // namespace

void evaluate_theory(Clingo::TheoryAtoms const &theory, LitMapper const &mapper, VarMap &var_map, std::vector<Inequality> &iqs) {
    for (auto &&atom : theory) {
        if (match(atom.term(), "dom", 0)) {
            check_syntax(atom.elements().size() == 1);
            auto &&elem = atom.elements().front();
            check_syntax(elem.tuple().size() == 1 && elem.condition().empty());
            auto &&term = elem.tuple().front();
            check_syntax(match(term, "..", 2));
            auto var = evaluate_var(atom.guard().second);
            auto lit = mapper(atom.literal());
            iqs.emplace_back(Inequality{{{1, var}}, evaluate_num(term.arguments().back()), Relation::LessEqual, lit});
            iqs.emplace_back(Inequality{{{1, var}}, evaluate_num(term.arguments().front()), Relation::GreaterEqual, lit});
        }
        else if (match(atom.term(), "sum", 0)) {
            std::vector<Term> lhs;
            for (auto &&elem : atom.elements()) {
                check_syntax(elem.tuple().size() == 1);
                auto &&term = elem.tuple().front();
                if (match(term, "-", 1)) {
                    lhs.emplace_back(Term{
                        -1,
                        evaluate_var(term.arguments().back())});
                }
                else if (match(term, "*", 2)) {
                    lhs.emplace_back(Term{
                        evaluate_num(term.arguments().front()),
                        evaluate_var(term.arguments().back())});
                }
                else {
                    lhs.emplace_back(Term{1, evaluate_var(term)});
                }
                if (!elem.condition().empty()) {
                    auto res = var_map.try_emplace(std::make_pair(lhs.back().var, elem.condition_id()), Clingo::Number(var_map.size()));
                    if (res.second) {
                        auto lit = mapper(elem.condition_id());
                        iqs.emplace_back(Inequality{{{1, res.first->second}}, 0, Relation::Equal, -lit});
                        iqs.emplace_back(Inequality{{{1, res.first->second}, {-1, lhs.back().var}}, 0, Relation::Equal, lit});
                    }
                    lhs.back().var = res.first->second;
                }
            }
            auto lit = mapper(atom.literal());
            iqs.emplace_back(Inequality{std::move(lhs),
                                        evaluate_num(atom.guard().second),
                                        evaluate_cmp(atom.guard().first),
                                        lit});
        }
    }
}
