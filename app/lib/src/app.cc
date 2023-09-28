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

#include <clingo-lpx-app/app.hh>

#include <clingo.hh>
#include <cmath>
#include <iostream>
#include <limits>

namespace ClingoLPX {

Rewriter::Rewriter(clingolpx_theory_t *theory, clingo_program_builder_t *builder)
    : theory_{theory}, builder_{builder} {}

void Rewriter::rewrite(Clingo::Control &control, Clingo::StringSpan files) {
    Clingo::Detail::handle_error(
        clingo_ast_parse_files(files.begin(), files.size(), rewrite_, this, control.to_c(), nullptr, nullptr, 0));
}

void Rewriter::rewrite(Clingo::Control &control, char const *str) {
    Clingo::Detail::handle_error(clingo_ast_parse_string(str, rewrite_, this, control.to_c(), nullptr, nullptr, 0));
}

auto Rewriter::add_(clingo_ast_t *stm, void *data) -> bool {
    auto *self = static_cast<Rewriter *>(data);
    return clingo_program_builder_add(self->builder_, stm);
}

auto Rewriter::rewrite_(clingo_ast_t *stm, void *data) -> bool {
    auto *self = static_cast<Rewriter *>(data);
    return clingolpx_rewrite_ast(self->theory_, stm, add_, self);
}

} // namespace ClingoLPX
