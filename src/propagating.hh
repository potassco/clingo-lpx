#pragma once

#include <problem.hh>

class ClingoLPPropagator : public Clingo::Propagator {
public:
    ClingoLPPropagator() = default;
    ClingoLPPropagator(ClingoLPPropagator const &) = default;
    ClingoLPPropagator(ClingoLPPropagator &&) noexcept = default;
    ClingoLPPropagator &operator=(ClingoLPPropagator const &) = default;
    ClingoLPPropagator &operator=(ClingoLPPropagator &&) noexcept = default;
    void init(Clingo::PropagateInit &init) override;
    void propagate(Clingo::PropagateControl &ctl, Clingo::LiteralSpan changes) override;
    void undo(Clingo::PropagateControl const &ctl, Clingo::LiteralSpan changes) noexcept override;
    void check(Clingo::PropagateControl &ctl) override;
    ~ClingoLPPropagator() override = default;
private:
    void check_(Clingo::PropagateControl &ctl, bool print);
    std::vector<Inequality> iqs_;
};
