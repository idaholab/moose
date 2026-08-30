#!/usr/bin/env python3
"""
Create a density-factor comparison plot for porous-baffle-2d-variable-density-bernoulli.i.

This compares:
- density_factor = 1e-3: analytic only, because the run fails before writing CSV output
- density_factor = 1e0: analytic and sampled data from archived CSV output
"""

from __future__ import annotations

import argparse
from pathlib import Path

from plot_variable_density_bernoulli_line_sampler import (
    dense_x_points,
    max_abs_error,
    parse_problem_data,
    read_csv,
    read_postprocessor_row,
)


SCRIPT_DIR = Path(__file__).resolve().parent
INPUT_PATH = SCRIPT_DIR / "porous-baffle-2d-variable-density-bernoulli.i"
CSV_1E0_PATH = (
    SCRIPT_DIR / "porous-baffle-2d-variable-density-bernoulli_df_1e0_out_centerline_solution_0001.csv"
)
POSTPROCESSOR_1E0_PATH = SCRIPT_DIR / "porous-baffle-2d-variable-density-bernoulli_df_1e0_out.csv"
DEFAULT_OUTPUT_PATH = (
    SCRIPT_DIR / "porous-baffle-2d-variable-density-bernoulli_density_factor_comparison.png"
)


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description=(
            "Compare the porous-baffle variable-density Bernoulli case at "
            "density_factor = 1e-3 and 1e0."
        )
    )
    parser.add_argument(
        "--output",
        type=Path,
        default=DEFAULT_OUTPUT_PATH,
        help="Output PNG path.",
    )
    parser.add_argument(
        "--show",
        action="store_true",
        help="Display the figure interactively after saving it.",
    )
    return parser.parse_args()


def sorted_columns(path: Path) -> tuple[list[float], dict[str, list[float]]]:
    columns = read_csv(path)
    pairs = sorted(zip(columns["x"], range(len(columns["x"]))))
    sort_indices = [index for _, index in pairs]
    ordered = {
        name: [values[index] for index in sort_indices]
        for name, values in columns.items()
    }
    return ordered["x"], ordered


