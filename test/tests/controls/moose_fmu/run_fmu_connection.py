import time
import matplotlib.pyplot as plt
from fmpy import extract, instantiate_fmu
from utils import (
    fmu_info,
    get_real,
    get_string,
    get_bool,
    set_real,
    set_string,
    set_bool,
)
import logging
import math

# Configure root logger
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s [%(levelname)s] %(name)s: %(message)s'
)
logger = logging.getLogger(__name__)


def main():
    # Filenames and simulation parameters
    fmu_filename = 'Dahlquist.fmu'  # dahlquist test equation y'(t) = k * y(t)
    moose_filename = 'MooseTest.fmu'

    start_time = 0.0
    stop_time = 5.0
    step_size = 0.1
    change_time = 1.0

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
    logger.info("Start the second moose run after 30s")
    time.sleep(30)

    # Re-instantiate Moose for coupled run
    moose_instance = instantiate_fmu(moose_model, moose_description)

    moose_instance.setupExperiment()
    moose_instance.enterInitializationMode()
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
            set_bool(moose_instance, vr_map, 'change_BC', True)
            set_string(moose_instance, vr_map, 'BC_info', "BCs/left/value")
            set_real(moose_instance, vr_map, 'BC_value', newBC)
            logger.info("Wait 10s to get the boundary condition set")
            time.sleep(10)

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
        print(f"fmu_time={ti:.1f} → moose_time={mti:.5f} → diffused={d:.5f}")

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


if __name__ == '__main__':
    main()
