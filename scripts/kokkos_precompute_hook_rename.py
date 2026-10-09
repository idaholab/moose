#!/usr/bin/env python3
# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

"""
Rename Kokkos precompute hooks to names stating the test function quantity.

Kokkos kernels and integrated BCs derived from KernelValue, IntegratedBCValue,
VectorKernelValue, VectorIntegratedBCValue, or TimeKernelValue return the
coefficient of the test function value from precomputeQpResidual() and its
Jacobian counterparts, which become precomputeQpResidualValue() and so on.
Objects derived from KernelGrad or VectorKernelGrad return the coefficient of
the test function gradient, which become precomputeQpResidualGrad() and so on.

Usage: kokkos_precompute_hook_rename.py {value,grad} FILE [FILE ...]
"""

import argparse
import re

HOOKS = (
    "precomputeQpOffDiagJacobianScalar",
    "precomputeQpOffDiagJacobian",
    "precomputeQpJacobian",
    "precomputeQpResidual",
)


def rename(text, suffix):
    """Append suffix to every unsuffixed precompute hook name in text."""
    pattern = re.compile(r"\b(" + "|".join(HOOKS) + r")\b(?!Value|Grad|Curl|Scalar)")
    return pattern.sub(lambda match: match.group(1) + suffix, text)


def main():
    """Rename the hooks in the files given on the command line."""
    parser = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    parser.add_argument(
        "quantity", choices=("value", "grad"), help="Test function quantity"
    )
    parser.add_argument("files", nargs="+", help="Files to rewrite in place")
    args = parser.parse_args()

    suffix = args.quantity.capitalize()
    for path in args.files:
        with open(path) as f:
            text = f.read()
        renamed = rename(text, suffix)
        if renamed != text:
            with open(path, "w") as f:
                f.write(renamed)


if __name__ == "__main__":
    main()
