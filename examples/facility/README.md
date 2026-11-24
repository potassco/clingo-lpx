# Facility Location

Shops with given demands are supplied by factories. Opening a factory costs a
fixed amount, it can produce a limited number of units, and each unit has a
production cost. The goods are transported by trucks carrying up to 10 units
each, and every truck on a route has a cost. The goal is to minimize the
total cost.

This is a mixed integer program combined with a Boolean layer:

- whether a factory is opened is decided by a choice rule,
- the produced and shipped amounts are continuous, and
- the number of trucks on a route is an integer.

Since the demands are fractional, the objective is optimized globally in
strict mode requiring each model to be strictly better than the previous one:

```
$ clingo-lpx --strict --objective=global,e encoding.lp instance.lp 0
...
open(east) open(north)
Assignment:
make(east)=88/5 make(north)=184/5 make(south)=0 ... ship(east,d)=31/5 ship(east,e)=57/5 ship(north,a)=25/2 ...
trucks(east,d)=1 trucks(east,e)=2 trucks(north,a)=2 trucks(north,b)=1 trucks(north,c)=2 ...
Objective: -439 [bounded]
```

The optimum opens the factories `north` and `east`. Without the `&int`
directive, the trucks can be split and the optimum of the relaxation is
`408.31`.
