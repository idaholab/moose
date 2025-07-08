from fmpy import simulate_fmu
import numpy as np


if __name__ == "__main__":

    T_end = 3

    result = simulate_fmu(
    'MooseDemo.fmu',
    start_values={
        # must match the parameter names you registered
        'flag':             'TIMESTEP_END',
        'moose_executable': '../../../moose_test-opt',
        'moose_inputfile':  'fmu_diffusion.i',
        'server_name':      'web_server',
        'max_retries':      3
    },
    #   'moose_mpi':        '',
    #    'mpi_num':          1,

    start_time=0.0,
    stop_time=3.0,
    step_size=0.5,
    output=['time', 'diffused', 'moose_time']
)

    time     = result["time"]
    dt     = result["moose_time"]
    diff_u = result["diffused"]

    for ti, di, diff in zip(time, dt, diff_u):
        print(f"fmu_time={ti:.1f} → moose_time={di:.1f} → diffused={diff:.5f} ")


