#!/usr/bin/env python3
"""Compare the t=0.4 MOOSE dam-break run with OpenFOAM and Martin-Moyce data."""

from __future__ import annotations

import csv
import math
import sys
from argparse import ArgumentParser
from pathlib import Path

import numpy as np
import pandas as pd
from scipy.io import netcdf_file


A_LENGTH = 0.05715
G = 9.81
NX = 400
NY = 50
DX = 10.0 * A_LENGTH / NX
DY = 1.25 * A_LENGTH / NY
TAU_SCALE = math.sqrt(G / A_LENGTH)
ALPHA_INTERFACE = 0.5

ROOT = Path(__file__).resolve().parents[1]
DEFAULT_MOOSE_EXODUS = Path("/private/tmp/dam_break_openfoam_geometry/run_t04.e")
EXPERIMENTAL = Path("/Users/chowr/Downloads/experimental_data/mm_consistency_check_all_data.csv")

OPENFOAM_CASES = {
    "OpenFOAM_damBreak_MM_a2250": Path(
        "/Users/chowr/Work/projects/freeSurface/rod_output/damBreak_MM_a2250"
    ),
    "OpenFOAM_damBreak2": Path(
        "/Users/chowr/Work/projects/freeSurface/rod_output/damBreak2/damBreak_MM_a2250"
    ),
    "OpenFOAM_damBreak3_relax": Path(
        "/Users/chowr/Work/projects/freeSurface/rod_output/damBreak3_withrelaxation_07_03"
    ),
}

OUT_PREFIX = ROOT / "dam_break_openfoam_geometry_run_t04_vs_openfoam_martin_moyce"

EXPECTED_MOOSE_POINTS = {
    0.10: (2.2318066502596774, 0.7688834997258502),
    0.20: (4.30809387014947, 0.4453753193838102),
    0.40: (8.859063213196528, 0.16429785934034774),
}

EXPECTED_OPENFOAM_BASE_POINTS = {
    0.10: (2.352252502183978, 0.7583453996244693),
    0.20: (4.60297906413202, 0.44962094263440067),
}

GATE_LIMITS = {
    "min_final_time": 0.399,
    "relative_total_alpha_drift": 1e-10,
    "alpha_min_floor": -1e-12,
    "alpha_max_ceiling": 1.000001,
    "moose_front_point_abs": 0.03,
    "moose_height_point_abs": 0.015,
    "openfoam_point_abs": 1e-10,
    "MOOSE_front_vs_OpenFOAM_damBreak_MM_a2250_mae": 0.18,
    "MOOSE_height_vs_OpenFOAM_damBreak_MM_a2250_mae": 0.012,
    "MOOSE_run_t04_front_vs_MM_n1_a2250_mae": 0.30,
    "MOOSE_run_t04_height_vs_MM_H_n1_mae": 0.015,
}


def clean_name(chars) -> str:
    return bytes(chars).decode("ascii", "ignore").replace("\x00", "").strip()


def crossing(coord: np.ndarray, value: np.ndarray, target: float = ALPHA_INTERFACE) -> float:
    order = np.argsort(coord)
    coord = coord[order]
    value = value[order]
    above = value >= target

    if not np.any(above):
        return float(coord[0])

    last = int(np.where(above)[0][-1])
    if last == len(coord) - 1:
        return float(coord[-1])

    x0 = float(coord[last])
    x1 = float(coord[last + 1])
    v0 = float(value[last])
    v1 = float(value[last + 1])
    if v1 == v0:
        return 0.5 * (x0 + x1)

    return x0 + (target - v0) * (x1 - x0) / (v1 - v0)


def read_moose_series(path: Path) -> pd.DataFrame:
    with netcdf_file(path, "r", mmap=False) as exodus:
        names = [clean_name(row) for row in exodus.variables["name_elem_var"].data]
        alpha_name_index = names.index("alpha") + 1
        alpha = np.array(exodus.variables[f"vals_elem_var{alpha_name_index}eb1"].data)
        times = np.array(exodus.variables["time_whole"].data)
        coordx = np.array(exodus.variables["coordx"].data)
        coordy = np.array(exodus.variables["coordy"].data)
        connect = np.array(exodus.variables["connect1"].data) - 1

    centers_x = coordx[connect].mean(axis=1)
    centers_y = coordy[connect].mean(axis=1)
    ux = np.unique(np.round(centers_x, 12))
    uy = np.unique(np.round(centers_y, 12))
    ix = np.searchsorted(ux, np.round(centers_x, 12))
    iy = np.searchsorted(uy, np.round(centers_y, 12))
    bottom = iy == 0
    left = ix == 0

    rows = []
    for time, values in zip(times, alpha):
        front = crossing(centers_x[bottom], values[bottom])
        height = crossing(centers_y[left], values[left])
        rows.append(
            {
                "source": "MOOSE_run_t04",
                "time": float(time),
                "tau": float(time * TAU_SCALE),
                "front_x_over_a": front / A_LENGTH,
                "back_height_over_a": height / A_LENGTH,
                "alpha_min": float(np.min(values)),
                "alpha_max": float(np.max(values)),
                "total_alpha": float(np.sum(values) * DX * DY),
            }
        )

    return pd.DataFrame(rows)


