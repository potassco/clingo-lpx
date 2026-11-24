# Shipping

At least 10 units have to be shipped using trucks and vans. There are 6
trucks and 8 vans, using a truck costs 3, and using a van costs 2. An
optional express delivery requires all 6 trucks but allows for at most 4
vans.

By default, each stable model is optimized separately:

```
$ clingo-lpx ship.lp 0
Answer: 1
Assignment:
truck=2 van=8
Objective: -22 [bounded]
Answer: 2
express
Assignment:
truck=6 van=4
Objective: -26 [bounded]
```

The objective is minimized, so the reported values are negated. To find the
best stable model, the objective has to be optimized globally:

```
$ clingo-lpx --objective=global ship.lp 0
Answer: 1
Assignment:
truck=2 van=8
Objective: -22 [bounded]
```
