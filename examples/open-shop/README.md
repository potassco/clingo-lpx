# Open Shop Scheduling

Each task consists of operations, each of which has to be processed on a
given machine for a given time. The operations of a task can be processed in
any order but not at the same time, and a machine can process one operation
at a time. The instance `tai4_4_1.lp` has four tasks with four operations
each and asks for a schedule with makespan at most `n` (132 by default).

The encodings decide the order of operations with choice rules and differ in
how they represent start times:

- `encoding-asp.lp` uses plain ASP with discretized time,
- `encoding-dl.lp` uses difference constraints (also works with clingo-dl),
- `encoding-casp.lp` uses `&sum` and `&dom` constraints, and
- `encoding-lp.lp` uses `&sum` constraints and can minimize the makespan.

The optimal makespan is 132:

```
$ clingo-lpx encoding-dl.lp tai4_4_1.lp -q
SATISFIABLE
$ clingo-lpx encoding-dl.lp tai4_4_1.lp -c n=131 -q
UNSATISFIABLE
```

The LP encoding can also compute it by minimizing the makespan, which is
defined in program part `objective`:

```
$ clingo-lpx encoding-lp.lp tai4_4_1.lp --parts=base,objective --objective=global,1 0
...
Objective: -132 [bounded]
```
