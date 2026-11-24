import gc

from clingo import ast
from clingo.control import Control
from clingo.core import Library
from clingo.theory import Theory
from clingolpx import create_theory


def test_solve():
    """
    Test theory functionality.
    """
    lib = Library()
    thy = Theory(lib, create_theory())

    ctl = Control(lib)
    thy.register(ctl)
    prg = ast.Program(lib)
    ast.parse_string(
        lib, "a. b. c. &diff{x - y} <= -1.", lambda stm: thy.rewrite(stm, prg.add)
    )
    ctl.join(prg)
    ctl.ground()
    thy.prepare(ctl)

    models = []

    def on_model(model):
        nonlocal models
        thy.on_model(model)
        ass = thy.assignment(model.thread_id)
        models.append(
            (
                [str(sym) for sym in sorted(model.symbols(shown=True))],
                [(str(sym), str(val)) for sym, val in sorted(ass)],
            )
        )

    ctl.solve(on_model=on_model)

    assert models == [
        (
            ['__lpx(x,"-1")', '__lpx(y,"0")', "a", "b", "c"],
            [("x", '"-1"'), ("y", '"0"')],
        )
    ]

    gc.collect()


def test_integer_multishot():
    """
    Test integer variables declared in a later step with several threads.
    """
    lib = Library()
    thy = Theory(lib, create_theory())

    ctl = Control(lib, ["0", "-t", "2"])
    thy.register(ctl)
    ctl.config.lpx.objective = "global,1"
    prg = ast.Program(lib)
    ast.parse_string(
        lib,
        "&sum { -x; y } <= 1.\n"
        "&sum { 3*x; 2*y } <= 12.\n"
        "&sum { 2*x; 3*y } <= 12.\n"
        "&sum { x } >= 0.\n"
        "&sum { y } >= 0.\n"
        "&maximize { y }.\n"
        "#program int.\n"
        "&int { x; y }.\n"
        "#program more.\n"
        "&sum { 2*x } <= 3.\n",
        lambda stm: thy.rewrite(stm, prg.add),
    )
    ctl.join(prg)

    def solve(parts):
        ctl.ground(parts)
        thy.prepare(ctl)
        values = []

        def on_model(model):
            thy.on_model(model)
            values.append(
                sorted(
                    (str(sym), str(val)) for sym, val in thy.assignment(model.thread_id)
                )
            )

        ctl.solve(on_model=on_model)
        return values

    # the relaxation has the optimum x=9/5 and y=14/5
    assert [("x", '"9/5"'), ("y", '"14/5"')] in solve([("base", [])])
    # declaring the variables integer requires branching
    step = solve([("int", [])])
    assert all("/" not in val for model in step for _, val in model)
    assert [("x", '"2"'), ("y", '"2"')] in step
    # a further bound changes the optimum
    step = solve([("more", [])])
    assert all("/" not in val for model in step for _, val in model)
    assert [("x", '"1"'), ("y", '"2"')] in step

    gc.collect()


def test_integer_config():
    """
    Test the configuration of integer options via the control object.
    """
    lib = Library()
    thy = Theory(lib, create_theory())
    ctl = Control(lib)
    thy.register(ctl)
    ctl.config.lpx.branch_polarity = "up"
    assert ctl.config.lpx.branch_polarity.value == "up"
    ctl.config.lpx.gcd_test = "no"
    assert ctl.config.lpx.gcd_test.value == "no"

    gc.collect()
