// {{{ MIT License

// Copyright Roland Kaminski, Philipp Wanko, and Max Ostrowski

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

#ifndef CLINGOLPX_APP_HH
#define CLINGOLPX_APP_HH

#include <clingo-lpx.h>
#include <clingo.hh>
#include <optional>

namespace ClingoLPX {

//! Helper class to rewrite logic programs to use with the clingo DL theory.
class Rewriter {
  public:
    Rewriter(clingolpx_theory_t *theory, clingo_program_builder_t *builder);
    //! Rewrite the given files.
    void rewrite(Clingo::Control &control, Clingo::StringSpan files);
    //! Rewrite the given program.
    void rewrite(Clingo::Control &control, char const *str);

  private:
    //! C callback to add a statement using the builder.
    static auto add_(clingo_ast_t *stm, void *data) -> bool;

    //! C callback to rewrite a statement and add it via the builder.
    static auto rewrite_(clingo_ast_t *stm, void *data) -> bool;

    clingolpx_theory_t *theory_;        //!< A theory handle to rewrite statements.
    clingo_program_builder_t *builder_; //!< The builder to add rewritten statements to.
};

} // namespace ClingoLPX

#endif // CLINGOLPX_APP_HH