def main() -> int:
    import matplotlib as mpl
    import matplotlib.pyplot as plt

    args = parse_args()

    mpl.rcParams.update(
        {
            "font.family": "serif",
            "font.serif": ["Computer Modern Roman", "CMU Serif", "DejaVu Serif"],
            "font.size": 15,
            "mathtext.fontset": "cm",
            "text.usetex": True,
            "axes.labelsize": 16,
            "axes.unicode_minus": False,
            "axes.grid": True,
            "grid.alpha": 0.25,
            "axes.spines.top": True,
            "axes.spines.right": True,
            "xtick.labelsize": 13,
            "ytick.labelsize": 13,
            "legend.fontsize": 12,
        }
    )

    problem_1em3 = parse_problem_data(INPUT_PATH, {"density_factor": "1e-3"})
    problem_1e0 = parse_problem_data(INPUT_PATH, {"density_factor": "1e0"})

    sample_x, columns_1e0 = sorted_columns(CSV_1E0_PATH)
    postprocessor_1e0 = read_postprocessor_row(POSTPROCESSOR_1E0_PATH)

    exact_x = dense_x_points(problem_1e0)

    rho_1em3 = [problem_1em3.rho(x) for x in exact_x]
    rho_1e0 = [problem_1e0.rho(x) for x in exact_x]
    porosity = [problem_1e0.porosity(x) for x in exact_x]
    pressure_1em3 = [problem_1em3.pressure(x) for x in exact_x]
    pressure_1e0 = [problem_1e0.pressure(x) for x in exact_x]
    velocity = [problem_1e0.superficial_u(x) for x in exact_x]

    sampled_pressure_exact_1e0 = [problem_1e0.pressure(x) for x in sample_x]
    sampled_velocity_exact_1e0 = [problem_1e0.superficial_u(x) for x in sample_x]
    sampled_density_exact_1e0 = [problem_1e0.rho(x) for x in sample_x]

    density_error_1e0 = max_abs_error(columns_1e0["rho_aux"], sampled_density_exact_1e0)
    pressure_error_1e0 = max_abs_error(columns_1e0["pressure"], sampled_pressure_exact_1e0)
    velocity_error_1e0 = max_abs_error(
        columns_1e0["superficial_u"], sampled_velocity_exact_1e0
    )

    fig, axes = plt.subplots(3, 1, sharex=True, figsize=(10, 11))
    density_ax, pressure_ax, velocity_ax = axes
    density_rho_ax = density_ax.twinx()

    def add_annotation(
        ax,
        text: str,
        edgecolor: str,
        xpos: float = 0.02,
        ypos: float = 0.98,
        va: str = "top",
    ) -> None:
        ax.text(
            xpos,
            ypos,
            text,
            transform=ax.transAxes,
            ha="left",
            va=va,
            fontsize=11,
            bbox={
                "boxstyle": "round",
                "facecolor": "white",
                "alpha": 0.9,
                "edgecolor": edgecolor,
            },
        )

    density_ax.plot(
        exact_x,
        porosity,
        color="#2ca02c",
        linewidth=2.0,
        linestyle="--",
        label="Porosity analytic",
    )
    density_ax.plot(
        sample_x,
        columns_1e0["porosity_aux"],
        color="#2ca02c",
        marker="s",
        markersize=3.5,
        linewidth=1.0,
        linestyle="-.",
        label=r"Porosity sampled ($df=1e0$)",
    )
    density_rho_ax.plot(
        exact_x,
        rho_1em3,
        color="#1f77b4",
        linewidth=2.0,
        linestyle=":",
        label=r"Density analytic ($df=1e\!-\!3$)",
    )
    density_rho_ax.plot(
        exact_x,
        rho_1e0,
        color="#9467bd",
        linewidth=2.0,
        label=r"Density analytic ($df=1e0$)",
    )
    density_rho_ax.plot(
        sample_x,
        columns_1e0["rho_aux"],
        color="#9467bd",
        marker="o",
        markersize=4,
        linewidth=1.1,
        label=r"Density sampled ($df=1e0$)",
    )
    density_ax.set_ylabel(r"$\epsilon$")
    density_rho_ax.set_ylabel(r"$\rho$")
    add_annotation(
        density_ax,
        r"$df=1e\!-\!3$: run failed before CSV output",
        "0.5",
        ypos=0.06,
        va="bottom",
    )

    pressure_ax.plot(
        exact_x,
        pressure_1em3,
        color="#d62728",
        linewidth=2.0,
        linestyle=":",
        label=r"Pressure analytic ($df=1e\!-\!3$)",
    )
    pressure_ax.plot(
        exact_x,
        pressure_1e0,
        color="#8c2d04",
        linewidth=2.0,
        label=r"Pressure analytic ($df=1e0$)",
    )
    pressure_ax.plot(
        sample_x,
        columns_1e0["pressure"],
        color="#111111",
        marker="o",
        markersize=4,
        linewidth=1.1,
        label=r"Pressure sampled ($df=1e0$)",
    )
    pressure_ax.set_ylabel(r"$p$")
    add_annotation(
        pressure_ax,
        rf"$L_2(p)$ ($df=1e0$) = {postprocessor_1e0['L2_pressure']:.3e}",
        "#d62728",
        ypos=0.06,
        va="bottom",
    )

    velocity_ax.plot(
        exact_x,
        velocity,
        color="#ff7f0e",
        linewidth=2.0,
        label=r"Velocity analytic ($df=1e\!-\!3$, $1e0$)",
    )
    velocity_ax.plot(
        sample_x,
        columns_1e0["superficial_u"],
        color="#111111",
        marker="o",
        markersize=4,
        linewidth=1.1,
        label=r"Velocity sampled ($df=1e0$)",
    )
    velocity_ax.set_ylabel(r"$U_s$")
    velocity_ax.set_xlabel(r"$x$")
    add_annotation(
        velocity_ax,
        rf"$L_2(U_s)$ ($df=1e0$) = {postprocessor_1e0['L2_superficial_u']:.3e}",
        "#ff7f0e",
    )

    for ax in axes:
        ax.axvline(
            problem_1e0.baffle_x,
            color="0.35",
            linestyle=":",
            linewidth=1.2,
            label="Baffle" if ax is density_ax else None,
        )
        ax.set_xlim(0.0, problem_1e0.domain_length)
        for spine in ax.spines.values():
            spine.set_visible(True)
            spine.set_linewidth(1.0)
        if ax is pressure_ax or ax is velocity_ax:
            ax.legend(loc="best", frameon=True, fontsize=12)

    density_lines, density_labels = density_ax.get_legend_handles_labels()
    rho_lines, rho_labels = density_rho_ax.get_legend_handles_labels()
    density_ax.legend(
        density_lines + rho_lines,
        density_labels + rho_labels,
        loc="best",
        frameon=True,
        fontsize=12,
    )

    args.output.parent.mkdir(parents=True, exist_ok=True)
    fig.tight_layout()
    fig.savefig(args.output, dpi=300, bbox_inches="tight")

    print(f"Input  {INPUT_PATH}")
    print(f"Read   {CSV_1E0_PATH}")
    print(f"Read   {POSTPROCESSOR_1E0_PATH}")
    print(f"Wrote  {args.output}")
    print("df=1e-3: run failed before writing CSV output; plotted analytic curves only.")
    print(f"df=1e0: L2_pressure = {postprocessor_1e0['L2_pressure']:.6e}")
    print(f"df=1e0: L2_superficial_u = {postprocessor_1e0['L2_superficial_u']:.6e}")
    print(f"df=1e0: max |rho - rho_exact| = {density_error_1e0:.6e}")
    print(f"df=1e0: max |p - p_exact|   = {pressure_error_1e0:.6e}")
    print(f"df=1e0: max |u - u_exact|   = {velocity_error_1e0:.6e}")

    if args.show:
        plt.show()
    else:
        plt.close(fig)

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
