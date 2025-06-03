// {{{ MIT License
//
// Copyright Roland Kaminski
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//
// }}}

#ifndef CLINGOLPX_H
#define CLINGOLPX_H

#include <clingo.h>

//! Major version number.
#define CLINGOLPX_VERSION_MAJOR 1
//! Minor version number.
#define CLINGOLPX_VERSION_MINOR 3
//! Revision number.
#define CLINGOLPX_VERSION_REVISION 0
//! String representation of version.
#define CLINGOLPX_VERSION "1.3.0"

#ifdef __cplusplus
extern "C" {
#endif

#if defined _WIN32 || defined __CYGWIN__
#define CLINGOLPX_WIN
#endif
#ifdef CLINGOLPX_NO_VISIBILITY
#define CLINGOLPX_VISIBILITY_DEFAULT
#define CLINGOLPX_VISIBILITY_PRIVATE
#else
#ifdef CLINGOLPX_WIN
#ifdef CLINGOLPX_BUILD_LIBRARY
#define CLINGOLPX_VISIBILITY_DEFAULT __declspec(dllexport)
#else
#define CLINGOLPX_VISIBILITY_DEFAULT __declspec(dllimport)
#endif
#define CLINGOLPX_VISIBILITY_PRIVATE
#else
#if __GNUC__ >= 4
#define CLINGOLPX_VISIBILITY_DEFAULT __attribute__((visibility("default")))
#define CLINGOLPX_VISIBILITY_PRIVATE __attribute__((visibility("hidden")))
#else
#define CLINGOLPX_VISIBILITY_DEFAULT
#define CLINGOLPX_VISIBILITY_PRIVATE
#endif
#endif
#endif

// NOLINTBEGIN(modernize-use-using,modernize-use-trailing-return-type)

enum clingolpx_value_type {
    clingolpx_value_type_int = 0,
    clingolpx_value_type_double = 1,
    clingolpx_value_type_symbol = 2
};
typedef int clingolpx_value_type_t;

typedef struct clingolpx_value {
    clingolpx_value_type_t type;
    union {
        int int_number;
        double double_number;
        clingo_symbol_t symbol;
    };
} clingolpx_value_t;

//! Callback to rewrite statements (see ::clingolpx_rewrite_ast).
typedef bool (*clingolpx_ast_callback_t)(clingo_ast_t *ast, void *data);

typedef struct clingolpx_theory clingolpx_theory_t;

//! Return the version of the theory.
CLINGOLPX_VISIBILITY_DEFAULT void clingolpx_version(int *major, int *minor, int *patch);

//! creates the theory
CLINGOLPX_VISIBILITY_DEFAULT bool clingolpx_create(clingolpx_theory_t **theory);

//! registers the theory with the control
CLINGOLPX_VISIBILITY_DEFAULT bool clingolpx_register(clingolpx_theory_t *theory, clingo_control_t *control);

//! Rewrite asts before adding them via the given callback.
CLINGOLPX_VISIBILITY_DEFAULT bool clingolpx_rewrite_ast(clingolpx_theory_t *theory, clingo_ast_t *ast,
                                                        clingolpx_ast_callback_t add, void *data);

//! prepare the theory between grounding and solving
CLINGOLPX_VISIBILITY_DEFAULT bool clingolpx_prepare(clingolpx_theory_t *theory, clingo_control_t *control);

//! destroys the theory, currently no way to unregister a theory
CLINGOLPX_VISIBILITY_DEFAULT bool clingolpx_destroy(clingolpx_theory_t *theory);

//! configure theory manually (without using clingo's options facility)
//! Note that the theory has to be configured before registering it and cannot be reconfigured.
CLINGOLPX_VISIBILITY_DEFAULT bool clingolpx_configure(clingolpx_theory_t *theory, char const *key, char const *value);

//! add options for your theory
CLINGOLPX_VISIBILITY_DEFAULT bool clingolpx_register_options(clingolpx_theory_t *theory, clingo_options_t *options);

//! validate options for your theory
CLINGOLPX_VISIBILITY_DEFAULT bool clingolpx_validate_options(clingolpx_theory_t *theory);

//! callback on every model
CLINGOLPX_VISIBILITY_DEFAULT bool clingolpx_on_model(clingolpx_theory_t *theory, clingo_model_t *model);

//! obtain a symbol index which can be used to get the value of a symbol
//! returns true if the symbol exists
//! does not throw
CLINGOLPX_VISIBILITY_DEFAULT bool clingolpx_lookup_symbol(clingolpx_theory_t *theory, clingo_symbol_t symbol,
                                                          size_t *index);

//! obtain the symbol at the given index
//! does not throw
CLINGOLPX_VISIBILITY_DEFAULT clingo_symbol_t clingolpx_get_symbol(clingolpx_theory_t *theory, size_t index);

//! initialize index so that it can be used with clingolpx_assignment_next
//! does not throw
CLINGOLPX_VISIBILITY_DEFAULT void clingolpx_assignment_begin(clingolpx_theory_t *theory, uint32_t thread_id,
                                                             size_t *index);

//! move to the next index that has a value
//! returns true if the updated index is valid
//! does not throw
CLINGOLPX_VISIBILITY_DEFAULT bool clingolpx_assignment_next(clingolpx_theory_t *theory, uint32_t thread_id,
                                                            size_t *index);

//! check if the symbol at the given index has a value
//! does not throw
CLINGOLPX_VISIBILITY_DEFAULT bool clingolpx_assignment_has_value(clingolpx_theory_t *theory, uint32_t thread_id,
                                                                 size_t index);

//! get the symbol and it's value at the given index
//! does not throw
CLINGOLPX_VISIBILITY_DEFAULT void clingolpx_assignment_get_value(clingolpx_theory_t *theory, uint32_t thread_id,
                                                                 size_t index, clingolpx_value_t *value);

//! callback on statistic updates
/// please add a subkey with the name of your theory
CLINGOLPX_VISIBILITY_DEFAULT bool clingolpx_on_statistics(clingolpx_theory_t *theory, clingo_statistics_t *step,
                                                          clingo_statistics_t *accu);

// NOLINTEND(modernize-use-using,modernize-use-trailing-return-type)

#ifdef __cplusplus
}
#endif

#endif
