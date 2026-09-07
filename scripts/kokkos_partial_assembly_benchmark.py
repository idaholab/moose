#!/usr/bin/env python3
# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

"""Measure the cost of the Kokkos quadrature-point Jacobian cache against assembled-Jacobian cost.

The same input is run at a sequence of polynomial orders with the matrix-free operator enabled,
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

ORDER_NAMES = {1: "FIRST", 2: "SECOND", 3: "THIRD", 4: "FOURTH", 5: "FIFTH", 6: "SIXTH"}

# Performance graph rows are pipe-delimited and indent the section name by its depth in the call
# tree: leading whitespace, section name, call count, then the self and total groups of
# (seconds, average, percent, megabytes)
ROW = re.compile(r"^\|(\s*)(\S.*?)\s*\|" + r"\s*([0-9.eE+-]+)\s*\|" * 9)

DOFS = re.compile(r"Num DOFs:\s*(\d+)")
NNZ = re.compile(r"total: nonzeros=(\d+)")
CACHE_MB = re.compile(r"quadrature-point Jacobian cache storage:\s*([0-9.eE+-]+) MB")

# A PETSc AIJ matrix stores one scalar and one column index per nonzero, plus one index per row
BYTES_PER_NONZERO = 8 + 4
BYTES_PER_ROW = 4


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

    result = subprocess.run(command, capture_output=True, text=True)

    if result.returncode:
        sys.exit(f"Case failed (order {order}):\n{result.stdout}\n{result.stderr}")

    return result.stdout


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


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--executable", default=DEFAULT_EXECUTABLE)
    parser.add_argument("--input", default=DEFAULT_INPUT)
    parser.add_argument(
        "--orders", type=int, nargs="+", default=[1, 2, 3, 4], help="polynomial orders to sweep"
    )
    parser.add_argument(
        "--refine", type=int, default=32, help="elements per side of the generated mesh"
    )
    parser.add_argument(
        "extra", nargs="*", help="additional command line arguments passed to every case"
    )
    args = parser.parse_args()

    print(
        f"{'p':>2} {'dofs':>8} {'assembled(s)':>13} {'cache(s)':>10} {'speedup':>8} "
        f"{'CSR(MB)':>8} {'cache(MB)':>10} {'ratio':>6}"
    )

    for order in args.orders:
        output = run(args.executable, args.input, order, args.refine, args.extra)

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


if __name__ == "__main__":
    main()
