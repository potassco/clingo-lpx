#include "problem.hh"

Relation invert(Relation rel) {
    switch (rel) {
        case Relation::LessEqual: {
            return Relation::GreaterEqual;
        }
        case Relation::GreaterEqual: {
            return Relation::LessEqual;
        }
        case Relation::Equal: {
            break;
        }
        case Relation::Less: {
            return Relation::Greater;
        }
        case Relation::Greater: {
            return Relation::Less;
        }
    }
    return Relation::Equal;
}

std::ostream &operator<<(std::ostream &out, Relation const &rel) {
    switch (rel) {
        case Relation::LessEqual: {
            out << "<=";
            break;
        }
        case Relation::GreaterEqual: {
            out << ">=";
            break;
        }
        case Relation::Equal: {
            out << "=";
            break;
        }
        case Relation::Less: {
            out << "<";
            break;
        }
        case Relation::Greater: {
            out << ">";
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

std::ostream &operator<<(std::ostream &out, Inequality const &x) {
    bool plus{false};
    for (auto const &term : x.lhs) {
        if (plus) {
            out << " + ";
        }
        else {
            plus = true;
        }
        out << term;
    }
    if (x.lhs.empty()) {
        out << "0";
    }
    out << " " << x.rel << " " << x.rhs;
    return out;
}
