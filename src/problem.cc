#include <problem.hh>

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
