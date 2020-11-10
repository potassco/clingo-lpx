#include <propagating.hh>

#include <parsing.hh>
#include <solving.hh>

void ClingoLPPropagator::init(Clingo::PropagateInit &init) {
    if (!slv.prepare(init, evaluate_theory(init.theory_atoms()))) {
        return;
    }
}

void ClingoLPPropagator::propagate(Clingo::PropagateControl &ctl, Clingo::LiteralSpan changes) {
    std::cerr << "propagate???" << std::endl;
    for (auto &lit: changes) {
        std::cerr << "  lit: " << lit << std::endl;
    }
    if (!slv.solve(ctl, changes)) {
        std::cerr << "sorry but unsat!!!" << std::endl;
        ctl.add_clause(slv.reason());
    }
    else {
        std::cerr << "we are sat!!!" << std::endl;
    }
}

void ClingoLPPropagator::undo(Clingo::PropagateControl const &ctl, Clingo::LiteralSpan changes) noexcept {
    slv.undo();
}

void ClingoLPPropagator::check(Clingo::PropagateControl &ctl) {
    std::cerr << "assignment:";
    for (auto const &[var, val] : slv.assignment()) {
        std::cerr << " " << var << "=" << val;
    }
    std::cerr << std::endl;
    std::cerr << "SAT with " << slv.statistics().pivots_ << " pivots" << std::endl;
}
