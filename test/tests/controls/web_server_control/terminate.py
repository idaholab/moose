#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://mooseframework.inl.gov
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import sys

from testmoosecontrol import TestMooseControl

# This should be called by the test harness with the get_postprocessor.i
if __name__ == '__main__':
    # Get requested timestep to terminate at
    num_steps = int(sys.argv[1])

    # Passed into the base controller to run the MooseControl
    with TestMooseControl('web_server') as control:
        # To account for the web server control executing on initial
        control.wait('INITIAL')
        control.set_continue()

        # Go through num_steps, then terminate
        for t in range(num_steps):
            # Wait, where we should be at TIMESTEP_BEGIN
            control.wait('TIMESTEP_BEGIN')
            # Tell MOOSE to continue with the solve
            if t < num_steps - 1:
                control.set_continue()
            # Tell MOOSE to terminate the solve after this timestep
            else:
                control.set_terminate()
