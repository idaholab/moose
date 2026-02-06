#!/usr/bin/env python3
# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

from moosefmu import configure_fmu_logging
import moose_fmu_tester_pyfmi

"""
Drive a MOOSE-generated FMU step-by-step (manual do_step loop).

Why step-by-step?
-----------------
- You control the communication step (dt) and the timing of information
  exchanges precisely at t0 + n*dt.
- Useful when you need exact alignment at the stop time (t1), or when you want
  to interleave custom logic (e.g., fixed-point coupling, data logging, or
  external synchronization) between steps.
- The FMU's *internal* integration step is fixed at FMU build time in MOOSE.
  Step-by-step does not change that internal dt, but it *does* control how
  often the co-simulation master exchanges data (do_step calls).
"""

if __name__ == "__main__":

    # Toggle this flag to switch between INFO and DEBUG logging for the script and FMU
    FMU_DEBUG_LOGGING = True

    logger = configure_fmu_logging(debug=FMU_DEBUG_LOGGING, logger_name=__name__)

    # Provide your own MOOSE command for non testing senarios
    cmd = moose_fmu_tester_pyfmi.test_controller()

    t0, t1, dt = 0, 1, 0.5
    moose_filename = "MooseTest.fmu"
    flag = "MULTIAPP_FIXED_POINT_END"

    result2 = moose_fmu_tester_pyfmi.moose_fmu_step_by_step(
        moose_filename, t0, t1, dt, flag, cmd
    )
    logger.info("Results from fmu step by step:")
    moose_fmu_tester_pyfmi.print_result(result2)
