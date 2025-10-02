from fmpy import simulate_fmu, extract, read_model_description, instantiate_fmu
from fmpy.simulation import apply_start_values
import numpy as np
import pandas as pd
import logging
import time
import shutil
import test.tests.controls.moose_fmu.moose_fmu_controller as moose_fmu_controller

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

def simulate_moose_fmu(moose_filename, t0, t1, dt, flag, cmd,*, debug_logging=True):

# time step size is set during fmu build process and can't not be adjusted during run time,
# use step by step approach if different time step size is needed
# If you notice the fmu_time and moose_time is not synced in the last time step, it is because simulate_fmu doesn't call do_step
# at stop_time, use step by step approach if sync at end time really matters to you, or try to use smaller dt for fmu, it will
# bring fmu_time closer to moose_time
    result = simulate_fmu(
        moose_filename,
        start_time=t0,
        stop_time=t1,
        step_size=dt,
        start_values={
        'flag':             flag,
        'moose_command': cmd,
        'server_name':      'web_server',
        'max_retries':      10},
        debug_logging=debug_logging,
        output      = ['time','moose_time', 'diffused', "rep_value"],
        set_stop_time=False
    )

    df = pd.DataFrame(result)

    df.to_csv("run_fmu.csv", index=False)

    return result

def moose_fmu_step_by_step(
    moose_filename: str,
    t0: float,
    t1: float,
    dt: float,
    flag: str,
    cmd:str,
    *,
    rtol: float = 1e-6,
    atol: float = 1e-9,
    time_tol: float | None = None,        # None -> auto dt/2
    step_csv: str = "run_fmu_step_by_step.csv"
):
    """
    Manual FMI 2.0 run + comparison with baseline CSV produced by simulate_moose_fmu().
    """
    if time_tol is None:
        time_tol = 1e-15

    unzipdir = extract(moose_filename)
    md = read_model_description(unzipdir)
    fmu = instantiate_fmu(unzipdir=unzipdir, model_description=md)

    try:
        vrs = {v.name: v.valueReference for v in md.modelVariables}

        # --- Initialization ---
        fmu.instantiate()
        fmu.setupExperiment(startTime=t0, stopTime=t1)
        fmu.enterInitializationMode()

        apply_start_values(
            fmu=fmu,
            model_description=md,
            start_values={
                "flag":             flag,
                'moose_command':    cmd,
                "server_name":      "web_server",
                "max_retries":      10,
            },
        )

        fmu.exitInitializationMode()
        # ----------------------

        # --- Step loop ---
        rows = []
        t = t0
        while t <= t1 :
            fmu.doStep(currentCommunicationPoint=t, communicationStepSize=dt)

            moose_time = fmu.getReal([vrs["moose_time"]])[0]
            diffused   = fmu.getReal([vrs["diffused"]])[0]
            rep_value = fmu.getReal([vrs["rep_value"]])[0]
            print(f"fmu_time={t:.3f} → moose_time={moose_time:.6f} → diffused={diffused:.6f}→ rep_value={rep_value:.6f}")
            rows.append((t, moose_time, diffused, rep_value))

            t = min(t + dt, t1 + time_tol)

        result = np.array(
            rows,
            dtype=[("time", np.float64), ("moose_time", np.float64), ("diffused", np.float64), ("rep_value", np.float64)],
        )

        # Save our step-by-step results
        df_step = pd.DataFrame(result)
        df_step.to_csv(step_csv, index=False)

        return result

    finally:
        # Cleanup
        try:
            fmu.terminate()
        except Exception:
            pass
        try:
            fmu.freeInstance()
        except Exception:
            pass
        shutil.rmtree(unzipdir, ignore_errors=True)

def print_result(result):

    fmu_time  = result["time"]
    dt        = result["moose_time"]
    diff_u    = result["diffused"]
    rep_value = result["rep_value"]

    for ti, di, diff, rep in zip(fmu_time, dt, diff_u, rep_value):
         print(f"fmu_time={ti:.1f} → moose_time={di:.5f} → diffused={diff:.5f}→ rep_value={rep:.5f} ")

def main():
    root_logger = logging.getLogger()
    root_logger.setLevel(logging.DEBUG if FMU_DEBUG_LOGGING else logging.INFO)
    if FMU_DEBUG_LOGGING:
        logger.debug("FMU debug logging is enabled")

    # Provide your own MOOSE command for non testing senarios
    cmd = moose_fmu_controller.test_controller()

    t0, t1, dt = 0, 1, 0.5
    moose_filename = 'MooseTest.fmu'
    flag = "MULTIAPP_FIXED_POINT_END"
    result1 = simulate_moose_fmu(moose_filename, t0, t1, dt, flag, cmd, debug_logging=FMU_DEBUG_LOGGING)
    logger.info("Results from simulate_fmu:")
    print_result(result1)

    time.sleep(2)
    logger.info("Start the second moose run after 2s")
    result2 = moose_fmu_step_by_step(moose_filename, t0, t1, dt, flag, cmd)
    logger.info("Results from fmu step by step:")
    print_result(result2)



if __name__ == "__main__":
    main()



