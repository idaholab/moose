from fmpy import simulate_fmu
import numpy as np
import pandas as pd
import logging

# Configure root logger
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s [%(levelname)s] %(name)s: %(message)s'
)
logger = logging.getLogger(__name__)

def main():

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
        'flag':             'INITIAL TIMESTEP_END',
        'moose_executable': '../../../moose_test-opt',
        'moose_inputfile':  'fmu_diffusion.i',
        'server_name':      'web_server',
        'max_retries':      10},
        input = input_data,
        debug_logging=False,
        output      = ['time','moose_time', 'diffused']
    )

    time     = result["time"]
    recorded_t = input_data['recorded_t']
    dt     = result["moose_time"]
    diff_u = result["diffused"]

    for ti, di, diff in zip(time, dt, diff_u):
        print(f"fmu_time={ti:.1f} → moose_time={di:.5f} → diffused={diff:.5f} ")

    df = pd.DataFrame(result)

    df.to_csv("run_fmu.csv", index=False)

if __name__ == "__main__":
    main()



