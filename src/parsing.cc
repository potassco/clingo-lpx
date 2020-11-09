#include <parsing.hh>

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

[[nodiscard]] Clingo::Symbol evaluate(Clingo::TheoryTerm const &term) {
    if (term.type() == Clingo::TheoryTermType::Symbol) {
        return Clingo::Function(term.name(), {});
    }

    if (term.type() == Clingo::TheoryTermType::Number) {
        return Clingo::Number(term.number());
    }

    check_syntax(!match(term, "-", 1) && !match(term, "*", 2) && !match(term, "/", 2));

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
        term.type() == Clingo::TheoryTermType::Tuple ||
        term.type() == Clingo::TheoryTermType::Function ||
        term.type() == Clingo::TheoryTermType::Symbol);

    return evaluate(term);
}

[[nodiscard]] Number evaluate_num(Clingo::TheoryTerm const &term) {
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
    return throw_syntax_error<Relation>();
}

} // namespace

[[nodiscard]] std::vector<Inequality> evaluate_theory(Clingo::TheoryAtoms const &theory) {
    std::vector<Inequality> iqs;
    for (auto const &atom : theory) {
        if (match(atom.term(), "sum", 0)) {
            std::vector<Term> lhs;
            for (auto const &elem : atom.elements()) {
                check_syntax(elem.tuple().size() == 1 && elem.condition().empty());
                auto const &term = elem.tuple().front();
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
            }
            iqs.emplace_back(Inequality{std::move(lhs),
                                      evaluate_num(atom.guard().second),
                                      evaluate_cmp(atom.guard().first)});
        }
    }
    return iqs;
}
