#!/usr/bin/env python3
# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

"""Measure the Kokkos quadrature-point Jacobian cache: its cost, and its p-multigrid convergence.

Two modes share the sweep-and-scrape machinery. Timing mode measures the cost of the cache against
assembled-Jacobian cost, and convergence mode measures the p-multigrid preconditioner built over the
cache: the linear iteration count of each Newton step as the polynomial order grows, which is what
p-independence means, and the ratios of successive nonlinear residual norms, which is what quadratic
Newton convergence means.

Convergence mode reads the per-solve linear iteration count from PETSc's converged-reason line
rather than by counting monitor lines, because the count is reported there directly and a solve that
approaches its iteration cap emits hundreds of monitor lines that carry nothing else the table needs.

Timing mode: the same input is run at a sequence of polynomial orders with the matrix-free operator
enabled,
which fills the cache inside the ordinary assembled Jacobian sweep. Both costs are therefore
available from one run: the performance graph reports the assembled sweep as the self time of the
kernel section and the cache fill as the nested section beneath it.

Assembling an element Jacobian evaluates the quadrature-point linearization once per trial
function, so its cost per element grows with the number of element degrees of freedom. Filling the
cache evaluates that linearization once per quadrature point, which is what the reported time ratio
measures.

Storage is compared between the assembled sparse matrix, whose nonzero count PETSc reports, and the
cache, whose byte count the Kokkos system reports.
"""

import argparse
import os
import re
import subprocess
import sys

MOOSE_DIR = os.environ.get(
    "MOOSE_DIR", os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
)

DEFAULT_EXECUTABLE = os.path.join(MOOSE_DIR, "test", "moose_test-opt")
DEFAULT_INPUT = os.path.join(
    MOOSE_DIR,
    "test",
    "tests",
    "kokkos",
    "kernels",
    "ad_nonlinear",
    "kokkos_ad_nonlinear_high_order_test.i",
)
CONVERGENCE_INPUT = os.path.join(
    MOOSE_DIR,
    "test",
    "tests",
    "kokkos",
    "preconditioners",
    "pmultigrid",
    "pmultigrid.i",
)

ORDER_NAMES = {
    1: "FIRST",
    2: "SECOND",
    3: "THIRD",
    4: "FOURTH",
    5: "FIFTH",
    6: "SIXTH",
    7: "SEVENTH",
    8: "EIGHTH",
}

# The coarse levels each fine order is measured over. The schedule halves the order down to one, so
# the coarsest problem stays cheap as the fine order grows, which is what makes an iteration count
# comparable across orders. Order one has no coarser level and so has no hierarchy to measure.
LEVEL_ORDERS = {2: "1", 3: "1 2", 4: "1 2", 8: "1 2 4"}

DEFAULT_TIMING_ORDERS = [1, 2, 3, 4]
DEFAULT_CONVERGENCE_ORDERS = [2, 3, 4, 8]

# Timing mode measures on a mesh large enough for per-element cost to dominate setup; convergence
# mode measures on the mesh the recorded p-multigrid iteration counts were taken on
DEFAULT_TIMING_REFINE = 32
DEFAULT_CONVERGENCE_REFINE = 4

# Performance graph rows are pipe-delimited and indent the section name by its depth in the call
# tree: leading whitespace, section name, call count, then the self and total groups of
# (seconds, average, percent, megabytes)
ROW = re.compile(r"^\|(\s*)(\S.*?)\s*\|" + r"\s*([0-9.eE+-]+)\s*\|" * 9)

DOFS = re.compile(r"Num DOFs:\s*(\d+)")
NNZ = re.compile(r"total: nonzeros=(\d+)")
CACHE_MB = re.compile(r"quadrature-point Jacobian cache storage:\s*([0-9.eE+-]+) MB")

