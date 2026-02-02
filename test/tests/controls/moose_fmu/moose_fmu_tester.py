# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

import contextlib
import os
import shutil
import sys

import numpy as np
import pandas as pd
from fmpy import extract, instantiate_fmu, read_model_description, simulate_fmu
from fmpy.simulation import apply_start_values


# Helper for testing the MOOSE FMU via the RunApp command_proxy option
def test_controller(user_cmd: str | None = None):
    # Get the command we should run
    # You'll hit this if you don't run a RunApp-derived Tester or
    # don't run it with the "command_proxy" option
    if user_cmd:
        return user_cmd
    RUNAPP_COMMAND = os.environ.get("RUNAPP_COMMAND")
    if RUNAPP_COMMAND is None:
        sys.exit("Missing expected command variable RUNAPP_COMMAND")

    return RUNAPP_COMMAND


def simulate_moose_fmu(moose_filename, t0, t1, dt, flag, cmd, *, debug_logging=True):

    result = simulate_fmu(
        moose_filename,
        start_time=t0,
        stop_time=t1,
        step_size=dt,
        start_values={
            "flag": flag,
            "moose_command": cmd,
            "server_name": "web_server",
            "max_retries": 10,
        },
        debug_logging=debug_logging,
        output=["time", "moose_time", "diffused", "rep_value"],
        set_stop_time=False,
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
    cmd: str,
    *,
    rtol: float = 1e-6,
    atol: float = 1e-9,
    time_tol: float | None = None,  # None -> 1e-15
    step_csv: str = "run_fmu_step_by_step.csv",
):
    """Manual FMI 2.0 run + comparison with baseline CSV produced by simulate_moose_fmu()."""
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
                "flag": flag,
                "moose_command": cmd,
                "server_name": "web_server",
                "max_retries": 10,
            },
        )

        fmu.exitInitializationMode()
        # ----------------------

        # --- Step loop ---
        rows = []
        t = t0
        while t <= t1:
            fmu.doStep(currentCommunicationPoint=t, communicationStepSize=dt)

            moose_time = fmu.getReal([vrs["moose_time"]])[0]
            diffused = fmu.getReal([vrs["diffused"]])[0]
            rep_value = fmu.getReal([vrs["rep_value"]])[0]
            print(
                f"fmu_time={t:.3f} -> moose_time={moose_time:.6f} -> diffused={diffused:.6f} -> rep_value={rep_value:.6f}"
            )
            rows.append((t, moose_time, diffused, rep_value))

            t = min(t + dt, t1 + time_tol)

        result = np.array(
            rows,
            dtype=[
                ("time", np.float64),
                ("moose_time", np.float64),
                ("diffused", np.float64),
                ("rep_value", np.float64),
            ],
        )

        # Save our step-by-step results
        df_step = pd.DataFrame(result)
        df_step.to_csv(step_csv, index=False)

        return result

    finally:
        # Cleanup
        with contextlib.suppress(Exception):
            fmu.terminate()
        with contextlib.suppress(Exception):
            fmu.freeInstance()
        shutil.rmtree(unzipdir, ignore_errors=True)


def moose_fmu_time(
    moose_filename: str,
    t0: float,
    t1: float,
    dt: float,
    flag: str,
    cmd: str,
    *,
    time_tol: float | None = None,
    step_csv: str = "run_fmu_time.csv",
):
    """Manual FMI 2.0 run for testing fmu moose time sync."""
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
                "flag": flag,
                "moose_command": cmd,
                "server_name": "web_server",
                "max_retries": 10,
            },
        )

        fmu.exitInitializationMode()
        # ----------------------

        # --- Step loop ---
        rows = []
        t = t0
        while t <= t1:
            fmu.doStep(currentCommunicationPoint=t, communicationStepSize=dt)

            moose_time = fmu.getReal([vrs["moose_time"]])[0]

            rows.append((t, moose_time))

            t = min(t + dt, t1 + time_tol)

        result = np.array(
            rows,
            dtype=[("time", np.float64), ("moose_time", np.float64)],
        )

        # Save our step-by-step results
        df_step = pd.DataFrame(result)
        df_step.to_csv(step_csv, index=False)

        return result

    finally:
        # Cleanup
        with contextlib.suppress(Exception):
            fmu.terminate()
        with contextlib.suppress(Exception):
            fmu.freeInstance()
        shutil.rmtree(unzipdir, ignore_errors=True)


def print_result(result):

    fmu_time = result["time"]
    dt = result["moose_time"]
    diff_u = result["diffused"]
    rep_value = result["rep_value"]

    for ti, di, diff, rep in zip(fmu_time, dt, diff_u, rep_value):
        print(
            f"fmu_time={ti:.1f} -> moose_time={di:.5f} -> diffused={diff:.5f} -> rep_value={rep:.5f} "
        )
