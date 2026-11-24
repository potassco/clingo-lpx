"""
Test the clingo-lpx application.
"""

import json
import subprocess
import sys


def run_app(*args: str, program: str = "") -> subprocess.CompletedProcess[str]:
    """
    Run the application in a separate process.

    The application cannot run in the test process because it reads the
    command line from sys.argv, exits the process, and might exit on the C
    level.
    """
    return subprocess.run(
        [sys.executable, "-c", "import clingolpx; clingolpx._pyclingolpx()", *args],
        input=program,
        capture_output=True,
        text=True,
        timeout=60,
        check=False,
    )


def test_version():
    """
    Test printing the version.
    """
    res = run_app("--version")
    assert res.returncode == 0
    assert res.stdout.startswith("clingo-lpx version ")


def test_sat():
    """
    Test solving a satisfiable program.
    """
    res = run_app("--outf=2", "0", program="&sum { x } >= 2. &sum { x } <= 2. {p}.")
    assert res.returncode == 30
    out = json.loads(res.stdout)
    assert out["Result"] == "SATISFIABLE"
    models = sorted(sorted(wit["Value"]) for wit in out["Call"][0]["Witnesses"])
    assert models == [['__lpx(x,"2")'], ['__lpx(x,"2")', "p"]]


def test_unsat():
    """
    Test solving an unsatisfiable program.
    """
    res = run_app("--outf=2", program="&sum { x } >= 3. &sum { x } <= 2.")
    assert res.returncode == 20
    assert json.loads(res.stdout)["Result"] == "UNSATISFIABLE"


def test_error():
    """
    Test passing an unknown option.
    """
    res = run_app("--unknown-option")
    assert res.returncode == 65
    assert "unknown option" in res.stderr


MIP = """\
&int { x; y }.
&sum { -x; y } <= 1.
&sum { 3*x; 2*y } <= 12.
&sum { 2*x; 3*y } <= 12.
&sum { x } >= 0.
&sum { y } >= 0.
&maximize { y }.
"""


def test_integer():
    """
    Test global optimization with integer variables using different options.

    The relaxation has the optimum y=14/5 but the integer optimum is y=2.
    """
    options = [
        [],
        ["--cut-period=1", "--cut-limit=1"],
        ["--no-order-branches", "--no-gcd-test"],
        ["--branch-polarity=farthest", "--fix-reduced-costs"],
        ["-t", "3"],
    ]
    for opts in options:
        res = run_app("--outf=2", "0", "--objective=global,1", *opts, program=MIP)
        assert res.returncode == 30, (opts, res.stderr)
        witnesses = json.loads(res.stdout)["Call"][0]["Witnesses"]
        last = sorted(witnesses[-1]["Value"])
        assert '__lpx(y,"2")' in last, opts
        assert '__lpx_objective("2","bounded")' in last, opts
        for wit in witnesses:
            assert all("/" not in val for val in wit["Value"]), opts
