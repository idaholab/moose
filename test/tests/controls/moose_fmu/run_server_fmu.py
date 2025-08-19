from fmpy import simulate_fmu
import numpy as np


if __name__ == "__main__":


    t0, t1, dt = 0, 5.0, 0.5
    times = np.arange(t0, t1 + dt/2, dt)

    input_data = np.zeros(times.size,
                        dtype=[('recorded_t',   np.float64),])
    input_data['recorded_t']   = times

    result = simulate_fmu(
        "MooseTest.fmu",
        start_time=t0,
        stop_time=t1,
        start_values={
        'flag':             'TIMESTEP_END',
        'moose_executable': '../../../moose_test-opt',
        'moose_inputfile':  'fmu_diffusion.i',
        'server_name':      'web_server',
        'max_retries':      10
    },
        input = input_data,
        output      = ['time','moose_time', 'diffused']
    )

    time     = result["time"]
    recorded_t = input_data['recorded_t']
    dt     = result["moose_time"]
    diff_u = result["diffused"]

    for ti, di, diff in zip(time, dt, diff_u):
        print(f"fmu_time={ti:.1f} → moose_time={di:.5f} → diffused={diff:.5f} ")


