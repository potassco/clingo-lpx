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

The following program (`examples/ship/ship.lp`) ships at least 10 units using
trucks and vans. An express delivery is optional but restricts the number of
vans.

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
$ clingo-lpx examples/ship/ship.lp 0
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

More examples can be found in the [examples](examples) folder, among them a
knapsack problem and a mixed integer program combining Boolean, integer, and
continuous variables.

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
- Theory atoms may only occur in rule heads (see [Rewriting
  constraints](#rewriting-constraints) for an exception). The rule above means
  that the constraint has to hold if the body holds. If the body does not hold,
  nothing is required (in particular, not the negation of the constraint).

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

Strict mode makes the arithmetic more expensive. Hence, clingo-lpx warns if
option `--strict` is not needed for the program. The relation `!=` is only
available with option `--rewrite-constraints` (see [Rewriting
constraints](#rewriting-constraints)).

### Integer variables

The `&int` directive declares variables that may only take integer values:

```
&int { x; y }.
&sum { -x; y } <= 1.
&sum { 3*x; 2*y } <= 12.
&sum { 2*x; 3*y } <= 12.
&sum { x } >= 0.
&sum { y } >= 0.
&maximize { y }.
```

```
$ clingo-lpx --objective=global,1 examples/integer/integer.lp 0
...
Assignment:
x=2 y=2
Objective: 2 [bounded]
SATISFIABLE
```

The relaxation of this problem has the optimum `y=14/5`. When the simplex
algorithm finds an assignment where an integer variable has a fractional value
`v`, the solver branches by introducing a literal `l` with `l -> x <= floor(v)`
and `-l -> x >= floor(v)+1`. By default, the side closer to `v` is tried first
(see option `--branch-polarity`). When such a literal is decided again later,
the side containing the current value of the variable is selected (see option
`--branch-phase`). Like in an order encoding, the literals of branches on the
same variable are connected by clauses such as `x <= 1 -> x <= 2`, and so are
the literals of bounds in the program, such as `p` in `&sum { x } <= 1 :- p.`

Before branching, the solver checks whether the rows of the tableau over
integer variables admit integer solutions given the fixed variables (the GCD
test). Optionally, Gomory cuts are derived from the tableau instead of
branching (see option `--cut-period`). To keep the arithmetic cheap, cuts are
weakened by rounding up coefficients with large denominators and by dropping
terms of variables bounded on both sides. Cuts are added as new rows implied by
the current bounds and replaced by newer ones once more than `--cut-limit` cuts
are kept. With `--objective=global`, option `--fix-reduced-costs` additionally
tightens bounds of integer variables that cannot improve on the best objective
value found so far.

Constraints over integer variables only are scaled to coprime integer
coefficients and their bounds are rounded. For example, `&sum { 2*x; 2*y } =
3` is false without search. Such constraints may also use the relations `<`
and `>` without option `--strict`.

Some remarks:

- The directive is unconditional. Variables in conditional terms like `x : p`
  are integer if `x` is.
- With `--objective=local`, the reported assignment is integral and optimal
  for the branch the solver ended up in, that is, among the assignments
  respecting the bounds introduced by branching, but not necessarily for the
  stable model. Hence, local optimization is mainly useful for linear
  programs without integer variables. Use `--objective=global` (with a step
  of `1` for integer objectives) to obtain the optimum.
- The literals introduced for branching and cuts are not part of the output.
  Hence, the same stable model and assignment can be printed multiple times
  when enumerating models. Option `--project` avoids this.
- Integer variables should be bounded. Otherwise, branching might not
  terminate.

### Rewriting constraints

With option `--rewrite-constraints`, `&sum` and `&diff` constraints may also
occur in rule bodies and use the relation `!=`. Both are rewritten into the
constraints supported by the solver:

- A constraint in a rule body holds if and only if its atom is true. For
  `a :- &sum { x } >= 1.`, the solver adds `x >= 1` if the atom is true and
  `x < 1` if it is false.
- A disequality `&sum { x } != 1` is split into `x < 1` and `x > 1` using two
  new literals, of which exactly one is true if the atom is true. The negation
  of an equality in a body is a disequality.

Atoms in rule heads and bodies are distinct. For example,
`&sum { x } >= 5 :- &sum { x } >= 5.` has two stable models, one where the
constraint holds and one where it does not.

The rewriting introduces the strict relations `<` and `>`. Over rational
variables, they need option `--strict`. Constraints over integer variables
only are rounded as described in [Integer variables](#integer-variables) and
work without option `--strict`. For example, `&sum { x/2; y/3 } != 1` becomes
`3*x + 2*y <= 5` or `3*x + 2*y >= 7` if `x` and `y` are integer.

Like the literals introduced for branching, the new literals are not part of
the output. Hence, the same stable model can be printed multiple times:

```
&int { x }.
&sum { x } >= 0.
&sum { x } <= 2.
a :- &sum { x } != 1.
```

```
$ clingo-lpx --rewrite-constraints neq.lp 0
Answer: 1 (Time: 0.005s)

Assignment:
x=1
Answer: 2 (Time: 0.005s)
a
Assignment:
x=2
Answer: 3 (Time: 0.005s)
a
Assignment:
x=0
SATISFIABLE
```

Option `--project` prints each stable model once.

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
$ clingo-lpx --objective=global examples/ship/ship.lp 0
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
| `--rewrite-constraints`                  | Enable constraints in rule bodies and the relation `!=`.                                                                              |
| `--objective={local,global[,step]}`      | Optimize w.r.t. each stable model (`local`, default) or all of them (`global`).                                                       |
| `--propagate-bounds={none,changed,full}` | Propagate bounds of constraints whose variables changed (`changed`, default), of all constraints, or none.                            |
| `--propagate-conflicts`                  | Add binary clauses for conflicting bounds on the same variable.                                                                       |
| `--select={none,match,conflict}`         | Sign heuristic for constraint literals: keep clasp's choice, prefer literals consistent with the current assignment, or the opposite. |
| `--store={no,partial,total}`             | Keep satisfying assignments when backtracking, which can reduce the number of pivots when enumerating models.                         |
| `--pivot-basic={bland,violation}`        | Fix the conflicting basic variable with the smallest index (default) or the largest bound violation first.                            |
| `--pivot-non-basic={bland,sparse}`       | Pivot on the column with the smallest index or the fewest non-zero elements (`sparse`, default).                                      |
| `--pivot-entering={bland,dantzig}`       | When optimizing, select the entering variable with the smallest index or the largest coefficient (`dantzig`, default).                |
| `--bland-threshold=<n>`                  | Switch to Bland's rule after max(n, rows) pivots without progress to avoid cycling (default 128).                                     |
| `--cut-period=<n>`                       | Try a Gomory cut instead of branching every n-th time an integer variable is fractional (default 0, which disables cuts).             |
| `--cut-limit=<n>`                        | Keep at most n cuts per solver thread replacing inactive ones (default 64).                                                           |
| `--[no-]order-branches`                  | Connect the literals of branches and bounds on the same variable like in an order encoding (enabled by default).                      |
| `--branch-polarity={nearest,...}`        | Try the side of a branch closer to the current value first (`nearest`, default), the farther one, or always up or down.               |
| `--branch-phase={saved,value}`           | When deciding branch literals, select the side containing the current value (`value`, default) or keep the solver's sign.             |
| `--[no-]gcd-test`                        | Before branching, check rows over integer variables for divisibility (enabled by default).                                            |
| `--[no-]fix-reduced-costs`               | With `--objective=global`, tighten bounds of integer variables using reduced costs (disabled by default).                             |

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
theory.rewrite_files(lib, ctl, ["examples/ship/ship.lp"])
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
`ctl.config.lpx.propagate_bounds = "full"`.

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
CPUPROFILE_FREQUENCY=1000 ./build/bin/clingo-lpx examples/open-shop/encoding-lp.lp examples/open-shop/tai4_4_1.lp --stats -c n=132 -q 0
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
