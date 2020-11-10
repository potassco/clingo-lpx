#include <parsing.hh>
#include <propagating.hh>

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
        ClingoLPPropagator prp;
        ctl.register_propagator(prp);
        ctl.add("base", {}, THEORY);
        for (auto const &x : files) {
            ctl.load(x);
        }
        ctl.ground({{"base", {}}});
        ctl.solve(Clingo::LiteralSpan{}, nullptr, false, false).get();
    }
};

int main(int argc, char const *argv[]) {
    ClingoLPApp app;
    return Clingo::clingo_main(app, {argv+1, argv+argc});
}
