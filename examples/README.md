# Examples

Each folder contains a problem with a short description and example calls.
The calls assume that `clingo-lpx` is in the path and are run from within the
respective folder.

| Folder                    | Problem                                            | Features                                      |
| :------------------------ | :------------------------------------------------- | :-------------------------------------------- |
| [ship](ship)              | Ship goods with trucks and vans                    | Linear constraints, conditions, optimization  |
| [integer](integer)        | A tiny integer program                             | `&int`, local vs. global optimization         |
| [knapsack](knapsack)      | Pack copies of items into a knapsack               | Integer variables combined with choice rules  |
| [facility](facility)      | Open factories and send trucks to shops            | Boolean, integer, and continuous variables    |
| [open-shop](open-shop)    | Schedule operations on machines                    | Several encodings including difference logic  |
