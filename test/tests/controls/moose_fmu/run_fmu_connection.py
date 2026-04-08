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
import math
import time

import matplotlib.pyplot as plt
import moose_fmu_tester_pyfmi
import pandas as pd
from moosefmu import configure_fmu_logging, get_float
from pyfmi import load_fmu

"""
Couple a toy FMU (Dahlquist test equation y'(t) = k * y) with a MOOSE FMU and
compare a reference run (MOOSE-only) against a coupled run (FMU -> MOOSE BC).

Workflow
--------
1) Reference run:
   - Run the MOOSE FMU by itself from start_time to stop_time with step_size.
   - Record 'moose_time' and the averaged 'diffused' temperature for plotting.

2) Coupled run:
   - Step the Dahlquist FMU each step to obtain the external signal 'x'.
   - At 'change_time', update MOOSE's left boundary condition:
       - Set 'BC_info' to "BCs/left/value"
       - Set 'BC_value' based on the Dahlquist output (x * 10 here)
     (A short sleep is used to allow the server-side BC update to take effect.)
   - Step the MOOSE FMU and record outputs for comparison.

Notes
-----
- FMUs are loaded and stepped with pyfmi only.
- MOOSE FMU start values include 'flag', 'moose_command', 'server_name',
  and 'max_retries'. Adjust these to your environment and launcher.
- Choose step_size so (stop_time - start_time) is an integer multiple of
  step_size to avoid end-time alignment surprises.
"""


def _initialize_moose_fmu(model, t0: float, t1: float, flag: str, cmd: str) -> None:
    model.setup_experiment(start_time=t0, stop_time=t1)
    model.enter_initialization_mode()
    model.set("flag", flag)
    model.set("moose_command", cmd)
    model.set("server_name", "web_server")
    model.set("max_retries", 10)
    model.exit_initialization_mode()


def _initialize_passthrough_fmu(model, t0: float, t1: float) -> None:
    model.setup_experiment(start_time=t0, stop_time=t1)
    model.enter_initialization_mode()
    model.exit_initialization_mode()


def _finalize_fmu(model) -> None:
    with contextlib.suppress(Exception):
        model.terminate()


def _load_cs_fmu(filename: str):
    """Load an FMU explicitly as FMI Co-Simulation."""
    model = load_fmu(filename, kind="CS")
    if not hasattr(model, "do_step"):
        raise TypeError(
            f"{filename} did not load as Co-Simulation model; missing do_step(). "
            "Load with kind='CS' and verify the FMU exposes a CoSimulation section."
        )
    return model


if __name__ == "__main__":
    logger = configure_fmu_logging(logger_name=__name__)

    # Filenames and simulation parameters
    fmu_filename = "Dahlquist.fmu"
    moose_filename = "MooseTest.fmu"

    start_time = 0.0
    stop_time = 2.0
    step_size = 0.5
    change_time = 1.0
    time_tol = 1e-15

    flag = "INITIAL MULTIAPP_FIXED_POINT_END"

    # Provide your own MOOSE command for non testing scenarios
    moose_command = moose_fmu_tester_pyfmi.test_controller()

    fmu_instance = _load_cs_fmu(fmu_filename)
    moose_instance = _load_cs_fmu(moose_filename)

    try:
        # Initialize FMUs for reference run
        _initialize_moose_fmu(
            moose_instance, start_time, stop_time, flag, moose_command
        )
        _initialize_passthrough_fmu(fmu_instance, start_time, stop_time)

        # Reference run (MOOSE only)
        moose_times_ref, diffused_ref = [], []
        t = start_time
        while t <= stop_time:
            moose_instance.do_step(current_t=t, step_size=step_size)
            moose_times_ref.append(get_float(moose_instance, "moose_time"))
            diffused_ref.append(get_float(moose_instance, "diffused"))
            t += step_size

        _finalize_fmu(moose_instance)

        # Pause before coupled run
        logger.info("Start the second MOOSE run after 1s")
        time.sleep(1)

        # Re-load and initialize MOOSE for coupled run
        moose_instance = _load_cs_fmu(moose_filename)
        _initialize_moose_fmu(
            moose_instance, start_time, stop_time, flag, moose_command
        )

        # Coupled run data
        times, moose_times, diffused = [], [], []
        t = start_time
        while t <= stop_time:
            # Step Dahlquist FMU and read its output state.
            fmu_instance.do_step(current_t=t, step_size=step_size)
            val = get_float(fmu_instance, "x")
            times.append(t)

            # Change boundary condition at specified time.
            if math.isclose(t, change_time, rel_tol=1e-5, abs_tol=1e-9):
                new_bc = val * 10.0
                moose_instance.set("BC_info", "BCs/left/value")
                moose_instance.set("BC_value", new_bc)
                logger.info("Wait 2s to ensure boundary condition update takes effect")
                time.sleep(2)

            # Step MOOSE FMU and capture outputs.
            moose_instance.do_step(current_t=t, step_size=step_size)
            moose_times.append(get_float(moose_instance, "moose_time"))
            diffused.append(get_float(moose_instance, "diffused"))

            t = min(t + step_size, stop_time + time_tol)
    finally:
        _finalize_fmu(fmu_instance)
        _finalize_fmu(moose_instance)

    # Report results
    for ti, mti, d in zip(times, moose_times, diffused):
        print(f"fmu_time={ti:.1f} -> moose_time={mti:.5f} -> diffused={d:.5f}")

    df = pd.DataFrame(
        zip(times, moose_times, diffused),
        columns=["fmu_time", "moose_time", "diffused"],
    )
    df.to_csv("run_fmu_connection.csv", index=False)

    # Plot comparison
    plt.figure()
    plt.plot(moose_times_ref, diffused_ref, label="Reference", linewidth=2)
    plt.plot(moose_times, diffused, label="Coupled", linestyle="--", linewidth=2)
    plt.xlabel("time")
    plt.ylabel("Averaged Temperature in 2D Diffusion Channel")
    plt.title("Reference vs. Coupled FMUs Temperature Over time")
    plt.legend()
    plt.grid(True)
    plt.savefig("temp_comparison.png", dpi=300, bbox_inches="tight")
    plt.close()
