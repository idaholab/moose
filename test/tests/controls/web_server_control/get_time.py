#!/usr/bin/env python3
from base_controller import *
import math

# This should be called by the test harness with the get_postprocessor.i
# input file to obtain changing postprocessor values from the web server
if __name__ == '__main__':

    # Helper for erroring of some values aren't close
    def expect_close(v1, v2):
        if not math.isclose(v1, v2):
            raise Exception(f'"{v1}" != "{v2}"')

    # Passed into the base controller to run the MooseControl
    def run_control(control):

        # Control through initialization
        control.wait('INITIAL')

        # Check simulation time
        pp_t = control.getPostprocessor('t')
        t  = control.getTime()
        expect_close(pp_t, t)

        # Check time step size
        pp_dt = control.getPostprocessor('dt')
        dt  = control.getTimeStepSize()
        expect_close(pp_dt, dt)

        # Tell MOOSE to continue with the solve
        control.setContinue()

        # Control through the timesteps
        for i in range(3):
            # Wait, where we shold be at TIMESTEP_END
            control.wait('TIMESTEP_END')

            pp_t = control.getPostprocessor('t')
            t  = control.getTime()
            expect_close(pp_t, t)

            pp_dt = control.getPostprocessor('dt')
            dt  = control.getTimeStepSize()
            expect_close(pp_dt, dt)

            # Tell MOOSE to continue with the solve
            control.setContinue()

    # Leverages base_controller.py to execute moose based on the
    # command from the test harness and instantiate the MooseControl
    base_controller('web_server', run_control)
