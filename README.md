# clingo-lpx

clingo-lpx extends [clingo] with linear constraints over the rationals.
Constraints are written as theory atoms and checked with a simplex solver
integrated into the search of clingo. Strict constraints and linear objective
functions are supported as well.

## Installation

Precompiled packages are available:

- <https://anaconda.org/potassco/clingo-lpx>
- <https://anaconda.org/conda-forge/clingo-lpx>
- <https://pypi.org/project/clingo-lpx/>
- <https://launchpad.net/~potassco/+archive/ubuntu/stable>

To install the conda packages, please install [miniconda] first. Note that
packages from the [conda-forge] channel offer better performance than the ones
from the potassco channel.

## Quick start

The following program (`examples/ship.lp`) ships at least 10 units using trucks
and vans. An express delivery is optional but restricts the number of vans.

```
{ express }.
&sum { truck; van } >= 10.
&sum { truck } >= 0.
&sum { van } >= 0.
&sum { truck } <= 6.
&sum { van } <= 8.
&sum { truck } >= 6 :- express.
&sum { van } <= 4 :- express.
&minimize { 3*truck; 2*van }.
```

Each stable model is printed together with an assignment of the variables:

```
$ clingo-lpx examples/ship.lp 0
Answer: 1 (Model: 1 Time: 0.001s)

Assignment:
truck=2 van=8
Objective: -22 [bounded]
Answer: 2 (Model: 2 Time: 0.001s)
express
Assignment:
truck=6 van=4
Objective: -26 [bounded]
SATISFIABLE
```

More examples can be found in the `examples` folder.

## Input language

### Linear constraints

Linear constraints have the form

```
&sum { e1; ...; en } rel rhs :- body.
```

where `rel` is one of `<=`, `>=`, or `=`. Each element `ei` is a linear term
optionally followed by a condition, as in `X*cost(X) : item(X)`. The right-hand
side `rhs` is a linear term, too.

- Variables are ground terms like `x`, `truck`, or `start(task(1))`. They live
  in their own namespace, so a variable `x` and an atom `x` are unrelated.
- Coefficients are integers or quoted decimals like `"0.75"`. Terms can be
  built with `+`, `-`, `*`, and `/`, but multiplication and division need a
  constant on one side, as in `3*x` or `x/2`.
- Theory atoms may only occur in rule heads. The rule above means that the
  constraint has to hold if the body holds. If the body does not hold, nothing
  is required (in particular, not the negation of the constraint).

For example:

```
p(1..3).
{ q(X) : p(X) }.
&sum { X*v(X) : p(X), q(X) } = 6.
&sum { v(X) } >= 0 :- p(X).
&sum { v(X) } <= 1 :- p(X).
```

For convenience, there are two shortcuts. The `&dom` constraint
`&dom { 1..2 } = x` stands for `x >= 1` and `x <= 2`. The `&diff` constraint
`&diff { a - b } <= 5` stands for `&sum { a - b } <= 5` and exists for
compatibility with [clingo-dl].

### Strict constraints

With option `--strict`, relations `<` and `>` are available. Values may then
contain a symbolic epsilon `e`, which stands for an arbitrarily small positive
number:

```
&sum { x } > 1.
&sum { x } < 2.
```

```
$ clingo-lpx --strict strict.lp
Answer: 1 (Model: 1 Time: 0.001s)

Assignment:
x=1+e
```

The relation `!=` is not supported.

### Optimization

An objective function is given by a `&maximize` or `&minimize` directive,
whose elements have the same syntax as in `&sum` constraints:

```
&maximize { 2*x; -2*y }.
```

Internally, the objective is always maximized, and `&minimize` simply negates
the coefficients. Hence, the reported objective value of a `&minimize`
directive is negated, like `-22` in the quick start example. The value is
followed by `[bounded]` or `[unbounded]`, the latter indicating that the
objective can be increased arbitrarily.

By default (`--objective=local`), the objective is optimized for each stable
model separately. With `--objective=global` and `0` models, each new model has
to be at least as good as the previous one, so that the last model printed is
globally optimal:

```
$ clingo-lpx --objective=global examples/ship.lp 0
Answer: 1 (Model: 1 Time: 0.001s)

Assignment:
truck=2 van=8
Objective: -22 [bounded]
SATISFIABLE
```

