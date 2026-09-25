#!/usr/bin/env python3
# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

"""Test that ParsedPostprocessor 'expression' is a controllable parameter."""

import os
import sys

sys.path.append(
    os.path.join(
        os.path.dirname(__file__), "..", "..", "controls", "web_server_control"
    )
)

from testmoosecontrol import TestMooseControl

# This should be called by the test harness with the parsed_pp_control.i
# input file to control the ParsedPostprocessor 'expression' parameter.
# The resulting 'parsed' postprocessor values are verified by the
# CSVDiff comparison against the gold file.
if __name__ == "__main__":
    with TestMooseControl("web_server") as control:
        # Control through initialization
        control.wait("INITIAL")
        control.set_continue()

        # Control through the timesteps, changing 'expression' from "c + t"
        # to "c + t < n" where n increases by 50 each timestep
        n = 0
        for _ in range(2):
            control.wait("TIMESTEP_BEGIN")

            control.set_string(
                "Postprocessors/parsed/expression", f"10 * (c + t < {n})"
            )

            n += 50
            control.set_continue()
