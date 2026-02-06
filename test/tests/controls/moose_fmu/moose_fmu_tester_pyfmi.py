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
import sys

import numpy as np
import pandas as pd
from pyfmi import load_fmu


def test_controller(user_cmd: str | None = None):
    """Return the command used to launch MOOSE, or exit if it is unavailable."""
    # Get the command we should run
    # You'll hit this if you don't run a RunApp-derived Tester or
    # don't run it with the "command_proxy" option
    if user_cmd:
        return user_cmd
    RUNAPP_COMMAND = os.environ.get("RUNAPP_COMMAND")

    if RUNAPP_COMMAND is None:
        sys.exit("Missing expected command variable RUNAPP_COMMAND")

    return RUNAPP_COMMAND


def _load_moose_fmu(moose_filename: str, *, debug_logging: bool):
    if debug_logging:
        return load_fmu(moose_filename, log_level=7)
    return load_fmu(moose_filename)


def _initialize_moose_fmu(model, t0: float, t1: float, flag: str, cmd: str) -> None:
    model.setup_experiment(start_time=t0, stop_time=t1)
    model.enter_initialization_mode()
    model.set("flag", flag)
    model.set("moose_command", cmd)
    model.set("server_name", "web_server")
    model.set("max_retries", 10)
    model.exit_initialization_mode()


def _finalize_moose_fmu(model) -> None:
     with contextlib.suppress(Exception):
         model.terminate()


def simulate_moose_fmu(moose_filename, t0, t1, dt, flag, cmd, *, debug_logging=True):
    """Run the FMU with a fixed step size and write results to run_fmu.csv."""
    model = _load_moose_fmu(moose_filename, debug_logging=debug_logging)

    try:
        _initialize_moose_fmu(model, t0, t1, flag, cmd)

        rows = []
        t = t0
        while t <= t1:
            model.do_step(current_t=t, step_size=dt)

            moose_time = float(model.get("moose_time"))
            diffused = float(model.get("diffused"))
            rep_value = float(model.get("rep_value"))
            rows.append((t, moose_time, diffused, rep_value))

            t = t + dt

        result = np.array(
            rows,
            dtype=[
                ("time", np.float64),
                ("moose_time", np.float64),
                ("diffused", np.float64),
                ("rep_value", np.float64),
            ],
        )

        df = pd.DataFrame(result)
        df.to_csv("run_fmu.csv", index=False)
        return result
    finally:
        _finalize_moose_fmu(model)


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
    """
    Manual FMI 2.0 run.

    Comparison with baseline CSV produced
    by simulate_moose_fmu().
    """
    if time_tol is None:
        time_tol = 1e-15

    model = _load_moose_fmu(moose_filename, debug_logging=True)

    try:
        _initialize_moose_fmu(model, t0, t1, flag, cmd)

        # --- Step loop ---
        rows = []
        t = t0
        while t <= t1:
            model.do_step(current_t=t, step_size=dt)

            moose_time = float(model.get("moose_time"))
            diffused = float(model.get("diffused"))
            rep_value = float(model.get("rep_value"))
            print(
                f"fmu_time={t:.3f} -> moose_time={moose_time:.6f} -> "
                "diffused={diffused:.6f} -> rep_value={rep_value:.6f}"
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
        _finalize_moose_fmu(model)


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

    model = _load_moose_fmu(moose_filename, debug_logging=True)

    try:
        _initialize_moose_fmu(model, t0, t1, flag, cmd)

        # --- Step loop ---
        rows = []
        t = t0
        while t <= t1:
            model.do_step(current_t=t, step_size=dt)

            moose_time = float(model.get("moose_time"))

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
        _finalize_moose_fmu(model)


def print_result(result):
    """Pretty-print FMU time, moose_time, diffused, and rep_value rows."""
    fmu_time = result["time"]
    dt = result["moose_time"]
    diff_u = result["diffused"]
    rep_value = result["rep_value"]

    for ti, di, diff, rep in zip(fmu_time, dt, diff_u, rep_value):
        print(
            f"fmu_time={ti:.1f} -> moose_time={di:.5f} -> "
            "diffused={diff:.5f} -> rep_value={rep:.5f} "
        )
