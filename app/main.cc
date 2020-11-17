#include <parsing.hh>
#include <solving.hh>

#ifdef CLINGOLP_PROFILE
#include <gperftools/profiler.h>
#endif

#include <clingo.hh>
#include <sstream>
#include <variant>

#ifdef CLINGOLP_PROFILE

class Profiler {
public:
    Profiler(char const *path) {
        ProfilerStart(path);
    }
    ~Profiler() {
        ProfilerStop();
    }
};

#endif

template <class T>
constexpr bool is_mono() {
    return std::is_same_v<std::decay_t<T>, std::monostate>;
}

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
        std::visit([&](auto &&prp) {
            if constexpr (!is_mono<decltype(prp)>()) {
                for (auto const &[var, val] : prp.assignment(model.thread_id())) {
                    if (comma) {
                        last_assignment_ << " ";
                    }
                    else {
                        comma = true;
                    }
                    last_assignment_ << var << "=" << val;
                }
            }
        }, prp_);
        return true;
    }

    void on_statistics(Clingo::UserStatistics step, Clingo::UserStatistics accu) override {
        std::visit([&](auto &&prp) {
            if constexpr (!is_mono<decltype(prp)>()) {
                prp.on_statistics(step, accu);
            }
        }, prp_);
    }

    void register_options(Clingo::ClingoOptions &opts) override {
        opts.add_flag("Clingo.LP", "strict", "Enable support for strict constraints", strict_);
    }

    void main(Clingo::Control &ctl, Clingo::StringSpan files) override {
        if (strict_) {
            prp_ = ClingoLPPropagator<Number, NumberQ>{};
        }
        else {
            prp_ = ClingoLPPropagator<Number, Number>{};
        }
        std::visit([&ctl](auto &&prp) {
            if constexpr (!is_mono<decltype(prp)>()) {
                ctl.register_propagator(prp);
            }
        }, prp_);
        ctl.add("base", {}, THEORY);
        for (auto const &x : files) {
            ctl.load(x);
        }
        ctl.ground({{"base", {}}});
#ifdef CLINGOLP_PROFILE
        Profiler prof("profile.out");
#endif
        ctl.solve(Clingo::LiteralSpan{}, this, false, false).get();
    }

private:
    std::stringstream last_assignment_;
    std::variant<std::monostate, ClingoLPPropagator<Number, Number>, ClingoLPPropagator<Number, NumberQ>> prp_;
    bool strict_{false};
};

int main(int argc, char const *argv[]) {
    ClingoLPApp app;
    return Clingo::clingo_main(app, {argv+1, argv+argc});
}
