from fmpy import simulate_fmu, extract, read_model_description, instantiate_fmu
from fmpy.simulation import apply_start_values
import numpy as np
import pandas as pd
import logging
import time
import shutil

# Configure root logger
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s [%(levelname)s] %(name)s: %(message)s'
)
logger = logging.getLogger(__name__)

def simulate_moose_fmu(moose_filename, t0, t1, dt):

    result = simulate_fmu(
        moose_filename,
        start_time=t0,
        stop_time=t1,
        step_size =dt,
        start_values={
        'flag':             'INITIAL TIMESTEP_END',
        'moose_executable': '../../../moose_test-opt',
        'moose_inputfile':  'fmu_diffusion.i',
        'server_name':      'web_server',
        'max_retries':      10},
        debug_logging=True,
        output      = ['time','moose_time', 'diffused', "rep_value"]
    )

    df = pd.DataFrame(result)

    df.to_csv("run_fmu.csv", index=False)

    return result

def moose_fmu_step_by_step(
    moose_filename: str,
    t0: float,
    t1: float,
    dt: float,
    *,
    rtol: float = 1e-6,
    atol: float = 1e-9,
    time_tol: float | None = None,        # None -> auto dt/2
    baseline_csv: str = "run_fmu.csv",
    step_csv: str = "run_fmu_step_by_step.csv"
):
    """
    Manual FMI 2.0 run + comparison with baseline CSV produced by simulate_moose_fmu().
    """
    if time_tol is None:
        time_tol = dt / 2.0

    unzipdir = extract(moose_filename)
    md = read_model_description(unzipdir)
    fmu = instantiate_fmu(unzipdir=unzipdir, model_description=md)

    try:
        vrs = {v.name: v.valueReference for v in md.modelVariables}

        # --- Initialization ---
        fmu.instantiate()
        fmu.setupExperiment(startTime=t0, stopTime=t1)
        fmu.enterInitializationMode()

        apply_start_values(
            fmu=fmu,
            model_description=md,
            start_values={
                "flag":             "INITIAL TIMESTEP_END",
                "moose_executable": "../../../moose_test-opt",
                "moose_inputfile":  "fmu_diffusion.i",
                "server_name":      "web_server",
                "max_retries":      10,
            },
        )

        fmu.exitInitializationMode()
        # ----------------------

        # --- Step loop ---
        rows = []
        t = t0
        while t < t1 - 1e-15:
            fmu.doStep(currentCommunicationPoint=t, communicationStepSize=dt)
            t = min(t + dt, t1)

            moose_time = fmu.getReal([vrs["moose_time"]])[0]
            diffused   = fmu.getReal([vrs["diffused"]])[0]
            rep_value = fmu.getReal([vrs["rep_value"]])[0]
            print(f"fmu_time={t:.3f} → moose_time={moose_time:.6f} → diffused={diffused:.6f}→ rep_value={rep_value:.6f}")
            rows.append((t, moose_time, diffused, rep_value))

        result = np.array(
            rows,
            dtype=[("time", np.float64), ("moose_time", np.float64), ("diffused", np.float64), ("rep_value", np.float64)],
        )

        # Save our step-by-step results
        df_step = pd.DataFrame(result)
        df_step.to_csv(step_csv, index=False)

        # --- Comparison with baseline CSV ---
        try:
            df_base = pd.read_csv(baseline_csv)
        except FileNotFoundError:
            logger.warning(f"Baseline CSV '{baseline_csv}' not found; skipping comparison.")
            return result

        # Ensure time-sorted for asof join (nearest match)
        df_base = df_base.sort_values("time").reset_index(drop=True)
        df_step = df_step.sort_values("time").reset_index(drop=True)

        # Align on 'time' using nearest within tolerance
        aligned = pd.merge_asof(
            df_base, df_step,
            on="time", direction="nearest", tolerance=time_tol,
            suffixes=("_base", "_step")
        )
        # Drop any rows that failed to align within tolerance
        aligned = aligned.dropna(subset=["moose_time_base", "moose_time_step", "diffused_base", "diffused_step", "rep_value_base", "rep_value_step"])

        n_base   = len(df_base)
        n_step   = len(df_step)
        n_align  = len(aligned)

        # Compute diffs and metrics
        mt_diff = aligned["moose_time_base"].to_numpy() - aligned["moose_time_step"].to_numpy()
        du_diff = aligned["diffused_base"].to_numpy()    - aligned["diffused_step"].to_numpy()
        rep_diff = aligned["rep_value_base"].to_numpy()  - aligned["rep_value_step"].to_numpy()

        def rmse(x: np.ndarray) -> float:
            return float(np.sqrt(np.mean(np.square(x)))) if x.size else float("nan")

        mt_max  = float(np.max(np.abs(mt_diff))) if mt_diff.size else float("nan")
        du_max  = float(np.max(np.abs(du_diff))) if du_diff.size else float("nan")
        rep_max  = float(np.max(np.abs(rep_diff))) if rep_diff.size else float("nan")
        mt_rmse = rmse(mt_diff)
        du_rmse = rmse(du_diff)
        rep_rmse = rmse(rep_diff)

        # allclose checks (vectorized) for the aligned rows
        ok_mt = np.allclose(
            aligned["moose_time_base"].to_numpy(),
            aligned["moose_time_step"].to_numpy(),
            rtol=rtol, atol=atol
        )
        ok_du = np.allclose(
            aligned["diffused_base"].to_numpy(),
            aligned["diffused_step"].to_numpy(),
            rtol=rtol, atol=atol
        )

        ok_rep = np.allclose(
            aligned["rep_value_base"].to_numpy(),
            aligned["rep_value_step"].to_numpy(),
            rtol=rtol, atol=atol
        )

        print("\n=== Comparison vs. baseline ===")
        print(f"Rows: baseline={n_base}, step_by_step={n_step}, aligned={n_align} (time_tol={time_tol})")
        print(f"moose_time: allclose={ok_mt} (rtol={rtol}, atol={atol}) | max_abs={mt_max:.3e} | rmse={mt_rmse:.3e}")
        print(f"diffused:   allclose={ok_du} (rtol={rtol}, atol={atol}) | max_abs={du_max:.3e} | rmse={du_rmse:.3e}")
        print(f"rep_value:  allclose={ok_rep} (rtol={rtol}, atol={atol}) | max_abs={rep_max:.3e} | rmse={rep_rmse:.3e}")

        if n_align < min(n_base, n_step):
            logger.warning(
                f"{min(n_base, n_step) - n_align} rows could not be aligned within ±{time_tol} s."
            )

        return result

    finally:
        # Cleanup
        try:
            fmu.terminate()
        except Exception:
            pass
        try:
            fmu.freeInstance()
        except Exception:
            pass
        shutil.rmtree(unzipdir, ignore_errors=True)

def main():

    t0, t1, dt = 0, 2.0, 0.5
    moose_filename = 'MooseTest.fmu'
    result = simulate_moose_fmu(moose_filename, t0, t1, dt)
    logger.info("Start the second moose run after 2s")
    time.sleep(2)
    result = moose_fmu_step_by_step(moose_filename, t0, t1, dt)

    fmu_time  = result["time"]
    dt        = result["moose_time"]
    diff_u    = result["diffused"]
    rep_value = result["rep_value"]

    for ti, di, diff, rep in zip(fmu_time, dt, diff_u, rep_value):
        print(f"fmu_time={ti:.1f} → moose_time={di:.5f} → diffused={diff:.5f}→ rep_value={rep:.5f} ")


if __name__ == "__main__":
    main()



