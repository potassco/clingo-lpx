"""Differential fuzzer for Solver::optimize of clingo-lpx.

Usage: fuzz-optimize.py CLINGO_LPX [COUNT [SEED]]

Random small LPs with optional Boolean structure are solved with clingo-lpx and
compared against a vertex enumeration over exact rationals. Strict constraints
are handled with a symbolic epsilon represented as a pair (c, k) ordered
lexicographically.
"""

import itertools
import random
import re
import subprocess
import sys
from fractions import Fraction

BIN = sys.argv[1]
N = int(sys.argv[2]) if len(sys.argv) > 2 else 1000
SEED = int(sys.argv[3]) if len(sys.argv) > 3 else 0

ZERO = (Fraction(0), Fraction(0))


def q_add(a, b):
    return (a[0] + b[0], a[1] + b[1])


def q_scale(a, f):
    return (a[0] * f, a[1] * f)


def q_parse(s):
    if "e" not in s:
        return (Fraction(s), Fraction(0))
    body = s[:-1].removesuffix("*")
    if "+" in body:
        c, k = body.split("+")
    else:
        c, k = "0", body
    return (Fraction(c), Fraction(k or 1))


def solve_lin(rows, rhs):
    """Solve a square system with epsilon right hand sides, None if singular."""
    n = len(rows)
    a = [[Fraction(x) for x in r] for r in rows]
    b = list(rhs)
    for c in range(n):
        p = next((r for r in range(c, n) if a[r][c] != 0), None)
        if p is None:
            return None
        a[c], a[p] = a[p], a[c]
        b[c], b[p] = b[p], b[c]
        for r in range(n):
            if r != c and a[r][c] != 0:
                f = a[r][c] / a[c][c]
                a[r] = [x - f * y for x, y in zip(a[r], a[c])]
                b[r] = q_add(b[r], q_scale(b[c], -f))
    return [q_scale(b[i], 1 / a[i][i]) for i in range(n)]


def dot(co, x):
    s = ZERO
    for c, v in zip(co, x):
        s = q_add(s, q_scale(v, Fraction(c)))
    return s


def closed(cons):
    """Replace strict by non-strict constraints with an epsilon offset."""
    res = []
    for co, rel, b in cons:
        b = (Fraction(b), Fraction(0))
        if rel == "<":
            rel, b = "<=", (b[0], Fraction(-1))
        elif rel == ">":
            rel, b = ">=", (b[0], Fraction(1))
        res.append((co, rel, b))
    return res


def feasible(cons, x):
    for co, rel, b in cons:
        s = dot(co, x)
        if rel == "<=" and s > b or rel == ">=" and s < b or rel == "=" and s != b:
            return False
    return True


def ref_max(n, cons, obj, box):
    """Maximize obj over cons intersected with |x_i| <= box via vertices."""
    allc = list(cons)
    for i in range(n):
        e = [0] * n
        e[i] = 1
        allc.append((e, "<=", (Fraction(box), Fraction(0))))
        allc.append((e, ">=", (Fraction(-box), Fraction(0))))
    best = None
    for sub in itertools.combinations(allc, n):
        x = solve_lin([c[0] for c in sub], [c[2] for c in sub])
        if x is None or not feasible(allc, x):
            continue
        v = dot(obj, x)
        if best is None or v > best:
            best = v
    return best


def reference(n, cons, obj):
    """Return None if infeasible, (value, bounded) otherwise."""
    # vertices of the original polyhedron have small coordinates, so a
    # changing optimum when enlarging the box indicates unboundedness
    cons = closed(cons)
    b1 = ref_max(n, cons, obj, 10**4)
    if b1 is None:
        return None
    b2 = ref_max(n, cons, obj, 10**5)
    return (b1, True) if b1 == b2 else (None, False)


def term(c, v):
    return v if c == 1 else f"{c}*{v}"


def gen(rng, strict):
    n = rng.randint(1, 4)
    m = rng.randint(0, 6)
    atoms = [f"a{i}" for i in range(rng.choice([0, 0, 1, 2, 3]))]
    rels = ["<=", ">=", "="] + (["<", ">"] if strict else [])
    cons = []
    for _ in range(m):
        k = rng.choice([2, 2, 3, n])
        idx = rng.sample(range(n), min(k, n))
        co = [0] * n
        for i in idx:
            co[i] = rng.choice([-3, -2, -1, -1, 1, 1, 2, 3])
        cons.append((co, rng.choice(rels), rng.choice([0, 0, *range(-6, 7)])))
    # variable bounds exercise bound flips
    upper = ["<=", "<"] if strict else ["<="]
    lower = [">=", ">"] if strict else [">="]
    for i in range(n):
        for rs in (upper, lower):
            if rng.random() < 0.4:
                co = [0] * n
                co[i] = rng.choice([1, 1, -1, 2])
                cons.append((co, rng.choice(rs), rng.randint(-4, 4)))
    guards = []
    for _ in cons:
        if atoms and rng.random() < 0.4:
            guards.append((rng.random() < 0.7, rng.choice(atoms)))
        else:
            guards.append(None)
    obj = [rng.choice([-2, -1, 0, 1, 2]) for _ in range(n)]
    if all(c == 0 for c in obj):
        obj[rng.randrange(n)] = 1
    return n, atoms, cons, guards, obj, rng.random() < 0.5


