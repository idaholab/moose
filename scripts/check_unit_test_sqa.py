#!/usr/bin/env python3
# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details

import argparse
import os
import sys

MOOSE_DIR = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
sys.path.insert(0, os.path.join(MOOSE_DIR, "python"))

from moosesqa import check_unit_test_sqa  # noqa: E402


def main():
    parser = argparse.ArgumentParser(
        description="Check C++ GoogleTests for SQA traceability metadata."
    )
    parser.add_argument(
        "--root",
        default=MOOSE_DIR,
        help="repository root to check (default: %(default)s)",
    )
    parser.add_argument(
        "--legacy-manifest",
        help="legacy manifest path (default: python/moosesqa/legacy_unit_tests.yml)",
    )
    args = parser.parse_args()

    diagnostics = check_unit_test_sqa(args.root, legacy_manifest=args.legacy_manifest)
    for diagnostic in diagnostics:
        print(
            "{}:{}: {}".format(diagnostic.filename, diagnostic.line, diagnostic.message)
        )
    if diagnostics:
        print("{} unit-test SQA error(s)".format(len(diagnostics)))
        return 1

    print("All logical GoogleTests have SQA metadata or a legacy entry.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
