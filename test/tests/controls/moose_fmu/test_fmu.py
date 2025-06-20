import numpy as np
from fmpy import simulate_fmu

if __name__ == "__main__":
    # simulate from t=0 to t=1 in 0.1-s steps
    result = simulate_fmu(
        filename="PythonSlave.fmu",
        start_time=0.0,
        stop_time=1.0,
        step_size=0.1,
        output=["intOut", "realOut"],
    )

    # result is a numpy.recarray; extract columns:
    t = result["time"]
    ints = result["intOut"]
    reals = result["realOut"]

    # print a few rows
    for ti, ii, rr in zip(t, ints, reals):
        print(f"t={ti:.1f} → intOut={ii}, realOut={rr}")

    # basic assertions
    assert np.all(ints == 1), "intOut should always be 1"
    assert np.allclose(reals, 3), "realOut should always be 3.0"

    print("✅ FMU outputs are constant as expected.")
