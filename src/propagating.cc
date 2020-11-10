#include <propagating.hh>

#include <parsing.hh>
#include <solving.hh>

void ClingoLPPropagator::check_(Clingo::PropagateControl &ctl, bool print) {
    std::vector<Inequality> iqs;
    std::vector<Clingo::literal_t> reason;
    for (auto &x : iqs_) {
        if (ctl.assignment().is_true(x.lit)) {
            iqs.emplace_back(x);
            reason.emplace_back(-x.lit);
        }
    }
    Solver slv{std::move(iqs)};
    if (slv.prepare()) {
        if (auto ret = slv.solve()) {
            if (print) {
                std::cerr << "assignment:";
                for (auto const &[var, val] : *ret) {
                    std::cerr << " " << var << "=" << val;
                }
                std::cerr << std::endl;
                std::cerr << "SAT with " << slv.statistics().pivots_ << " pivots" << std::endl;
            }
        }
        else {
            //std::cerr << "UNSAT with " << slv.statistics().pivots_ << " pivots" << std::endl;
            ctl.add_clause(reason);
        }
    }
    else {
        //std::cerr << "UNSAT" << std::endl;
        ctl.add_clause(reason);
    }
}

void ClingoLPPropagator::init(Clingo::PropagateInit &init) {
    iqs_ = evaluate_theory(init.theory_atoms());
    for (auto &x : iqs_) {
        x.lit = init.solver_literal(x.lit);
        init.add_watch(x.lit);
    }
}

void ClingoLPPropagator::propagate(Clingo::PropagateControl &ctl, Clingo::LiteralSpan changes) {
    check_(ctl, false);
}

void ClingoLPPropagator::undo(Clingo::PropagateControl const &ctl, Clingo::LiteralSpan changes) noexcept {
}

void ClingoLPPropagator::check(Clingo::PropagateControl &ctl) {
    check_(ctl, true);
}
