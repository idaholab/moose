#!/usr/bin/env python3
import sys
from base_controller import base_controller

# This should be called by the test harness with the get_postprocessor.i
if __name__ == '__main__':

    # Get requested timestep to terminate at
    num_steps = int(sys.argv[1])

    # Passed into the base controller to run the MooseControl
    def run_control(control):
        # To account for the web server control executing on initial
        control.wait('INITIAL')
        control.setContinue()

        # Go through num_steps, then terminate
        for t in range(num_steps):
            # Wait, where we should be at TIMESTEP_BEGIN
            control.wait('TIMESTEP_BEGIN')
            # Tell MOOSE to continue with the solve
            if t < num_steps - 1:
                control.setContinue()
            # Tell MOOSE to terminate the solve after this timestep
            else:
                control.setTerminate()

    # Leverages base_controller.py to execute moose based on the
    # command from the test harness and instantiate the MooseControl
    base_controller('web_server', run_control)
