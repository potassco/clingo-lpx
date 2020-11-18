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

class Application : public Clingo::Application, public Clingo::SolveEventHandler {
public:
    Application() = default;
    Application(Application const &) = delete;
    Application(Application &&) = delete;
    Application &operator=(Application &&) = delete;
    Application &operator=(Application const &) = delete;
    ~Application() override = default;

    [[nodiscard]] char const *program_name() const noexcept override {
        return "clingo-lpx";
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
        opts.add_flag("Clingo.LPX", "strict", "Enable support for strict constraints", strict_);
    }

    void main(Clingo::Control &ctl, Clingo::StringSpan files) override {
        if (strict_) {
            prp_ = Propagator<Number, NumberQ>{};
        }
        else {
            prp_ = Propagator<Number, Number>{};
        }
        std::visit([&ctl](auto &&prp) {
            if constexpr (!is_mono<decltype(prp)>()) {
                prp.register_control(ctl);
            }
        }, prp_);
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
    std::variant<std::monostate, Propagator<Number, Number>, Propagator<Number, NumberQ>> prp_;
    bool strict_{false};
};

int main(int argc, char const *argv[]) {
    Application app;
    return Clingo::clingo_main(app, {argv+1, argv+argc});
}