def program(n, atoms, cons, guards, obj, minimize):
    out = []
    if atoms:
        out.append(f"{{ {'; '.join(atoms)} }}.")
        out.append(f"#show {'. #show '.join(f'{a}/0' for a in atoms)}.")
    else:
        out.append("#show.")
    for (co, rel, b), g in zip(cons, guards):
        ts = "; ".join(term(c, f"x{i}") for i, c in enumerate(co) if c != 0)
        body = "" if g is None else f" :- {'' if g[0] else 'not '}{g[1]}"
        out.append(f"&sum {{ {ts} }} {rel} {b}{body}.")
    sign = -1 if minimize else 1
    ts = "; ".join(term(sign * c, f"x{i}") for i, c in enumerate(obj) if c != 0)
    out.append(f"&{'minimize' if minimize else 'maximize'} {{ {ts} }}.")
    return "\n".join(out) + "\n"


def run(prg, extra):
    res = subprocess.run(
        [BIN, "0", *extra],
        input=prg,
        capture_output=True,
        text=True,
        check=False,
        timeout=60,
    )
    if "SATISFIABLE" not in res.stdout or res.returncode not in (10, 20, 30):
        raise RuntimeError(res.stdout + res.stderr)
    models = []
    for blk in res.stdout.split("Answer: ")[1:]:
        lines = blk.split("\n")
        true = frozenset(lines[1].split())
        asg = lines[lines.index("Assignment:") + 1]
        vals = {k: q_parse(v) for k, v in (kv.split("=") for kv in asg.split())}
        m = re.search(r"Objective: (\S+) \[(\w+)\]", blk)
        models.append((true, vals, q_parse(m.group(1)), m.group(2) == "bounded"))
    return models


def active(cons, guards, true):
    return [c for c, g in zip(cons, guards) if g is None or (g[1] in true) == g[0]]


def satisfies(cons, x):
    """Check the solver's epsilon assignment against the original relations."""
    for co, rel, b in cons:
        s = q_add(dot(co, x), (-Fraction(b), Fraction(0)))
        if not {
            "<=": s <= ZERO,
            "<": s < ZERO,
            ">=": s >= ZERO,
            ">": s > ZERO,
            "=": s == ZERO,
        }[rel]:
            return False
    return True


def same(a, b):
    """Compare optima ignoring the scaling of epsilon.

    The solver scales epsilon per bound, so only the rational part and
    whether the supremum is attained are canonical.
    """
    return a[0] == b[0] and (a[1] == 0) == (b[1] == 0)


def check_model(n, cons, obj, model, exp):
    _, vals, val, bounded = model
    x = [vals.get(f"x{i}", ZERO) for i in range(n)]
    if not satisfies(cons, x) or val != dot(obj, x):
        return False
    return bounded == exp[1] and (not bounded or same(val, exp[0]))


def expected(inst):
    n, atoms, cons, guards, obj, _ = inst
    sat = {}
    for k in range(len(atoms) + 1):
        for true in map(frozenset, itertools.combinations(atoms, k)):
            e = reference(n, active(cons, guards, true), obj)
            if e is not None:
                sat[true] = e
    return sat


def check(inst, sat, models, glob):
    n, _, cons, guards, obj, _ = inst
    for m in models:
        e = sat.get(m[0])
        if e is None:
            return "infeasible model"
        if not check_model(n, active(cons, guards, m[0]), obj, m, e):
            return "wrong optimum"
    if not glob:
        # every Boolean assignment with a feasible LP is enumerated once
        if sorted(map(sorted, sat)) != sorted(sorted(m[0]) for m in models):
            return "wrong models"
        return None
    if not sat:
        return None if not models else "expected unsat"
    if not models:
        return "expected sat"
    # the last model must be globally optimal
    unb = [t for t, e in sat.items() if not e[1]]
    best = max((e[0] for e in sat.values() if e[1]), default=None)
    last = models[-1]
    if unb and last[3] or not unb and not same(last[2], best):
        return "not globally optimal"
    for a, b in itertools.pairwise(models):
        if a[3] and b[3] and b[2][0] < a[2][0]:
            return "objective decreased"
    return None


CONFIGS = [
    [],
    ["--store=no"],
    ["--store=total"],
    ["--propagate-bounds=full"],
    ["--select=match"],
    ["--objective=global"],
    ["--objective=global", "--store=no"],
]


def main():
    rng = random.Random(SEED)
    fails = 0
    stats = {"unsat": 0, "bounded": 0, "unbounded": 0}
    for it in range(N):
        strict = rng.random() < 0.4
        inst = gen(rng, strict)
        prg = program(*inst)
        sat = expected(inst)
        for e in sat.values():
            stats["bounded" if e[1] else "unbounded"] += 1
        stats["unsat"] += 2 ** len(inst[1]) - len(sat)
        for extra in CONFIGS:
            glob = "--objective=global" in extra
            if strict:
                extra = [
                    x + ",e" if x == "--objective=global" else x for x in extra
                ] + ["--strict"]
            models = run(prg, extra)
            err = check(inst, sat, models, glob)
            if err is not None:
                fails += 1
                print(f"MISMATCH ({err}) it={it} extra={extra}\n{prg}", flush=True)
                break
    print(f"done {N} instances, {fails} failures, {stats}")


main()
