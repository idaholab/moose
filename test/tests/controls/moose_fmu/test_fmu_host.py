import sys
from pyfmi import load_fmu

def test_init_and_step(fmu_path: str, host: str, port: int, final_time: float = 0.01):
    print(f"[TEST] Loading FMU from: {fmu_path} with DEBUG logging")
    # Load FMU with maximum verbosity
    model = load_fmu(fmu_path, log_level=7)

    print(f"[TEST] Setting host={host!r}, port={port}")
    model.set("host", host)
    model.set("port", port)

    print("[TEST] Running simulate() (initialization hooks will be called)")
    opts = model.simulate_options()
    opts["result_handling"] = "csv"

    # 3) Run simulate
    print("[TEST] Running simulate()")
    try:
        res = model.simulate(final_time=final_time, options=opts)
        print("[TEST] simulate() OK")
    except Exception as e:
        print(f"[TEST ERROR] simulate() failed: {e}")
        print("----- FMU LOG -----")
        print(model.get_log())
        sys.exit(1)

    # 4) Retrieve 't'
    try:
        t_vals = res["t"]
        print(f"[TEST] Retrieved 't' values: {t_vals}")
    except Exception as e:
        print(f"[TEST] ERROR retrieving 't': {e}")

    return res

if __name__ == "__main__":
    fmu_file = sys.argv[1] if len(sys.argv) > 1 else "MOOSE2Fmu.fmu"
    host_arg = sys.argv[2] if len(sys.argv) > 2 else "localhost"
    port_arg = int(sys.argv[3]) if len(sys.argv) > 3 else 12345
    test_init_and_step(fmu_file, host_arg, port_arg)
