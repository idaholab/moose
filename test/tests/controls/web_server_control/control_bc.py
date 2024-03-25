#!/usr/bin/env python3
from base_controller import *

# This should be called by the test harness with the control_bc.i
# input file to increase the value of BCs/left/value in a
# transient simulation to make sure that changing said values
# is parallel consisten
if __name__ == '__main__':
    def run_control(control):
      # Left boundary condition value that we're going to increase
      bc_value = 0

      # Running 4 timesteps (control_bc.i:Transient/num_steps)
      num_steps = 4

      # Control through the 4 timesteps
      for i in range(num_steps):
        # Wait for TIMESTEP_BEGIN, which should be the only flag
        # the webserver is listening on
        control.wait('TIMESTEP_BEGIN')

        # Increase the left BC value
        bc_value += 0.1
        control.setControllableReal('BCs/left/value', bc_value)

        # Tell MOOSE to continue with the solve
        control.setContinue()

    # Leverages base_controller.py to execute moose based on the
    # command from the test harness and instantiate the MooseControl
    base_controller('web_server', run_control)
