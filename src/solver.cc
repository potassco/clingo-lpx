#include <catch.hpp>
#include <clingo.hh>
#include <gmpxx.h>
#include <cassert>

enum class Operator {
    LessEqual = 0,
    GreaterEqual = 1,
    Equal = 2,
};

std::ostream &operator<<(std::ostream &out, Operator const &op) {
    switch (op) {
        case Operator::LessEqual: {
            out << "<=";
            break;
        }
        case Operator::GreaterEqual: {
            out << ">=";
            break;
        }
        case Operator::Equal: {
            out << "=";
            break;
        }
    }
    return out;
}

using Number = mpq_class;

struct Term {
    Number co;
    Clingo::Symbol var;
};

std::ostream &operator<<(std::ostream &out, Term const &term) {
    if (term.co == -1) {
        out << "-";
    }
    else if (term.co != 1) {
        out << term.co << "*";
    }
    out << term.var;
    return out;
}

struct Equation {
    std::vector<Term> lhs;
    Number rhs;
    Operator op;
};

std::ostream &operator<<(std::ostream &out, Equation const &eq) {
    bool plus{false};
    for (auto const &term : eq.lhs) {
        if (plus) {
            out << " + ";
        }
        else {
            plus = true;
        }
        out << term;
    }
    if (eq.lhs.empty()) {
        out << "0";
    }
    out << " " << eq.op << " " << eq.rhs;
    return out;
}

char const *THEORY = R"(
#theory lp {
    sum_term {
    -  : 3, unary;
    *  : 1, binary, left;
    /  : 1, binary, left
    };
    &sum/0 : sum_term, {<=,=,>=}, sum_term, any
}.
)";

template <typename T=void>
[[nodiscard]] T throw_syntax_error(char const *message="Invalid Syntax") {
    assert(false);
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

    check_syntax(!match(term, "-", 1) && !match(term, "*", 2) && !!match(term, "/", 2));

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

[[nodiscard]] Operator evaluate_cmp(char const *op) {
    if (std::strcmp(op, "<=") == 0) {
        return Operator::LessEqual;
    }
    if (std::strcmp(op, ">=") == 0) {
        return Operator::GreaterEqual;
    }
    if (std::strcmp(op, "=") == 0) {
        return Operator::Equal;
    }
    return throw_syntax_error<Operator>();
}

[[nodiscard]] std::vector<Equation> evaluate_theory(Clingo::TheoryAtoms const &theory) {
    std::vector<Equation> eqs;
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
            eqs.emplace_back(Equation{std::move(lhs), evaluate_num(atom.guard().second), evaluate_cmp(atom.guard().first)});
        }
    }
    return eqs;
}

TEST_CASE("theory") {
    Clingo::Control ctl;
    ctl.add("base", {}, THEORY);

    SECTION("example 1") {
        ctl.add("base", {},
            "&sum { x1; x2 } <= 20.\n"
            "&sum { x1; x3 } = 5.\n"
            "&sum { x2; x3 } >= 10.\n");
        ctl.ground({{"base", {}}});

        for (auto const &eq : evaluate_theory(ctl.theory_atoms())) {
            std::cerr << eq << std::endl;
        }
    }

    SECTION("example 2") {
        ctl.add("base", {},
            "&sum { x } >= 2.\n"
            "&sum { x } <= 0.\n");
        ctl.ground({{"base", {}}});

        for (auto const &eq : evaluate_theory(ctl.theory_atoms())) {
            std::cerr << eq << std::endl;
        }
    }

    SECTION("example 3") {
        ctl.add("base", {},
            "&sum {   x;   y } >= 2.\n"
            "&sum { 2*x;  -y } >= 0.\n"
            "&sum {  -x; 2*y } >= 1.\n"
            );
        ctl.ground({{"base", {}}});

        for (auto const &eq : evaluate_theory(ctl.theory_atoms())) {
            std::cerr << eq << std::endl;
        }
    }
};

