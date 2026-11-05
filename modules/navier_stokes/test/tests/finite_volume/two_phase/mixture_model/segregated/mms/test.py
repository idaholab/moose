import math
import os
import subprocess
import unittest

import mooseutils
import pandas
from mooseutils import fuzzyAbsoluteEqual

# ---------------------------------------------------------------------------------------------
# Combined space and time refinement
#
# A pure time refinement on a fixed mesh plateaus once the temporal error drops below the spatial
# error, so the measured slope stops being the temporal order. Refining the mesh and the time step
# together with dt proportional to h avoids that: the total error is O(h^2) + O(dt^2), so a scheme
# that is second order in both gives a clean slope of two, while a first order time integrator pulls
# the combined slope down to one. The study below relies on exactly that contrast.
# ---------------------------------------------------------------------------------------------


# The all-terms study starts one level in. Its coarse end is not in the asymptotic range at all:
# between 10 and 20 the temperature converges at 1.39 and the velocities at 1.65, which drags the
# mean down without saying anything about the discretisation. From 20 the ladder is clean, the
# velocities running mildly super-convergent at 2.31 on the finest pair against 2.07 for the phase
# fraction and the temperature.
_FINE_LEVELS = [(20, 0.02), (40, 0.01), (80, 0.005)]


def _executable():
    """The application the study is run with.

    The TestHarness names it in MOOSE_PYTHONUNITTEST_EXECUTABLE, which is the only way to reach a
    combined application build: searching the directories above this one finds the Navier-Stokes
    binary, which such a build does not produce.
    """
    exe = os.environ.get("MOOSE_PYTHONUNITTEST_EXECUTABLE")
    if exe:
        return exe
    return mooseutils.find_moose_executable_recursive(
        os.path.dirname(os.path.abspath(__file__))
    )


_ALL_TERMS = "2d-drift-flux-mms-all-terms.i"
# The pressure is asserted alongside the other fields. Nothing else reads the pressure itself,
# only its gradient, so a constant error in it is invisible everywhere except here.
_ALL_FIELDS = ("L2phi", "L2u", "L2v", "L2T", "L2p")


def _run_levels(extra_args, input_file, prefix, levels):
    """Run the transient case over the refinement levels, returning the final-time errors."""
    exe = _executable()
    rows = []
    for nx, dt in levels:
        base = "{}_{}".format(prefix, nx)
        cmd = [
            exe,
            "-i",
            input_file,
            "Mesh/gmg/nx={}".format(nx),
            "Mesh/gmg/ny={}".format(nx),
            "Executioner/dt={}".format(dt),
            "Outputs/file_base={}".format(base),
        ] + extra_args
        subprocess.run(
            cmd, check=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL
        )
        rows.append(pandas.read_csv(base + ".csv").iloc[-1])
    return rows


def _order(rows, label):
    """Mean of the observed order over every consecutive pair of levels.

    Taking the finest pair alone makes the verdict hang on a single ratio, which is noisy: a study
    can be converging perfectly well and still put one pair outside a tolerance band, either
    because the coarse end has not reached the asymptotic range or because a field is briefly
    super-convergent. Averaging over the ladder uses every level that was paid for and is not
    thrown by one ratio.
    """
    orders = [
        math.log(rows[i - 1][label] / rows[i][label], 2.0) for i in range(1, len(rows))
    ]
    return sum(orders) / len(orders)


def check_temporal_convergence(
    extra_args,
    expected_order,
    tol=0.3,
    input_file=_ALL_TERMS,
    prefix="at",
    labels=_ALL_FIELDS,
    levels=_FINE_LEVELS,
):
    rows = _run_levels(extra_args, input_file, prefix, levels)
    for label in labels:
        value = _order(rows, label)
        print("%s, %f" % (label, value))
        assert fuzzyAbsoluteEqual(value, expected_order, tol)


class TestAllTermsBDF2(unittest.TestCase):
    """Every term of the model at once, with each transient in the conservative form the kernel
    assembles by default: d(rho_m u)/dt in momentum, d(rho_d alpha)/dt in the dispersed phase mass
    balance and d(rho_m cp_m T)/dt in energy, all with multipliers that move in time. Second order
    in every field, the energy included."""

    def test(self):
        check_temporal_convergence(
            ["Executioner/scheme=bdf2"],
            2.0,
            # The velocity is super-convergent on the finest pair. The Dirichlet pressure boundary
            # that makes the pressure system non-singular also carries a coarse mesh error which
            # converges away quickly, so the fitted orders are about 2.10 and 2.71 and their mean
            # sits above a 0.3 band. Both are at or above second order, so nothing has lost order,
            # and the band is widened to admit that rather than to excuse a degradation: 0.5 still
            # rejects the 1.22 and 0.85 that a first order term produces.
            tol=0.5,
            input_file=_ALL_TERMS,
            prefix="at_bdf2",
            labels=_ALL_FIELDS,
            levels=_FINE_LEVELS,
        )
