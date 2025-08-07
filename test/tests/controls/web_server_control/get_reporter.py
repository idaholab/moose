#!/usr/bin/env python3
from base_controller import *
import json
import math

# This should be called by the test harness with the get_postprocessor.i
# input file to obtain changing postprocessor values from the web server
if __name__ == '__main__':
    # Open the gold file associated with this test, which contains the
    # results of the reporter values that we're trying to read from
    with open('gold/get_reporter_out.json', mode ='r') as file:
        data = json.load(file)

    nsteps = len(data['time_steps'])
    gold = {}
    for obj_name, entry in data['reporters'].items():
        for val_name in entry['values'].keys():
            rname = f'{obj_name}/{val_name}'
            gold[rname] = []
            for i in range(nsteps):
                gold[rname].append(data['time_steps'][i][obj_name][val_name])

    # Helper for erroring of some values aren't close
    def expect_close(v1, v2):
        if isinstance(v1, int):
            v1 = float(v1)
        if isinstance(v2, int):
            v2 = float(v2)

        if type(v1) != type(v2):
            raise TypeError(f'{v1} is type {type(v1)} and {v2} is {type(v2)}.')

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
            raise TypeError(f'Cannot compare {type(v1)}s')

        if not success:
            raise ValueError(f'"{v1}" != "{v2}"')
        return success

    # Passed into the base controller to run the MooseControl
    def run_control(control):
        # Wait until the server is ready, where it should be at INITIAL
        control.wait('INITIAL')

        # Get the initial value of the reporters and compare to the gold
        for rep, gold_values in gold.items():
            value = control.getReporterValue(rep)
            expect_close(value, gold_values[0])

        # Tell MOOSE to continue with the solve
        control.setContinue()

        # Control through the timesteps (determined by length of the gold file)
        for t in range(nsteps - 1):
            # Wait, where we shold be at TIMESTEP_BEGIN
            control.wait('TIMESTEP_END')

            # Get the current value of the postprocessor and compare to the gold
            for rep, gold_values in gold.items():
                value = control.getReporterValue(rep)
                expect_close(value, gold_values[t + 1])

            # Tell MOOSE to continue with the solve
            control.setContinue()

    # Leverages base_controller.py to execute moose based on the
    # command from the test harness and instantiate the MooseControl
    base_controller('web_server', run_control)
