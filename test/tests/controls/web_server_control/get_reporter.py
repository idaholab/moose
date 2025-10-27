#!/usr/bin/env python3
# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

import json
import math

from testmoosecontrol import TestMooseControl

# This should be called by the test harness with the get_postprocessor.i
# input file to obtain changing postprocessor values from the web server
if __name__ == "__main__":
    # Open the gold file associated with this test, which contains the
    # results of the reporter values that we're trying to read from
    with open("gold/get_reporter_out.json", "r", encoding="utf-8") as f:
        data = json.load(f)

    nsteps = len(data["time_steps"])
    gold = {}
    for obj_name, entry in data["reporters"].items():
        for val_name in entry["values"]:
            rname = f"{obj_name}/{val_name}"
            gold[rname] = []
            for i in range(nsteps):
                gold[rname].append(data["time_steps"][i][obj_name][val_name])

    def expect_close(v1, v2):
        """Error if some values aren't close."""
        if isinstance(v1, int):
            v1 = float(v1)
        if isinstance(v2, int):
            v2 = float(v2)

        if not isinstance(v1, type(v2)):
            raise TypeError(f"{v1} is type {type(v1)} and {v2} is {type(v2)}.")

        # Dicts (points)
        if isinstance(v1, dict):
            success = all([k in v2 for k in v1])
            success = success and all([k in v1 for k in v2])
            for k in v1:
                success = success and expect_close(v1[k], v2[k])
        # List (Vector, matrix, or vector of vectors)
        elif isinstance(v1, list):
            success = len(v1) == len(v2)
            for v1i, v2i in zip(v1, v2):
                success = success and expect_close(v1i, v2i)
        # Float
        elif isinstance(v1, float):
            success = math.isclose(v1, v2)
        # String
        elif isinstance(v1, str):
            success = v1 == v2
        # Unknown type
        else:
            raise TypeError(f"Cannot compare {type(v1)}s")

        if not success:
            raise ValueError(f'"{v1}" != "{v2}"')
        return success

    with TestMooseControl("web_server") as control:
        # Wait until the server is ready, where it should be at INITIAL
        control.wait("INITIAL")

        # Get the initial value of the reporters and compare to the gold
        for rep, gold_values in gold.items():
            value = control.get_reporter(rep)
            expect_close(value, gold_values[0])

        # Tell MOOSE to continue with the solve
        control.set_continue()

        # Control through the timesteps (determined by length of the gold file)
        for t in range(nsteps - 1):
            # Wait, where we shold be at TIMESTEP_BEGIN
            control.wait("TIMESTEP_END")

            # Get the current value of the postprocessor and compare to the gold
            for rep, gold_values in gold.items():
                value = control.get_reporter(rep)
                expect_close(value, gold_values[t + 1])

            # Tell MOOSE to continue with the solve
            control.set_continue()