def read_openfoam_alpha(path: Path) -> np.ndarray:
    lines = path.read_text().splitlines()
    for i, line in enumerate(lines):
        stripped = line.strip()
        if stripped.startswith("internalField") and "nonuniform" in stripped:
            j = i + 1
            while not lines[j].strip():
                j += 1
            count = int(lines[j].strip())
            while lines[j].strip() != "(":
                j += 1
            values = []
            for raw in lines[j + 1 :]:
                text = raw.strip()
                if text == ")":
                    break
                values.append(float(text))
            if len(values) != count:
                raise ValueError(f"Expected {count} alpha values in {path}, found {len(values)}")
            return np.array(values)
        if stripped.startswith("internalField") and "uniform" in stripped:
            return np.full(NX * NY, float(stripped.rstrip(";").split()[-1]))

    raise ValueError(f"No internalField found in {path}")


def numeric_time_dirs(case: Path) -> list[tuple[float, Path]]:
    dirs = []
    for child in case.iterdir():
        if not child.is_dir():
            continue
        try:
            dirs.append((float(child.name), child))
        except ValueError:
            pass
    return sorted(dirs)


def read_openfoam_series(source: str, case: Path) -> pd.DataFrame:
    x_centers = (np.arange(NX) + 0.5) * DX
    y_centers = (np.arange(NY) + 0.5) * DY
    centers_x = np.tile(x_centers, NY)
    centers_y = np.repeat(y_centers, NX)
    bottom = np.repeat(np.arange(NY), NX) == 0
    left = np.tile(np.arange(NX), NY) == 0

    rows = []
    for time, time_dir in numeric_time_dirs(case):
        alpha_path = time_dir / "alpha.water"
        if not alpha_path.exists():
            continue
        values = read_openfoam_alpha(alpha_path)
        front = crossing(centers_x[bottom], values[bottom])
        height = crossing(centers_y[left], values[left])
        rows.append(
            {
                "source": source,
                "time": time,
                "tau": time * TAU_SCALE,
                "front_x_over_a": front / A_LENGTH,
                "back_height_over_a": height / A_LENGTH,
                "alpha_min": float(np.min(values)),
                "alpha_max": float(np.max(values)),
                "total_alpha": float(np.sum(values) * DX * DY),
            }
        )

    return pd.DataFrame(rows)


def read_experiment() -> pd.DataFrame:
    return pd.read_csv(EXPERIMENTAL, comment="#")


def interp(series: pd.DataFrame, x: np.ndarray, column: str) -> np.ndarray:
    ordered = series.sort_values("tau")
    return np.interp(x, ordered["tau"], ordered[column])


def metric_rows(comparison: str, reference: np.ndarray, simulation: np.ndarray, tau: np.ndarray):
    diff = simulation - reference
    return {
        "comparison": comparison,
        "n_points": int(len(diff)),
        "tau_min": float(np.min(tau)) if len(tau) else math.nan,
        "tau_max": float(np.max(tau)) if len(tau) else math.nan,
        "bias": float(np.mean(diff)) if len(diff) else math.nan,
        "mae": float(np.mean(np.abs(diff))) if len(diff) else math.nan,
        "rmse": float(np.sqrt(np.mean(diff * diff))) if len(diff) else math.nan,
        "max_abs_error": float(np.max(np.abs(diff))) if len(diff) else math.nan,
    }


def add_overlay_rows(rows, comparison: str, tau, reference, simulation):
    for t, ref, sim in zip(tau, reference, simulation):
        rows.append(
            {
                "comparison": comparison,
                "tau": float(t),
                "reference": float(ref),
                "simulation": float(sim),
                "simulation_minus_reference": float(sim - ref),
            }
        )


