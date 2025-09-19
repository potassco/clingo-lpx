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

#include <clingo/theory.h>

//! Major version number.
#define CLINGOLPX_VERSION_MAJOR 1
//! Minor version number.
#define CLINGOLPX_VERSION_MINOR 4
//! Revision number.
#define CLINGOLPX_VERSION_REVISION 0
//! String representation of version.
#define CLINGOLPX_VERSION "1.4.0"

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

CLINGOLPX_VISIBILITY_DEFAULT bool clingolpx_create(clingo_lib_t *lib, clingo_theory_t *theory);

// NOLINTEND(modernize-use-using,modernize-use-trailing-return-type)

#ifdef __cplusplus
}
#endif

#endif
