// {{{ MIT License

// Copyright Roland Kaminski

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.

// }}}

#include <clingo-lpx.h>
#include <optional>
#ifdef CLINGOLPX_PROFILE
#include <gperftools/profiler.h>
#endif

namespace ClingoLPX {

#ifdef CLINGOLPX_PROFILE

namespace {

class Profiler {
  public:
    Profiler(char const *path) { ProfilerStart(path); }
    ~Profiler() { ProfilerStop(); }
};

} // namespace

#endif

using Clingo::Detail::handle_error;

//! Helper class to rewrite logic programs to use with the clingo DL theory.
class Rewriter {
  public:
    Rewriter(clingolpx_theory_t *theory, clingo_program_builder_t *builder) : theory_{theory}, builder_{builder} {}

    //! Rewrite the given files.
    void rewrite(Clingo::Control &control, Clingo::StringSpan files) {
        Clingo::Detail::handle_error(
            clingo_ast_parse_files(files.begin(), files.size(), rewrite_, this, control.to_c(), nullptr, nullptr, 0));
    }

    //! Rewrite the given program.
    void rewrite(Clingo::Control &control, char const *str) {
        Clingo::Detail::handle_error(clingo_ast_parse_string(str, rewrite_, this, control.to_c(), nullptr, nullptr, 0));
    }

  private:
    //! C callback to add a statement using the builder.
    static auto add_(clingo_ast_t *stm, void *data) -> bool {
        auto *self = static_cast<Rewriter *>(data);
        return clingo_program_builder_add(self->builder_, stm);
    }

    //! C callback to rewrite a statement and add it via the builder.
    static auto rewrite_(clingo_ast_t *stm, void *data) -> bool {
        auto *self = static_cast<Rewriter *>(data);
        return clingolpx_rewrite_ast(self->theory_, stm, add_, self);
    }

    clingolpx_theory_t *theory_;        //!< A theory handle to rewrite statements.
    clingo_program_builder_t *builder_; //!< The builder to add rewritten statements to.
};

//! Application class to run clingo-lpx.
class App : public Clingo::Application, private Clingo::SolveEventHandler {
  public:
    App() { handle_error(clingolpx_create(&theory_)); }
    App(App const &) = default;
    App(App &&) = default;
    auto operator=(App const &) -> App & = default;
    auto operator=(App &&) -> App & = default;
    ~App() override { clingolpx_destroy(theory_); }
    //! Set program name to clingo-lpx.
    [[nodiscard]] auto program_name() const noexcept -> char const * override { return "clingo-lpx"; }
    //! Set the version.
    [[nodiscard]] auto version() const noexcept -> char const * override { return CLINGOLPX_VERSION; }
    void print_model(Clingo::Model const &model, std::function<void()> default_printer) noexcept override {
        static_cast<void>(default_printer);
        try {
            auto symbols = model.symbols();
            std::sort(symbols.begin(), symbols.end());
            bool comma = false;
            for (auto const &sym : symbols) {
                if (comma) {
                    std::cout << " ";
                }
                std::cout << sym;
                comma = true;
            }
            std::cout << "\nAssignment:\n";
            symbols = model.symbols(Clingo::ShowType::Theory);
            std::sort(symbols.begin(), symbols.end());
            comma = false;
            std::optional<std::pair<Clingo::Symbol, bool>> objective;
            for (auto const &sym : symbols) {
                if (sym.match("__lpx", 2) && sym.arguments().back().type() == Clingo::SymbolType::String) {
                    if (comma) {
                        std::cout << " ";
                    }
                    auto args = sym.arguments();
                    std::cout << args.front() << "=" << args.back().string();
                    comma = true;
                } else if (sym.match("__lpx_objective", 2) &&
                           sym.arguments().front().type() == Clingo::SymbolType::String &&
                           sym.arguments().back().type() == Clingo::SymbolType::Number) {
                    auto args = sym.arguments();
                    objective = std::make_pair(args.front(), args.back() == Clingo::Number(1));
                }
            }
            if (objective.has_value()) {
                std::cout << "\nOptimization: " << objective->first.string() << " ["
                          << (objective->second ? "bounded" : "unbounded") << "]";
            }
            std::cout << std::endl;
        } catch (...) {
        }
    }
    //! Pass models to the theory.
    auto on_model(Clingo::Model &model) -> bool override {
        handle_error(clingolpx_on_model(theory_, model.to_c()));
        return true;
    }
    //! Pass statistics to the theory.
    void on_statistics(Clingo::UserStatistics step, Clingo::UserStatistics accu) override {
        handle_error(clingolpx_on_statistics(theory_, step.to_c(), accu.to_c()));
    }
    //! Run main solving function.
    void main(Clingo::Control &ctl, Clingo::StringSpan files) override { // NOLINT(bugprone-exception-escape)
        handle_error(clingolpx_register(theory_, ctl.to_c()));

        Clingo::AST::with_builder(ctl, [&](Clingo::AST::ProgramBuilder &builder) {
            Rewriter rewriter{theory_, builder.to_c()};
            rewriter.rewrite(ctl, files);
        });

        ctl.ground({{"base", {}}});
#ifdef CLINGOLPX_PROFILE
        Profiler prof{"clingo-lpx-solve.prof"};
#endif
        ctl.solve(Clingo::SymbolicLiteralSpan{}, this, false, false).get();
    }
    //! Register options of the theory and optimization related options.
    void register_options(Clingo::ClingoOptions &options) override {
        handle_error(clingolpx_register_options(theory_, options.to_c()));
    }
    //! Validate options of the theory.
    void validate_options() override { handle_error(clingolpx_validate_options(theory_)); }

  private:
    clingolpx_theory_t *theory_{nullptr}; //!< The underlying DL theory.
};

} // namespace ClingoLPX

//! Run the clingo-lpx application.
auto main(int argc, char *argv[]) -> int { // NOLINT(bugprone-exception-escape)
    ClingoLPX::App app;
    return Clingo::clingo_main(app, {argv + 1, static_cast<size_t>(argc - 1)});
}