def build_comparison(series: pd.DataFrame, experiment: pd.DataFrame):
    moose = series[series["source"] == "MOOSE_run_t04"]
    summary = []
    overlay = []

    for source in sorted(set(series["source"]) - {"MOOSE_run_t04"}):
        other = series[series["source"] == source].sort_values("tau")
        tau = other["tau"].to_numpy()
        moose_front = interp(moose, tau, "front_x_over_a")
        moose_height = interp(moose, tau, "back_height_over_a")
        other_front = other["front_x_over_a"].to_numpy()
        other_height = other["back_height_over_a"].to_numpy()
        summary.append(metric_rows(f"MOOSE_front_vs_{source}", other_front, moose_front, tau))
        summary.append(metric_rows(f"MOOSE_height_vs_{source}", other_height, moose_height, tau))
        add_overlay_rows(overlay, f"MOOSE_front_vs_{source}", tau, other_front, moose_front)
        add_overlay_rows(overlay, f"MOOSE_height_vs_{source}", tau, other_height, moose_height)

    mm_front = experiment[experiment["dataset"] == "MM_n1_a2250"].sort_values("tau")
    mm_height = experiment[experiment["dataset"] == "MM_H_n1"].sort_values("tau")
    for source in sorted(series["source"].unique()):
        sample = series[series["source"] == source]
        max_tau = float(sample["tau"].max())
        for label, exp_data, column in (
            ("front_vs_MM_n1_a2250", mm_front, "front_x_over_a"),
            ("height_vs_MM_H_n1", mm_height, "back_height_over_a"),
        ):
            points = exp_data[exp_data["tau"] <= max_tau]
            tau = points["tau"].to_numpy()
            reference = points["value"].to_numpy()
            simulation = interp(sample, tau, column)
            comparison = f"{source}_{label}"
            summary.append(metric_rows(comparison, reference, simulation, tau))
            add_overlay_rows(overlay, comparison, tau, reference, simulation)

    return pd.DataFrame(summary), pd.DataFrame(overlay)


def plot(series: pd.DataFrame, experiment: pd.DataFrame, out_prefix: Path):
    import matplotlib

    matplotlib.use("Agg")
    import matplotlib.pyplot as plt

    mm_front = experiment[experiment["dataset"] == "MM_n1_a2250"].sort_values("tau")
    mm_height = experiment[experiment["dataset"] == "MM_H_n1"].sort_values("tau")

    fig, axes = plt.subplots(2, 1, figsize=(8.5, 8.0), sharex=True)
    style = {
        "MOOSE_run_t04": ("-", "black", 2.4),
        "OpenFOAM_damBreak_MM_a2250": ("--", "#1f77b4", 1.6),
        "OpenFOAM_damBreak2": ("--", "#2ca02c", 1.4),
        "OpenFOAM_damBreak3_relax": ("--", "#ff7f0e", 1.4),
    }

    for source, data in series.groupby("source"):
        line, color, width = style[source]
        data = data.sort_values("tau")
        axes[0].plot(data["tau"], data["front_x_over_a"], line, color=color, lw=width, label=source)
        axes[1].plot(data["tau"], data["back_height_over_a"], line, color=color, lw=width, label=source)

    axes[0].scatter(
        mm_front["tau"],
        mm_front["value"],
        s=28,
        marker="o",
        facecolors="none",
        edgecolors="black",
        label="Martin-Moyce front, n=1, a=2.25 in",
    )
    axes[1].scatter(
        mm_height["tau"],
        mm_height["value"],
        s=28,
        marker="s",
        facecolors="none",
        edgecolors="black",
        label="Martin-Moyce height, n=1",
    )

    axes[0].set_ylabel("front x / a")
    axes[1].set_ylabel("back height / a")
    axes[1].set_xlabel(r"$\tau = t\sqrt{g/a}$")
    axes[0].grid(True, alpha=0.25)
    axes[1].grid(True, alpha=0.25)
    axes[0].legend(fontsize=8, loc="upper left")
    axes[1].legend(fontsize=8, loc="upper right")
    axes[0].set_title("Dam-break comparison: MOOSE run_t04, OpenFOAM, Martin-Moyce")
    axes[1].set_xlim(0.0, max(6.35, float(series["tau"].max())))
    fig.tight_layout()
    fig.savefig(f"{out_prefix}.png", dpi=180)
    fig.savefig(f"{out_prefix}.pdf")


def nearest_row(data: pd.DataFrame, time: float) -> pd.Series:
    index = (data["time"] - time).abs().idxmin()
    return data.loc[index]


def check_abs(name: str, value: float, expected: float, tolerance: float, failures: list[str]):
    error = abs(value - expected)
    status = "PASS" if error <= tolerance else "FAIL"
    print(f"{status}: {name}: value={value:.15g}, expected={expected:.15g}, abs_error={error:.3g}, tol={tolerance:g}")
    if error > tolerance:
        failures.append(f"{name} abs error {error:g} exceeds {tolerance:g}")


def check_le(name: str, value: float, limit: float, failures: list[str]):
    status = "PASS" if value <= limit else "FAIL"
    print(f"{status}: {name}: value={value:.15g}, limit={limit:g}")
    if value > limit:
        failures.append(f"{name} value {value:g} exceeds {limit:g}")


