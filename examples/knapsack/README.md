# Knapsack

Items come with a value and a number of available copies. Each copy needs
some amount of resources like weight or volume, which are limited. Some items
conflict and cannot be packed together. The goal is to maximize the total
value.

The encoding decides with a choice rule which items to pack and uses integer
variables for the number of copies. Instance `small.lp` has one resource and
`large.lp` has two resources and 20 items.

```
$ clingo-lpx --objective=global,1 encoding.lp small.lp 0
...
pack(stove) pack(tent) pack(water)
Assignment:
copies(food)=0 copies(rope)=0 copies(stove)=1 copies(tent)=1 copies(water)=4
Objective: 116 [bounded]
```

```
$ clingo-lpx --objective=global,1 encoding.lp large.lp 0
...
pack(i4) pack(i9) pack(i13) pack(i20)
Objective: 433 [bounded]
```

Without the `&int` directive, the relaxation of the large instance has the
fractional optimum `3929/9`.
