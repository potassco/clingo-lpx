#include <parsing.hh>
#include <solving.hh>

#include <clingo.hh>
#include <sstream>

class ClingoLPApp : public Clingo::Application, public Clingo::SolveEventHandler {
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

    void print_model(Clingo::Model const &model, std::function<void()> default_printer) noexcept override {
        default_printer();
        std::cout << last_assignment_.str() << std::endl;
    }

    bool on_model(Clingo::Model &model) override {
        last_assignment_.str("");
        last_assignment_ << "Assignment:\n";
        bool comma = false;
        for (auto const &[var, val] : prp_.assignment(model.thread_id())) {
            if (comma) {
                last_assignment_ << " ";
            }
            else {
                comma = true;
            }
            last_assignment_ << var << "=" << val;
        }
        return true;
    }

    void on_statistics(Clingo::UserStatistics step, Clingo::UserStatistics accu) override {
        prp_.on_statistics(step, accu);
    }

    void main(Clingo::Control &ctl, Clingo::StringSpan files) override {
        ctl.register_propagator(prp_);
        ctl.add("base", {}, THEORY);
        for (auto const &x : files) {
            ctl.load(x);
        }
        ctl.ground({{"base", {}}});
        ctl.solve(Clingo::LiteralSpan{}, this, false, false).get();
    }

private:
    std::stringstream last_assignment_;
    ClingoLPPropagator prp_;
};

int main(int argc, char const *argv[]) {
    ClingoLPApp app;
    return Clingo::clingo_main(app, {argv+1, argv+argc});
}
