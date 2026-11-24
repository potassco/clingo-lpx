% A small instance with one resource and one conflict.
item(tent, 50, 1).
item(stove, 30, 1).
item(rope, 6, 3).
item(food, 12, 4).
item(water, 9, 5).

requires(tent, weight, 9).
requires(stove, weight, 4).
requires(rope, weight, 2).
requires(food, weight, 3).
requires(water, weight, 3).

capacity(weight, 25).

% the stove only makes sense with gas, which does not fit next to the food
conflict(stove, food).