# Convergence mode reads two PETSc lines, whose wording is pinned against a captured run:
#   "  0 SNES Function norm 2.991470300666e+00"          (-snes_monitor)
#   "    Linear solve converged due to CONVERGED_RTOL iterations 88"  (-ksp_converged_reason)
# A diverged solve reports "did not converge due to <reason> iterations <n>" in the same shape, and
# is read too, so that a solve sitting on its iteration cap is reported rather than skipped.
SNES_NORM = re.compile(
    r"^\s*(\d+) SNES Function norm\s+([0-9.eE+-]+)\s*$", re.MULTILINE
)
KSP_REASON = re.compile(
    r"^\s*Linear solve (converged|did not converge) due to (\S+) iterations (\d+)\s*$",
    re.MULTILINE,
)

# A PETSc AIJ matrix stores one scalar and one column index per nonzero, plus one index per row
BYTES_PER_NONZERO = 8 + 4
BYTES_PER_ROW = 4


def execute(command, order):
    """Run one case and return its console output, failing loudly if the case does not"""
    result = subprocess.run(command, capture_output=True, text=True)

    if result.returncode:
        sys.exit(f"Case failed (order {order}):\n{result.stdout}\n{result.stderr}")

    return result.stdout


def run(executable, input_file, order, refine, extra):
    """Run one case with the matrix-free operator enabled and return its console output"""
    command = [
        executable,
        "-i",
        input_file,
        f"Variables/u/order={ORDER_NAMES[order]}",
        f"Mesh/square/nx={refine}",
        f"Mesh/square/ny={refine}",
        "Executioner/use_kokkos_matrix_free_jacobian=true",
        "Outputs/pgraph/type=PerfGraphOutput",
        "Outputs/pgraph/level=2",
        "-ksp_view_pmat",
        "::ascii_info",
    ] + extra

    return execute(command, order)


def run_convergence(executable, input_file, order, refine, args, extra):
    """Run one p-multigrid case and return its console output

    The level schedule follows the fine order, so that what the sweep varies is the depth of the
    hierarchy along with the order rather than the size of the coarsest problem.
    """
    if order not in LEVEL_ORDERS:
        sys.exit(f"No coarse level schedule is defined for order {order}")

    command = [
        executable,
        "-i",
        input_file,
        f"Variables/u/order={ORDER_NAMES[order]}",
        f"Mesh/square/nx={refine}",
        f"Mesh/square/ny={refine}",
        f"Preconditioning/pmg/level_orders={LEVEL_ORDERS[order]}",
        f"Executioner/nl_rel_tol={args.snes_rtol}",
        # Stated rather than assumed, so that an input carrying no p-multigrid block of its own can
        # be swept by adding one on the command line
        "Executioner/use_kokkos_matrix_free_jacobian=true",
        "Outputs/exodus=false",
        "-snes_monitor",
        "-ksp_converged_reason",
        "-ksp_type",
        args.krylov,
        "-ksp_rtol",
        str(args.ksp_rtol),
        "-ksp_max_it",
        str(args.ksp_max_it),
    ]

    if args.eisenstat_walker:
        # Spelled with its value rather than bare, because the trailing arguments follow it and PETSc
        # would otherwise read the first of them as this option's value
        command += ["-snes_ksp_ew", "true"]

    return execute(command + extra, order)


def sections(output):
    """Return {call tree path: (calls, self seconds)} for every performance graph section

    A path is the section names from the root of the tree down to the section, joined by slashes,
    which is what distinguishes the several sections that share a name across the residual,
    Jacobian, and residual-and-Jacobian sweeps.
    """
    found = {}
    stack = []

    for line in output.splitlines():
        match = ROW.match(line)

        if not match:
            continue

        indent, name = len(match.group(1)), match.group(2)

        while stack and stack[-1][0] >= indent:
            stack.pop()

        stack.append((indent, name))

        found["/".join(entry[1] for entry in stack)] = (
            int(match.group(3)),
            float(match.group(4)),
        )

    return found


