#!/usr/bin/env python3
# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

import contextlib
import shutil

import numpy as np
import pandas as pd
from fmpy import extract, instantiate_fmu, read_model_description
from fmpy.simulation import apply_start_values
from moosefmu import set_real


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
    time_tol: float | None = None,  # None -> auto dt/2
    step_csv: str = "run_fmu_step_by_step.csv",
):
    """Manual FMI 2.0 run + comparison with baseline CSV produced by simulate_moose_fmu()."""
    if time_tol is None:
        time_tol = 1e-15

    moose_model = extract(moose_filename)
    md = read_model_description(moose_model)
    fmu = instantiate_fmu(unzipdir=moose_model, model_description=md)

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

            if t <= 1000:
                set_real(fmu, vrs, "mfr_in", 0.5)
                print("set mfr_in to 0.5")

            air_heatrate = fmu.getReal([vrs["air_heatrate"]])[0]
            print(
                f"fmu_time={t:.3f} -> moose_time={moose_time:.6f} -> air_heatrate={air_heatrate:.6f}"
            )
            rows.append((t, moose_time, air_heatrate))

            t = min(t + dt, t1 + time_tol)

        result = np.array(
            rows,
            dtype=[
                ("time", np.float64),
                ("moose_time", np.float64),
                ("air_heatrate", np.float64)
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
        shutil.rmtree(moose_model, ignore_errors=True)




if __name__ == "__main__":

    # Toggle this flag to switch between INFO and DEBUG logging for the script and FMU

    t0, t1, dt = 0, 6000, 2000
    moose_filename = "DomeTest.fmu"
    flag = "MULTIAPP_FIXED_POINT_END"
    cmd = "../../combined-opt -i dome_rss.i"
    result = moose_fmu_step_by_step(
        moose_filename, t0, t1, dt, flag, cmd)

    fmu_time = result["time"]
    dt = result["moose_time"]
    air_heatrate = result["air_heatrate"]

    for ti, di, diff, rep in zip(fmu_time, dt, air_heatrate):
        print(
            f"fmu_time={ti:.1f} -> moose_time={di:.5f} -> air_heatrate={diff:.5f}"
        )
