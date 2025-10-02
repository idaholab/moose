import logging
import moose_fmu_tester

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

    # Configure root logger
    logging.basicConfig(
        level=logging.DEBUG if FMU_DEBUG_LOGGING else logging.INFO,
        format='%(asctime)s [%(levelname)s] %(name)s: %(message)s'
    )
    logger = logging.getLogger(__name__)

    # Keep urllib3 connection pool noise suppressed unless explicitly debugging
    urllib3_logger = logging.getLogger("urllib3.connectionpool")
    urllib3_logger.propagate = False
    urllib3_logger.disabled = True

    if FMU_DEBUG_LOGGING:
        logging.getLogger("Moose2FMU").setLevel(logging.DEBUG)
    else:
        logging.getLogger("Moose2FMU").setLevel(logging.INFO)

    root_logger = logging.getLogger()
    root_logger.setLevel(logging.DEBUG if FMU_DEBUG_LOGGING else logging.INFO)
    if FMU_DEBUG_LOGGING:
        logger.debug("FMU debug logging is enabled")

    # Provide your own MOOSE command for non testing senarios
    cmd = moose_fmu_tester.test_controller()

    t0, t1, dt = 0, 1, 0.5
    moose_filename = 'MooseTest.fmu'
    flag = "MULTIAPP_FIXED_POINT_END"

    result2 = moose_fmu_tester.moose_fmu_step_by_step(moose_filename, t0, t1, dt, flag, cmd)
    logger.info("Results from fmu step by step:")
    moose_fmu_tester.print_result(result2)