def section(found, path):
    """Return (calls, self seconds) for the one performance graph section matching path

    Path components are matched as suffixes of the corresponding tail components of a section's
    path, so a component need not carry the class name the section is registered under.
    """
    parts = path.split("/")
    matches = []

    for key, value in found.items():
        components = key.split("/")[-len(parts) :]

        if len(components) == len(parts) and all(
            component.endswith(part) for component, part in zip(components, parts)
        ):
            matches.append(value)

    if len(matches) != 1:
        sys.exit(
            f"Expected one performance graph section matching '{path}', found {len(matches)}"
        )

    return matches[0]


def search(pattern, output, description):
    """Return the first capture of pattern in output as a float"""
    match = pattern.search(output)

    if not match:
        sys.exit(f"Could not read the {description} from the run output")

    return float(match.group(1))


def convergence_metrics(output):
    """Return the nonlinear residual norms and the linear iteration count of each Newton step

    The two are read independently and paired by position: a Newton step's linear solve is the one
    reported between its own residual norm and the next, so the first norm belongs to the initial
    residual and the first iteration count to the solve that reduced it.
    """
    norms = [float(match.group(2)) for match in SNES_NORM.finditer(output)]
    solves = [
        (match.group(1), int(match.group(3))) for match in KSP_REASON.finditer(output)
    ]

    if not norms:
        sys.exit("Could not read any SNES function norm from the run output")

    if not solves:
        sys.exit("Could not read any linear solve iteration count from the run output")

    return norms, solves


def quadratic_ratios(norms):
    """Return the successive ratios r_{k+1} / r_k^2, whose boundedness is quadratic convergence"""
    return [
        norms[k + 1] / (norms[k] * norms[k])
        for k in range(len(norms) - 1)
        if norms[k] > 0
    ]


def convergence_sweep(args, extra):
    """Sweep the polynomial order and report iteration counts and nonlinear residual ratios"""
    input_file = args.input if args.input else CONVERGENCE_INPUT
    orders = args.orders if args.orders else DEFAULT_CONVERGENCE_ORDERS
    refine = args.refine if args.refine is not None else DEFAULT_CONVERGENCE_REFINE

    print(
        f"# {args.krylov.upper()}, ksp_rtol {args.ksp_rtol}, nl_rel_tol {args.snes_rtol}, "
        f"{refine}x{refine} mesh, Eisenstat-Walker "
        f"{'on' if args.eisenstat_walker else 'off'}"
    )
    print(
        f"{'p':>2} {'levels':>9} {'dofs':>7} {'newton':>6} {'linear its':>16} "
        f"{'r_(k+1)/r_k^2':>16}"
    )

    first = {}

    for order in orders:
        output = run_convergence(
            args.executable, input_file, order, refine, args, extra
        )

        dofs = int(search(DOFS, output, "degree of freedom count"))
        norms, solves = convergence_metrics(output)

        # A solve that stopped on its iteration cap rather than on the tolerance is marked, so that
        # a count read off a capped solve is never mistaken for a converged one
        counts = " ".join(
            f"{count}{'' if reason == 'converged' else '*'}" for reason, count in solves
        )
        ratios = " ".join(f"{ratio:.2e}" for ratio in quadratic_ratios(norms)[-3:])

        first[order] = solves[0][1]

        print(
            f"{order:>2} {LEVEL_ORDERS[order]:>9} {dofs:>7} {len(solves):>6} "
            f"{counts:>16} {ratios:>16}"
        )

    report_band(first)


def report_band(first):
    """Report the p-independence band: the first Newton step's linear iteration count against p

    The band is measured against the lowest order swept rather than against order one, because order
    one carries no coarser level and so has no hierarchy to measure.
    """
    if len(first) < 2:
        return

    orders = sorted(first)
    base, top = orders[0], orders[-1]
    # The acceptance band of the plan's Criterion 2: within 20 percent or three iterations,
    # whichever is the more forgiving at this count
    allowed = max(0.2 * first[base], 3)

    print(
        f"\np-independence: {first[base]} iterations at p = {base}, {first[top]} at p = {top}; "
        f"band is {first[base] + allowed:.0f} or fewer at p = {top} "
        f"({'MET' if first[top] <= first[base] + allowed else 'NOT MET'})"
    )