def check_ge(name: str, value: float, limit: float, failures: list[str]):
    status = "PASS" if value >= limit else "FAIL"
    print(f"{status}: {name}: value={value:.15g}, limit={limit:g}")
    if value < limit:
        failures.append(f"{name} value {value:g} is below {limit:g}")


def run_gate(series: pd.DataFrame, summary: pd.DataFrame) -> int:
    failures: list[str] = []
    moose = series[series["source"] == "MOOSE_run_t04"].sort_values("time")
    openfoam_base = series[series["source"] == "OpenFOAM_damBreak_MM_a2250"].sort_values("time")

    check_ge("MOOSE final time", float(moose["time"].max()), GATE_LIMITS["min_final_time"], failures)
    check_ge("MOOSE alpha min", float(moose["alpha_min"].min()), GATE_LIMITS["alpha_min_floor"], failures)
    check_le("MOOSE alpha max", float(moose["alpha_max"].max()), GATE_LIMITS["alpha_max_ceiling"], failures)

    initial_alpha = float(moose.iloc[0]["total_alpha"])
    final_alpha = float(moose.iloc[-1]["total_alpha"])
    relative_drift = abs(final_alpha - initial_alpha) / initial_alpha
    check_le(
        "MOOSE relative total_alpha drift",
        relative_drift,
        GATE_LIMITS["relative_total_alpha_drift"],
        failures,
    )

    for time, (expected_front, expected_height) in EXPECTED_MOOSE_POINTS.items():
        row = nearest_row(moose, time)
        check_abs(
            f"MOOSE front/a at t={time:g}",
            float(row["front_x_over_a"]),
            expected_front,
            GATE_LIMITS["moose_front_point_abs"],
            failures,
        )
        check_abs(
            f"MOOSE height/a at t={time:g}",
            float(row["back_height_over_a"]),
            expected_height,
            GATE_LIMITS["moose_height_point_abs"],
            failures,
        )

    for time, (expected_front, expected_height) in EXPECTED_OPENFOAM_BASE_POINTS.items():
        row = nearest_row(openfoam_base, time)
        check_abs(
            f"OpenFOAM base front/a at t={time:g}",
            float(row["front_x_over_a"]),
            expected_front,
            GATE_LIMITS["openfoam_point_abs"],
            failures,
        )
        check_abs(
            f"OpenFOAM base height/a at t={time:g}",
            float(row["back_height_over_a"]),
            expected_height,
            GATE_LIMITS["openfoam_point_abs"],
            failures,
        )

    summary_by_name = summary.set_index("comparison")
    for comparison in (
        "MOOSE_front_vs_OpenFOAM_damBreak_MM_a2250",
        "MOOSE_height_vs_OpenFOAM_damBreak_MM_a2250",
        "MOOSE_run_t04_front_vs_MM_n1_a2250",
        "MOOSE_run_t04_height_vs_MM_H_n1",
    ):
        check_le(
            f"{comparison} MAE",
            float(summary_by_name.loc[comparison, "mae"]),
            GATE_LIMITS[f"{comparison}_mae"],
            failures,
        )

    if failures:
        print("\nBenchmark gate FAILED:")
        for failure in failures:
            print(f"- {failure}")
        return 1

    print("\nBenchmark gate PASSED.")
    return 0


def parse_args():
    parser = ArgumentParser(description=__doc__)
    parser.add_argument("--moose-exodus", type=Path, default=DEFAULT_MOOSE_EXODUS)
    parser.add_argument("--out-prefix", type=Path, default=OUT_PREFIX)
    parser.add_argument("--check", action="store_true", help="enforce benchmark-gate thresholds")
    parser.add_argument("--no-plot", action="store_true", help="skip PNG/PDF plot generation")
    return parser.parse_args()


def main():
    args = parse_args()
    moose = read_moose_series(args.moose_exodus)
    openfoam = [read_openfoam_series(name, path) for name, path in OPENFOAM_CASES.items()]
    series = pd.concat([moose, *openfoam], ignore_index=True)
    experiment = read_experiment()
    summary, overlay = build_comparison(series, experiment)

    series.to_csv(f"{args.out_prefix}_series.csv", index=False)
    overlay.to_csv(f"{args.out_prefix}_overlay.csv", index=False)
    summary.to_csv(f"{args.out_prefix}_summary.csv", index=False)

    print(f"Wrote {args.out_prefix}_series.csv")
    print(f"Wrote {args.out_prefix}_overlay.csv")
    print(f"Wrote {args.out_prefix}_summary.csv")

    if not args.no_plot:
        plot(series, experiment, args.out_prefix)
        print(f"Wrote {args.out_prefix}.png")
        print(f"Wrote {args.out_prefix}.pdf")

    if args.check:
        return run_gate(series, summary)

    return 0


if __name__ == "__main__":
    sys.exit(main())