A step value requires each new model to improve by at least the step, as in
`--objective=global,1`. In strict mode, the step `e` requires models to be
strictly better (`--objective=global,e`).

## Options

The most important options are listed below.
Run `clingo-lpx --help` for the full list.

| Option                                   | Description                                                                                                                           |
| :--------------------------------------- | :------------------------------------------------------------------------------------------------------------------------------------ |
| `--strict`                               | Enable support for strict constraints.                                                                                                |
| `--objective={local,global[,step]}`      | Optimize w.r.t. each stable model (`local`, default) or all of them (`global`).                                                       |
| `--propagate-bounds={none,changed,full}` | Propagate bounds of constraints whose variables changed or of all constraints (default `none`).                                       |
| `--propagate-conflicts`                  | Add binary clauses for conflicting bounds on the same variable.                                                                       |
| `--select={none,match,conflict}`         | Sign heuristic for constraint literals: keep clasp's choice, prefer literals consistent with the current assignment, or the opposite. |
| `--store={no,partial,total}`             | Keep satisfying assignments when backtracking, which can reduce the number of pivots when enumerating models.                         |
| `--pivot-basic={bland,violation}`        | Fix the conflicting basic variable with the smallest index (default) or the largest bound violation first.                            |
| `--pivot-non-basic={bland,sparse}`       | Pivot on the column with the smallest index or the fewest non-zero elements (`sparse`, default).                                      |
| `--pivot-entering={bland,dantzig}`       | When optimizing, select the entering variable with the smallest index or the largest coefficient (`dantzig`, default).                |
| `--bland-threshold=<n>`                  | Switch to Bland's rule after max(n, rows) pivots without progress to avoid cycling (default 128).                                     |

## Python API

The Python package provides the `pyclingo-lpx` command, which works like
`clingo-lpx` but additionally supports `#script (python)`. To use the solver
from Python, create a theory object and pass it the control object:

```python
from clingo.control import Control
from clingo.core import Library
from clingo.theory import Theory
from clingolpx import create_theory

lib = Library()
theory = Theory(lib, create_theory())
ctl = Control(lib, ["0"])
theory.register(ctl)
ctl.config.lpx.objective = "global"  # same as --objective=global
theory.rewrite_files(lib, ctl, ["examples/ship.lp"])
ctl.ground()
theory.prepare(ctl)


def on_model(model):
    theory.on_model(model)
    values = {str(sym): val.string for sym, val in theory.assignment(model.thread_id)}
    print(model, values)


ctl.solve(on_model=on_model)
```

The steps are: register the theory, add the program, ground, prepare the
theory, and solve. Programs given as strings can be added with
`theory.rewrite_string(lib, ctl, program)`. The `on_model` callback has to call
`theory.on_model` before the assignment can be queried. Values are strings like
`"3/2"` or `"1+e"`. Models are also extended with atoms `__lpx(Var,"Value")`
and `__lpx_objective("Value","bounded")`.

The options above are available in the `lpx` group of the configuration with
dashes replaced by underscores, e.g., `ctl.config.lpx.strict = "yes"` or
`ctl.config.lpx.propagate_bounds = "changed"`.

## Development

### Math Libraries

The project uses [FLINT] for arithmetic if it is available. Alternatively, the
slower but more lightweight [IMath] library can be used by configuring with
`CLINGOLPX_MATH_LIBRARY=imath`. Note that [IMath] uses the MIT and [FLINT] the
LGPL license.

### Profiling

To compile and profile the package, [cmake], [gperftools], [clingo], and a C++
compiler supporting C++20 have to be installed. All these requirements can be
installed with [conda-forge].

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

### Literature

- "Integrating Simplex with `DPLL(T)`" by Bruno Dutertre and Leonardo de Moura
- "SPASS-SATT: A CDCL(LA) Solver" by Martin Bromberger, Mathias Fleury, Simon Schwarz, Christoph Weidenbach

[FLINT]: https://github.com/flintlib/flint
[IMath]: https://github.com/creachadair/imath
[cmake]: https://cmake.org
[clingo]: https://github.com/potassco/clingo
[clingo-dl]: https://github.com/potassco/clingo-dl
[conda-forge]: https://conda-forge.org/
[gperftools]: https://gperftools.github.io/gperftools/cpuprofile.html
[miniconda]: https://docs.conda.io/en/latest/miniconda.html
