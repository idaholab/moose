#!/usr/bin/env python3
from base_controller import *

def run_control(control):
    # Number of timesteps we will step through
    num_timesteps = 10

    # For incrementing BCs/left/value
    left_BC_value = 0

    # Controlling on each TIMESTEP_BEGIN
    for t in range(num_timesteps):
        # Wait for TIMESTEP_BEGIN
        control.wait()
        expect_equal(control.getWaitingFlag(), 'TIMESTEP_BEGIN')
        control.wait('TIMESTEP_BEGIN') # extraneous, but tests with a flag

        # Increment the BC value by 1
        left_BC_value += 0.1
        control.setControllableReal('BCs/left/value', left_BC_value)

        # And continue to the next timestep
        control.setContinue()

if __name__ == '__main__':
    # To be used to run base_controller.i
    base_controller('web_server', run_control)
