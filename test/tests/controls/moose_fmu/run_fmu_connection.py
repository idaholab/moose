#!/usr/bin/env python3

import time
import matplotlib.pyplot as plt
from fmpy import extract, instantiate_fmu
from fmpy.simulation import apply_start_values
from MooseFMU import (
    fmu_info,
    get_real,
    set_real,
    set_string,
)
from MooseFMU import configure_fmu_logging
import math
import pandas as pd
import moose_fmu_tester

"""
Couple a toy FMU (Dahlquist test equation y'(t) = k * y) with a MOOSE FMU and
compare a reference run (MOOSE-only) against a coupled run (FMU -> MOOSE BC).

Workflow
--------
1) Reference run:
   - Run the MOOSE FMU by itself from start_time to stop_time with step_size.
   - Record 'moose_time' and the averaged 'diffused' temperature for plotting.

2) Coupled run:
   - Step the Dahlquist FMU each step to obtain an external signal (val).
   - At 'change_time', update MOOSE's left boundary condition:
       - Set 'BC_info' to "BCs/left/value"
       - Set 'BC_value' based on the Dahlquist output (val * 10 here)
     (A short sleep is used to allow the server-side BC update to take effect.)
   - Step the MOOSE FMU and record outputs for comparison.

Notes
-----
- Both FMUs are unpacked via fmpy.extract and instantiated via instantiate_fmu.
- MOOSE FMU start values include 'flag', 'moose_command', 'server_name',
  and 'max_retries'. Adjust these to your environment and launcher.
- Choose step_size so (stop_time - start_time) is an integer multiple of step_size
  to avoid end-time alignment surprises.
- The Dahlquist FMU output is accessed by index (modelVariables[1]); consider
  mapping by name in production code to avoid brittle indexing.
"""

if __name__ == '__main__':
    logger = configure_fmu_logging(logger_name=__name__)
    # Filenames and simulation parameters
    fmu_filename = 'Dahlquist.fmu'
    moose_filename = 'MooseTest.fmu'

    start_time = 0.0
    stop_time = 2.0
    step_size = 0.5
    change_time = 1.0

    flag = "INITIAL MULTIAPP_FIXED_POINT_END"

    # Extract FMUs
    fmu_model = extract(fmu_filename)
    moose_model = extract(moose_filename)

    # Read model descriptions
    fmu_description = fmu_info(fmu_model, fmu_filename)
    moose_description = fmu_info(moose_model, moose_filename)

    # Instantiate FMUs
    fmu_instance = instantiate_fmu(fmu_model, fmu_description)
    moose_instance = instantiate_fmu(moose_model, moose_description)

    # Initialize moose FMU for reference run
    moose_instance.setupExperiment()
    moose_instance.enterInitializationMode()

   # Provide your own MOOSE command for non testing senarios
    moose_command = moose_fmu_tester.test_controller()

    apply_start_values(
            fmu=moose_instance,
            model_description=moose_description,
            start_values={
                "flag":             flag,
                'moose_command':    moose_command,
                "server_name":      "web_server",
                "max_retries":      10,
            },
        )

    moose_instance.exitInitializationMode()

    fmu_instance.setupExperiment()
    fmu_instance.enterInitializationMode()
    fmu_instance.exitInitializationMode()

    # Prepare variable-reference map for Moose
    vr_map = {v.name: v.valueReference for v in moose_description.modelVariables}

    # Reference run (Moose only)
    moose_times_ref, diffused_ref = [], []
    t = start_time
    while t <= stop_time:
        moose_instance.doStep(currentCommunicationPoint=t,
                              communicationStepSize=step_size)
        moose_times_ref.append(get_real(moose_instance, vr_map, 'moose_time'))
        diffused_ref.append(get_real(moose_instance, vr_map, 'diffused'))
        t += step_size

    moose_instance.terminate()
    moose_instance.freeInstance()

    # Pause before coupled run
    logger.info("Start the second moose run after 2s")
    time.sleep(1)

    # Re-instantiate Moose for coupled run
    moose_instance = instantiate_fmu(moose_model, moose_description)

    moose_instance.setupExperiment()
    moose_instance.enterInitializationMode()

    apply_start_values(
            fmu=moose_instance,
            model_description=moose_description,
            start_values={
                "flag":             flag,
                'moose_command': "../../../moose_test-opt -i fmu_diffusion.i",
                "server_name":      "web_server",
                "max_retries":      10,
            },
        )

    moose_instance.exitInitializationMode()

    # Coupled run data
    times, moose_times, diffused = [], [], []
    t = start_time
    while t <= stop_time:
        # Step FMU
        fmu_instance.doStep(currentCommunicationPoint=t,
                             communicationStepSize=step_size)
        val = fmu_instance.getReal([fmu_description.modelVariables[1].valueReference])[0]
        times.append(t)

        # Change boundary condition at specified time
        if math.isclose(t, change_time, rel_tol=1e-5, abs_tol=1e-9):
            newBC = val * 10
            set_string(moose_instance, vr_map, 'BC_info', "BCs/left/value")
            set_real(moose_instance, vr_map, 'BC_value', newBC)
            logger.info("Wait 2s to get the boundary condition set")
            time.sleep(2)

        # Step Moose
        moose_instance.doStep(currentCommunicationPoint=t,
                               communicationStepSize=step_size)
        moose_times.append(get_real(moose_instance, vr_map, 'moose_time'))
        diffused.append(get_real(moose_instance, vr_map, 'diffused'))

        t += step_size

    # Terminate and free instances
    fmu_instance.terminate()
    moose_instance.terminate()

    fmu_instance.freeInstance()
    moose_instance.freeInstance()

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
    plt.plot(moose_times_ref, diffused_ref, label='Reference', linewidth=2)
    plt.plot(moose_times, diffused, label='Coupled', linestyle='--', linewidth=2)
    plt.xlabel('time')
    plt.ylabel('Averaged Temperature in 2D Diffusion Channel')
    plt.title('Reference vs. Coupled FMUs Temperature Over time')
    plt.legend()
    plt.grid(True)
    plt.savefig('temp_comparison.png', dpi=300, bbox_inches='tight')
    plt.close()