def timing_sweep(args, extra):
    """Sweep the polynomial order and report cache cost and storage against the assembled Jacobian"""
    input_file = args.input if args.input else DEFAULT_INPUT
    orders = args.orders if args.orders else DEFAULT_TIMING_ORDERS
    refine = args.refine if args.refine is not None else DEFAULT_TIMING_REFINE

    print(
        f"{'p':>2} {'dofs':>8} {'assembled(s)':>13} {'cache(s)':>10} {'speedup':>8} "
        f"{'CSR(MB)':>8} {'cache(MB)':>10} {'ratio':>6}"
    )

    for order in orders:
        output = run(args.executable, input_file, order, refine, extra)

        dofs = int(search(DOFS, output, "degree of freedom count"))
        nonzeros = search(NNZ, output, "assembled matrix nonzero count")
        cache_megabytes = search(CACHE_MB, output, "cache storage")

        found = sections(output)
        assembled_calls, assembled_time = section(
            found, "computeKokkosJacobian/KokkosKernel"
        )
        cache_calls, cache_time = section(found, "KokkosKernel/KokkosQpJacobianCache")

        assembled_per_call = assembled_time / assembled_calls
        cache_per_call = cache_time / cache_calls
        csr_megabytes = (nonzeros * BYTES_PER_NONZERO + dofs * BYTES_PER_ROW) / (
            1024 * 1024
        )

        print(
            f"{order:>2} {dofs:>8} {assembled_per_call:>13.4f} {cache_per_call:>10.4f} "
            f"{assembled_per_call / cache_per_call:>8.1f} {csr_megabytes:>8.1f} "
            f"{cache_megabytes:>10.1f} {csr_megabytes / cache_megabytes:>6.1f}"
        )


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--executable", default=DEFAULT_EXECUTABLE)
    parser.add_argument(
        "--mode",
        choices=["timing", "convergence"],
        default="timing",
        help="measure cache cost against the assembled Jacobian, or p-multigrid convergence",
    )
    # The input, the swept orders and the mesh size default per mode, so that each mode's documented
    # reproduction command carries only the arguments that mode actually varies
    parser.add_argument("--input", default=None)
    parser.add_argument(
        "--orders", type=int, nargs="+", default=None, help="orders to sweep"
    )
    parser.add_argument(
        "--refine",
        type=int,
        default=None,
        help="elements per side of the generated mesh",
    )
    parser.add_argument(
        "--krylov",
        choices=["gmres", "cg"],
        default="gmres",
        help="outer Krylov accelerator; cg is valid only where the linearization is symmetric",
    )
    parser.add_argument(
        "--ksp-rtol", type=float, default=1e-8, help="linear relative tolerance"
    )
    parser.add_argument(
        "--snes-rtol", type=float, default=1e-8, help="nonlinear relative tolerance"
    )
    parser.add_argument(
        "--ksp-max-it",
        type=int,
        default=400,
        help="linear iteration cap, held well below the PETSc default so that a sweep reports a "
        "hard case rather than spending the default cap on it",
    )
    parser.add_argument(
        "--eisenstat-walker",
        action="store_true",
        help="let the linear tolerance follow the nonlinear residual (-snes_ksp_ew)",
    )
    parser.add_argument(
        "extra",
        nargs="*",
        help="additional command line arguments passed to every case",
    )
    args = parser.parse_args()

    if args.mode == "timing":
        timing_sweep(args, args.extra)
    else:
        convergence_sweep(args, args.extra)


if __name__ == "__main__":
    main()
