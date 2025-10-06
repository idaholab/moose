#!/usr/bin/env python3

from MooseFMU import configure_fmu_logging
import moose_fmu_tester

"""
Run a basic simulation of a MOOSE-generated FMU via fmpy.simulate_fmu.

Notes on time stepping and end-time alignment
--------------------------------------------
- The FMU's integration step (dt) is fixed at FMU build time in MOOSE and cannot
  be changed at runtime when using fmpy.simulate_fmu.

- If you need a different dt or exact alignment at the final time (t1), either:
  (a) rebuild the FMU with the desired dt with DefaultExperiment, or
  (b) drive the FMU step-by-step (manual do_step loop) instead of simulate_fmu.

- simulate_fmu integrates *up to* stop_time but does not necessarily perform a
  do_step exactly at stop_time. As a result, the last reported fmu_time can be
  slightly less than stop_time. If exact synchronization at t1 matters, use the
  step-by-step approach or choose a smaller dt so fmu_time is closer to t1.
"""


if __name__ == "__main__":

    # Toggle this flag to switch between INFO and DEBUG logging for the script and FMU
    FMU_DEBUG_LOGGING = True

    logger = configure_fmu_logging(debug=FMU_DEBUG_LOGGING, logger_name=__name__)

    # Provide your own MOOSE command for non testing senarios
    cmd = moose_fmu_tester.test_controller()

    t0, t1, dt = 0, 1, 0.5
    moose_filename = 'MooseTest.fmu'
    flag = "MULTIAPP_FIXED_POINT_END"
    result1 = moose_fmu_tester.simulate_moose_fmu(moose_filename, t0, t1, dt, flag, cmd, debug_logging=FMU_DEBUG_LOGGING)
    logger.info("Results from simulate_fmu:")
    moose_fmu_tester.print_result(result1)




