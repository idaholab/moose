#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://mooseframework.inl.gov
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from testmoosecontrol import TestMooseControl, expect_close

# This should be called by the test harness with the get_postprocessor.i
# input file to obtain changing postprocessor values from the web server
if __name__ == '__main__':
    with TestMooseControl('web_server') as control:
        # Control through initialization
        control.wait('INITIAL')

        # Check simulation time
        pp_t = control.get_postprocessor('t')
        t  = control.get_time()
        expect_close(pp_t, t)

        # Check time step size
        pp_dt = control.get_postprocessor('dt')
        dt  = control.get_dt()
        expect_close(pp_dt, dt)

        # Tell MOOSE to continue with the solve
        control.set_continue()

        # Control through the timesteps
        for i in range(3):
            # Wait, where we shold be at TIMESTEP_END
            control.wait('TIMESTEP_END')

            pp_t = control.get_postprocessor('t')
            t  = control.get_time()
            expect_close(pp_t, t)

            pp_dt = control.get_postprocessor('dt')
            dt  = control.get_dt()
            expect_close(pp_dt, dt)

            # Tell MOOSE to continue with the solve
            control.set_continue()

