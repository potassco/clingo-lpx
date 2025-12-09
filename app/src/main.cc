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

#include <clingo/app.hh>
#include <clingo/theory.hh>

#ifdef CLINGOLPX_PROFILE
#include <gperftools/profiler.h>
#endif

#include <iostream>
#include <optional>

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

//! Application class to run clingo-lpx.
class App : public Clingo::App, private Clingo::SolveEventHandler {
  public:
    App(Clingo::Library const &lib) : lib_{lib} {}
    App(App &&other) = delete;
    //! Set program name to clingo-dl.
    auto do_program_name() noexcept -> std::string_view override { return "clingo-lpx"; }
    //! Set the version.
    auto do_version() noexcept -> std::string_view override { return CLINGOLPX_VERSION; }
    //! Pass models to the theory.
    auto do_model(Clingo::Model model) -> bool override {
        theory_.model(model);
        return true;
    }
    //! Pass statistics to the theory.
    void do_stats(Clingo::Stats step, Clingo::Stats accu) override { theory_.stats(step, accu); }

    //! Run main solving function.
    void do_main(Clingo::Control const &ctl, Clingo::StringSpan files) override { // NOLINT
        theory_.register_theory(ctl);
        theory_.rewrite(lib_, ctl, files);
        if (ctl.mode() == Clingo::ControlMode::solve) {
            ctl.ground();
            theory_.prepare(ctl);
#ifdef CLINGOLPX_PROFILE
            Profiler prof{"clingo-lpx-solve.prof"};
#endif
            std::ignore = ctl.solve({}, std::ref(*static_cast<SolveEventHandler *>(this)));
        } else {
            ctl.main();
        }
    }

    //! Register options of the theory and optimization related options.
    void do_register_options(Clingo::Options options) override { theory_.register_options(options); }
    //! Validate options of the theory.
    void do_validate_options() override { theory_.validate_options(); }

  private:
    Clingo::Library lib_;
    Clingo::Theory theory_{lib_, clingolpx_create};
};

} // namespace ClingoLPX

//! Run the clingo-dl application.
auto main(int argc, char *argv[]) -> int { // NOLINT(bugprone-exception-escape)
    Clingo::Library lib;
    ClingoLPX::App app{lib};
    auto args = std::vector<std::string_view>{argv + 1, argv + argc};
    return Clingo::main(lib, args, &app);
}
