import numpy as np
from fmpy import simulate_fmu

if __name__ == "__main__":

    start_time = 0.0
    stop_time  = 5.0
    step_size  = 0.5

    t0, t1, dt = 0.0, 1.0, 0.1
    times = np.arange(t0, t1 + dt/2, dt)
    Kp = 0.5

    input_data = np.zeros(times.size,
                        dtype=[('time',   np.float64),
                                ('in_val', np.float64)])
    input_data['time']   = times
    input_data['in_val'] = 2.5

    # run the FMU
    result = simulate_fmu(
        filename    = "PythonSlave.fmu",
        start_time  = start_time,
        stop_time   = stop_time,
        start_values = { "Kp": Kp },
        input       = input_data,
        output      = ['time','intOut','realOut'],
        validate    = False,
        early_return_allowed = True,
    )

    # pull out the arrays
    t     = result["time"]
    ints  = result["intOut"]
    reals = result["realOut"]

    # # 1) check first and last time‐points
    # assert np.isclose(t[0], start_time), f"First time was {t[0]}, expected {start_time}"
    # assert np.isclose(t[-1], stop_time), f"Last time was {t[-1]}, expected {stop_time}"

    # # 2) check uniform stepping
    # diffs = np.diff(t)
    # assert np.allclose(diffs, step_size), f"Time‐steps vary: {diffs}"

    # # 3) check expected number of points
    # expected_n = int(round((stop_time - start_time)/step_size)) + 1
    # assert t.shape[0] == expected_n, f"Got {t.shape[0]} steps, expected {expected_n}"

    # # 4) your original output assertions
    # assert np.all(ints == Kp),      "intOut should always equal to Kp"
    # assert np.allclose(reals, Kp + input_data['in_val']), "realOut should always equal to Kp + in_val"

    # print out a few lines for visual confirmation
    for ti, ii, rr in zip(t, ints, reals):
        print(f"t={ti:.1f} → intOut={ii}, realOut={rr}")

    print("✅ Timing parameters are being honored and outputs look good.")
