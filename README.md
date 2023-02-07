# clingo modulo simplex

A simplistic simplex solver for checking satisfiability of a set of equations.

## Examples

There are some encodings for different systems in the `examples` folder:

```
$ clingo-lpx -c n=132 encoding-lp.lp tai4_4_1.lp
clingo-lpx version 1.1.0
Reading from encoding-lp.lp ...
Solving...
Answer: 1
[...]
Assignment:
(1,1)=132 (1,2)=0  (1,3)=71  (1,4)=69  \
(2,1)=19  (2,2)=98 (2,3)=126 (2,4)=28  \
(3,1)=60  (3,2)=98 (3,3)=0   (3,4)=132 \
(4,1)=132 (4,2)=98 (4,3)=21  (4,4)=28
SATISFIABLE

$ clingo-dl -c n=132 encoding-dl.lp tai4_4_1.lp
[...]

$ clingcon -c n=132 encoding-casp.lp tai4_4_1.lp
[...]

$ clingo -c n=132 encoding.lp tai4_4_1.lp
[...]
```

## Installation

Precompiled packages are available:

- https://anaconda.org/potassco/clingo-lpx
- https://anaconda.org/conda-forge/clingo-lpx
- https://pypi.org/project/clingo-lpx/
- https://launchpad.net/~potassco/+archive/ubuntu/stable

To install the conda packages, please install [miniconda] first.
Note that packages from the [conda-forge] channel offer better performance than the ones from the potassco channel.

## Input format

The system supports `&sum` constraints over rationals with relations among `<=`, `>=`, and `=`.
The elements of the sum constraints must have form `x`, `-x`, `n*x`, `-n*x`, or `-n/d*x`
where `x` is a function symbol or tuple, and `n` and `d` are numbers.
A number has to be either a non negative integer or decimal number in quotes.

For example, the following program is accepted:
```
{ x }.
&sum { x; -y; 2*x; -3*y; 2/3*x; -3/4*y, "0.75"*z } >= 100 :- x.
```

Furthermore, `&dom` constraints are supported, which are shortcuts for `&sum` constraints.
The program
```
{ x }.
&dom { 1..2 } = x.
```
is equivalent to
```
{ x }.
&sum { x } >= 1.
&sum { x } <= 2.
```

When option `--strict` is passed to the solver, then also strict constraints are supported:
```
{ x }.
&sum { x } > 1.
&sum { x } < 2.
```
The assignment will then contain an epsilon component for each variable.
For example, with the above program, `x>=1+e` will appear in the output.
This feature could also be used to support constraints in rule body and the `!=` relation;
neither is implemented at the moment.

Finally, the solver supports `&minimize` and `&maxmize` objectives where the former is a shortcut for the latter negating coefficients.
The terms between the braces have the same syntax as for `&sum` constraints.
However, objectives do not have a guard.

For example, the following objective is accepted:
```
&maximize { 2*x; -2*y }.
```

By default, the optimal objective value is reported w.r.t. a stable model.
Using option `--objective=global` together with `--models=0` can be used to enumerate globally optimal solutions.
It is also possible to give a step value requiring the next objective to be greater than or equal to the current one plus the step value.
In strict mode with option `--strict`, it is possible to use a symbolic epsilon value as step value,
that is, by passing option `--objective=global,e`.

## Options

| Option | Description |
| :-- | :-- |
| `--[no-]strict` | Enable support for strict constraints. |
| `--[no-]propagate-conflicts` | Add binary clauses for conflicting bounds involving the same variable. |
| `--[no-]propagate-bounds` | Enable propagation of conflicting bounds. The current algorithm should be considered preliminary. It is neither as exhaustive as it could be nor is it very efficient. |
| `--objective={local,global[,step]}` | Configure how to treat the objective function. Values `local` and `global` compute optimal assignments w.r.t. to one and all stable models, respectively. When computing global optima, it is also possible to give a step value requiring the next objective to be greater than or equal to the current one plus the step value. In strict mode with option `--strict`, it is possible to use a symbolic epsilon value as step value, that is, by passing option `--objective=global,e`. |
| `--select={none,match,conflict}` | Configure the sign heuristic for linear constraints. It can be set to `none` to use the sign heuristic of the ASP solver, `match` to make literals true whenever the corresponding constraint does not violate the current assignment, or `conflict` to do the opposite. |
| `--store={no,partial,total}` | Configure whether to maintain satisfying assignments when backtracking. Value `partial` and `total` determine whether this is done w.r.t. to partial or total propagation fixed points. The latter is especially interesting when enumerating models to reduce the number of pivots. |
| `--[no-]enable-python` | Enable Python script tags. Only works when running the python module, e.g., `python -m clingolpx`. |

## Profiling

To compile and profile the package, [cmake], [gperftools], [clingo], and a C++ compiler supporting C++17 have to be installed.
All these requirements can be installed with [conda-forge].

```bash
conda create -n profile -c conda-forge cmake libflint clingo cxx-compiler gperftools
conda activate profile
cmake -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCLINGOLPX_PROFILE=ON
cmake --build build
```

The resulting binary can then be profiled using the following calls:

```bash
CPUPROFILE_FREQUENCY=1000 ./build/bin/clingo-lpx examples/encoding-lp.lp examples/tai4_4_1.lp --stats -c n=132 -q 0
google-pprof --gv ./build/bin/clingo-lpx clingo-lpx-solve.prof
```

## Literature

- "Integrating Simplex with `DPLL(T)`" by Bruno Dutertre and Leonardo de Moura
- "SPASS-SATT: A CDCL(LA) Solver" by Martin Bromberger, Mathias Fleury, Simon Schwarz, Christoph Weidenbach

## Math Libraries

The project currently uses [FLINT] for arithmetics.
Alternatively, the slower but more lightwight [IMath] library can be used when configuring with `CLINGOLPX_MATH_LIBRARY=imath`.
Furthermore, note that [IMath] uses the MIT and [FLINT] the LGPL license.

[FLINT]: https://github.com/wbhart/flint2
[IMath]: https://github.com/creachadair/imath
[cmake]: https://cmake.org
[clingo]: https://github.com/potassco/clingo
[conda-forge]: https://conda-forge.org/
[gperftools]: https://gperftools.github.io/gperftools/cpuprofile.html
[miniconda]: https://docs.conda.io/en/latest/miniconda.html
