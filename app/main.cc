#include <parsing.hh>
#include <solving.hh>

#include <clingo.hh>

class ClingoLPApp : public Clingo::Application {
public:
    ClingoLPApp() = default;
    ClingoLPApp(ClingoLPApp const &) = delete;
    ClingoLPApp(ClingoLPApp &&) = delete;
    ClingoLPApp &operator=(ClingoLPApp &&) = delete;
    ClingoLPApp &operator=(ClingoLPApp const &) = delete;
    ~ClingoLPApp() override = default;

    [[nodiscard]] char const *program_name() const noexcept override {
        return "clingo-lp";
    }
    [[nodiscard]] char const *version() const noexcept override {
        return "1.0.0";
    }
    void main(Clingo::Control &ctl, Clingo::StringSpan files) override {
        ctl.add("base", {}, THEORY);
        for (auto const &x : files) {
            ctl.load(x);
        }
        ctl.ground({{"base", {}}});
        Solver slv{evaluate_theory(ctl.theory_atoms())};
        ctl.solve();
        if (slv.prepare()) {
            auto ret = slv.solve();
            if (ret) {
                std::cerr << "assignment:";
                for (auto const &[var, val] : *ret) {
                    std::cerr << " " << var << "=" << val;
                }
                std::cerr << std::endl;
                std::cerr << "SAT with " << slv.statistics().pivots_ << " pivots" << std::endl;
            }
            else {
                std::cerr << "UNSAT with " << slv.statistics().pivots_ << " pivots" << std::endl;
            }
        }
        else {
            std::cerr << "UNSAT in prepare" << std::endl;
        }
    }
};

int main(int argc, char const *argv[]) {
    ClingoLPApp app;
    return Clingo::clingo_main(app, {argv+1, argv+argc});
}
