# simplex

A simplistic simplex solver for checking satisfiability of a set of equations.

## Installation

To compile the package, [cmake], [gmp], [clingo], and a C++ compiler supporting C++17 have to be installed.
All these requirements can be installed with [anaconda].

```bash
conda create -n simplex -c potassco/label/dev cmake ninja gmp clingo gxx_linux-64
conda activate simplex
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

[cmake]: https://cmake.org
[gmp]: https://gmplib.org
[clingo]: https://github.com/potassco/clingo
[anaconda]: https://anaconda.org

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

## Literature

- "Integrating Simplex with `DPLL(T)`" by Bruno Dutertre and Leonardo de Moura
