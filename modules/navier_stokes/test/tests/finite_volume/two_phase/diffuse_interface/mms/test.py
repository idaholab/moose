"""
Convergence of the reduced-pressure diffuse-interface solve on the tanh-interface manufactured
solution (see derive_tanh_interface_mms.py and tanh_interface.i).

The input ties the time step to the mesh, dt = 0.5 / N, so a refinement of N refines space and
time together; mms.run_spatial refines with Mesh/uniform_refine, which would leave dt fixed, so
the refinement loop is written out here. A run only counts if every time step converged: the
outer-iteration residuals are read back from the log, because an unconverged step still writes
an error norm.

The tests verify the combined refinement. They do not resolve the temporal order on its own:
refining dt alone on a fixed mesh shows alpha first order, from the semi-implicit MULES splitting
(see the class comment of ConservativeDiffuseInterfaceVOFMULESCorrector), a term of order dt h
that the combined refinement cannot see.
"""

import math
import os
import re
import subprocess
import unittest

import mooseutils
import pandas

MPI = 4
FIELDS = ["L2u", "L2v", "L2p", "L2alpha"]
# The orders expected of a second-order scheme, with the same tolerances as the single-phase
# linear-segregated vortex cases; the pressure gets the wider one
EXPECTED = {
    "L2u": (2.0, 0.3),
    "L2v": (2.0, 0.3),
    "L2p": (2.0, 0.5),
    "L2alpha": (2.0, 0.3),
}
# Every step must have left the outer loop below its tolerances (1e-8 in the input); a few steps
# of the two-phase case cycle on the pressure outlet switch at this level, which is still three
# orders below the discretization error
MAX_FINAL_RESIDUAL = 2e-5


def executable():
    exe = os.environ.get("MOOSE_PYTHONUNITTEST_EXECUTABLE")
    return exe if exe else mooseutils.find_moose_executable_recursive(os.getcwd())


def run(N, tag, extra_args):
    base = "{}_N{}".format(tag, N)
    log = base + ".log"
    with open(log, "w") as f:
        cmd = [
            os.environ.get("MOOSE_MPI_COMMAND", "mpiexec"),
            "-n",
            str(MPI),
            executable(),
            "-i",
            "tanh_interface.i",
            "N={}".format(N),
            "Outputs/file_base=" + base,
        ] + extra_args
        rc = subprocess.call(cmd, stdout=f, stderr=subprocess.STDOUT, encoding="utf-8")
    if rc != 0:
        raise RuntimeError("{} failed, see {}".format(" ".join(cmd), log))

    # Final outer-iteration residuals of every step
    ansi = re.compile(r"\x1b\[[0-9;]*m")
    worst, last = 0.0, {}
    for line in open(log):
        line = ansi.sub("", line)
        if line.startswith("Time Step"):
            worst = max([worst] + list(last.values()))
            last = {}
        elif "Momentum equation" in line or "Pressure equation" in line:
            last[line.split(":")[0] + line.split("Component")[-1][:2]] = float(
                line.split()[-4]
            )
    worst = max([worst] + list(last.values()))
    if worst > MAX_FINAL_RESIDUAL:
        raise RuntimeError("{}: a step ended with residual {:.1e}".format(base, worst))

    return pandas.read_csv(base + ".csv").iloc[-1]


def check_orders(test, levels, tag, extra_args):
    rows = [run(N, tag, extra_args) for N in levels]
    # Observed order between the two finest levels
    ratio = math.log(rows[-2]["h"] / rows[-1]["h"])
    for field in FIELDS:
        order = math.log(rows[-2][field] / rows[-1][field]) / ratio
        expected, tol = EXPECTED[field]
        print("{} {}: {:.2f}".format(tag, field, order))
        test.assertTrue(
            abs(order - expected) < tol,
            "{} {}: observed order {:.2f}, expected {} within {}".format(
                tag, field, order, expected, tol
            ),
        )


class TestSinglePhase(unittest.TestCase):
    """Equal densities and viscosities, no gravity: the flow does not see the volume fraction."""

    def test(self):
        check_orders(
            self,
            [20, 40, 80],
            "single_phase",
            [
                "rho_l=1",
                "rho_g=1",
                "mu_l=0.01",
                "mu_g=0.01",
                "g=0",
                "P0=1",
                "c_alpha=0.1",
            ],
        )


class TestTwoPhase(unittest.TestCase):
    """Density ratio 1000 under gravity, to t = 0.2. N = 20 is pre-asymptotic (eps/h = 1.4)."""

    def test(self):
        check_orders(self, [40, 80], "two_phase", ["c_alpha=0.1", "end_time=0.2"])


if __name__ == "__main__":
    unittest.main(__name__, verbosity=2)
