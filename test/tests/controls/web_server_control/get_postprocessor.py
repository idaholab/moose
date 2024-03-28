#!/usr/bin/env python3
from base_controller import *
import csv
import math

# This should be called by the test harness with the get_postprocessor.i
# input file to obtain changing postprocessor values from the web server
if __name__ == '__main__':
    # Open the gold file associated with this test, which contains the
    # results of the postprocessor that we're trying to read from
    with open('gold/get_postprocessor_out.csv', mode ='r') as file:
        gold = []
        for row in list(csv.reader(file))[1:]:
            gold.append(float(row[1]))

    # Helper for erroring of some values aren't close
    def expect_close(v1, v2):
        if not math.isclose(v1, v2):
            raise Exception(f'"{v1}" != "{v2}"')

    # Passed into the base controller to run the MooseControl
    def run_control(control):
        # Wait until the server is ready, where it should be at INITIAL
        control.wait('INITIAL')

        # Get the initial value of the postprocessor and compare to the gold
        value = control.getPostprocessor('t')
        expect_close(value, gold[0])

        # Tell MOOSE to continue with the solve
        control.setContinue()

        # Control through the timesteps (determined by length of the gold file)
        for t in range(len(gold) - 1):
            # Wait, where we shold be at TIMESTEP_BEGIN
            control.wait('TIMESTEP_BEGIN')

            # Get the current value of the postprocessor and compare to the gold
            value = control.getPostprocessor('t')
            expect_close(value, gold[t])

            # Tell MOOSE to continue with the solve
            control.setContinue()

    # Leverages base_controller.py to execute moose based on the
    # command from the test harness and instantiate the MooseControl
    base_controller('web_server', run_control)
