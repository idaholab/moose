from fmpy import simulate_fmu
import numpy as np


if __name__ == "__main__":

    T_end = 3

    result = simulate_fmu(
        "MooseSlave.fmu",
        start_time=0.0,
        stop_time=T_end,
        output      = ['time','moose_time', 'diffused']
    )

    time     = result["time"]
    dt     = result["moose_time"]
    diff_u = result["diffused"]

    for ti, di, diff in zip(time, dt, diff_u):
        print(f"fmu_time={ti:.1f} → moose_time={di:.1f} → diffused={diff:.5f} ")

    # result = simulate_fmu(
    #     "MooseSlave.fmu",
    #     start_time=0.0,
    #     stop_time=T_end,
    #     output      = ['time','moose_time']
    # )

    # time     = result["time"]
    # dt     = result["moose_time"]

    # for ti, di in zip(time, dt):
    #     print(f"fmu_time={ti:.1f} → moose_time={di:.1f}")

