# Integer Program

The program maximizes `y` subject to three linear constraints over the
integer variables `x` and `y`. Without the `&int` directive, the optimum
`x=9/5` and `y=14/5` is fractional. The integer optimum is `y=2`.

With the default local optimization, the reported assignment is optimal only
for the branch the solver ended up in:

```
$ clingo-lpx integer.lp
x=3 y=1
Objective: 1 [bounded]
```

Global optimization finds the optimum. Because the objective only takes
integer values, each model can be required to improve by at least 1:

```
$ clingo-lpx --objective=global,1 integer.lp 0
Answer: 1
x=3 y=1
Objective: 1 [bounded]
Answer: 2
x=2 y=2
Objective: 2 [bounded]
```
