
import numpy as np
import matplotlib.pyplot as plt
from fmpy import read_model_description, extract, instantiate_fmu
import time
import math

def fmu_info(fmu_model: str, filename: str):
    print(f"Load fmu model decription: {filename}")
    fmu_description = read_model_description(fmu_model)

    print("Fmu model info:")
    for v in fmu_description.modelVariables:
        print(f"{v.name:10s}  causality={v.causality:10s}  variability={v.variability:12s}  "
            f"type={v.type}  start={v.start} valueReference={v.valueReference}")

    return fmu_description

def get_real(fmu, vr_map, name):
    vr = vr_map[name]
    return fmu.getReal([vr])[0]

def get_string(fmu, vr_map, name):
    vr = vr_map[name]
    return fmu.getString([vr])[0]

def get_bool(fmu, vr_map, name):
    vr = vr_map[name]
    return fmu.getBoolean([vr])[0]

def set_string(fmu, vr_map, name, value):
    vr = vr_map[name]
    return fmu.setString([vr], [value])

def set_real(fmu, vr_map, name, value):
    vr = vr_map[name]
    return fmu.setReal([vr], [value])

def set_bool(fmu, vr_map, name, value):
    vr = vr_map[name]
    return fmu.setBoolean([vr], [value])

if __name__ == "__main__":


    fmu_filename = 'Dahlquist.fmu'
    moose_filename = 'MooseTest.fmu'

    start_time = 0.0
    stop_time = 5.0
    step_size = 0.1
    change_time = 2.0

    # Extract FMU
    fmu_model = extract(fmu_filename)
    moose_model = extract(moose_filename)

    # Read FMU model description
    fmu_description = fmu_info(fmu_model, fmu_filename)
    moose_decription = fmu_info(moose_model, moose_filename)

    # Instantiate the FMU
    fmu_instance = instantiate_fmu(fmu_model, fmu_description)
    moose_instance = instantiate_fmu(moose_model, moose_decription)

    moose_instance.setDebugLogging(loggingOn=True, categories=['logStatusError', 'logStatusWarning'])

    # Initialize
    fmu_instance.setupExperiment()
    fmu_instance.enterInitializationMode()
    fmu_instance.exitInitializationMode()

    moose_instance.setupExperiment()
    moose_instance.enterInitializationMode()
    moose_instance.exitInitializationMode()

    vr_map = { v.name: v.valueReference for v in moose_decription.modelVariables }

    times = []
    outputs = []
    moose_times = []
    moose_times_ref = []
    diffusedT = []
    diffusedT_ref = []

    mytime = start_time
    while mytime <= stop_time:

        moose_instance.doStep(currentCommunicationPoint=mytime, communicationStepSize=step_size)
        moose_t = get_real(moose_instance, vr_map, 'moose_time')
        diff  = get_real(moose_instance, vr_map, 'diffused')

        moose_times_ref.append(moose_t)
        diffusedT_ref.append(diff)

        mytime += step_size

    moose_instance.terminate()
    moose_instance.freeInstance()

    print("Start the second moose run after 30s")
    time.sleep(30)

    moose_instance = instantiate_fmu(moose_model, moose_decription)
    moose_instance.setupExperiment()
    moose_instance.enterInitializationMode()
    moose_instance.exitInitializationMode()

    mytime = start_time
    while mytime <= stop_time:
        fmu_instance.doStep(currentCommunicationPoint=mytime, communicationStepSize=step_size)
        value = fmu_instance.getReal([fmu_description.modelVariables[1].valueReference])

        times.append(mytime)
        outputs.append(value)

        if math.isclose(mytime, 1.0, rel_tol=1e-5, abs_tol=1e-9):
           newBC = value[0]*10

           set_bool(moose_instance, vr_map, 'change_BC', True)
           set_string(moose_instance, vr_map, 'BC_info', "BCs/left/value")
           set_real(moose_instance, vr_map, 'BC_value', newBC)
           print("Wait 10s to get the boundary condition set")
           time.sleep(10)

        moose_instance.doStep(currentCommunicationPoint=mytime, communicationStepSize=step_size)
        moose_t = get_real(moose_instance, vr_map, 'moose_time')
        diff  = get_real(moose_instance, vr_map, 'diffused')

        moose_times.append(moose_t)
        diffusedT.append(diff)

        mytime += step_size


    fmu_instance.terminate()
    moose_instance.terminate()

    fmu_instance.freeInstance()
    moose_instance.freeInstance()

    for ti, di, diff in zip(times, moose_times, diffusedT):
        print(f"fmu_time={ti:.1f} → moose_time={di:.5f} → diffused={diff:.5f} ")

    plt.figure()
    plt.plot(moose_times_ref, diffusedT_ref, label='Reference', linewidth=2)
    plt.plot(moose_times, diffusedT, label='Coupled', linestyle='--', linewidth=2)

    plt.xlabel('time')
    plt.ylabel('Temperature')
    plt.title('Reference vs. Coupled FMUs Temperature Over time')
    plt.legend()
    plt.grid(True)

    # save to PNG (300 dpi, tight layout)
    plt.savefig('temp_comparison.png', dpi=300, bbox_inches='tight')
    plt.close()
