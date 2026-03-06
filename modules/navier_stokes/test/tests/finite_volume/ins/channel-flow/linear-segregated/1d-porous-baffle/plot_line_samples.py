#!/usr/bin/env python3
"""
Plot LineValueSampler CSV outputs for velocity and pressure with LaTeX-style fonts.
"""

from __future__ import annotations

import csv
import re
import sys
from pathlib import Path


def _read_csv(path: Path):
    with path.open("r", newline="") as f:
        reader = csv.reader(f)
        header = None
        rows = []
        for row in reader:
            if not row:
                continue
            if row[0].lstrip().startswith("#"):
                continue
            if header is None:
                header = [h.strip() for h in row]
                continue
            rows.append(row)

    if header is None:
        raise RuntimeError(f"No header found in {path}")

    cols = {name: [] for name in header}
    for row in rows:
        if len(row) != len(header):
            raise RuntimeError(f"Row length mismatch in {path}")
        for name, val in zip(header, row):
            cols[name].append(float(val))

    return cols


def _pick_x(cols):
    if "x" not in cols:
        raise RuntimeError("Expected an 'x' column in the CSV")
    return "x", cols["x"]


def _pick_y(cols, y_col):
    if y_col not in cols:
        raise RuntimeError(f"Column '{y_col}' not found in CSV")
    return y_col, cols[y_col]


def _build_figure(u_series, p_series, x_label, title, u_styles, p_styles, labels):
    import matplotlib as mpl
    import matplotlib.pyplot as plt
    from matplotlib.ticker import MultipleLocator

    mpl.rcParams.update(
        {
            "font.family": "serif",
            "font.serif": ["Computer Modern Roman", "CMU Serif", "DejaVu Serif"],
            "mathtext.fontset": "cm",
            "axes.unicode_minus": False,
            "axes.grid": False,
            "grid.alpha": 0.3,
            "axes.spines.top": True,
            "axes.spines.right": True,
        }
    )

    fig, (ax1, ax2) = plt.subplots(1, 2, sharex=False, figsize=(10, 4))

    for (x_u, u), style, label in zip(u_series, u_styles, labels):
        ax1.plot(
            x_u,
            u,
            color="#1f77b4",
            linestyle=style,
            linewidth=1.4,
            alpha=0.9,
            label=label,
        )
    ax1.set_ylabel(r"Superficial velocity [m/s]")
    ax1.set_xlabel(x_label)
    ax1.set_title(title)

    for (x_p, p), style, label in zip(p_series, p_styles, labels):
        ax2.plot(
            x_p,
            p,
            color="#d62728",
            linestyle=style,
            linewidth=1.4,
            alpha=0.9,
            label=label,
        )
    ax2.set_ylabel(r"Pressure [Pa]")
    ax2.set_xlabel(x_label)

    for ax in (ax1, ax2):
        for spine in ax.spines.values():
            spine.set_visible(True)
            spine.set_linewidth(1.0)
        ax.axvline(0.5, color="0.2", linestyle="--", linewidth=1.0, alpha=0.7)
        ax.axvline(1.0, color="0.2", linestyle="--", linewidth=1.0, alpha=0.7)
        ax.set_xlim(left=0.0, right=1.5)
        ax.xaxis.set_major_locator(MultipleLocator(0.1))
        ax.ticklabel_format(style="plain", axis="both", useOffset=False)

    fig.tight_layout()
    if labels:
        legend_kwargs = dict(
            loc="best",
            frameon=True,
            fontsize=8,
            ncol=1,
            handlelength=2.5,
        )
        ax1.legend(**legend_kwargs)
        ax2.legend(**legend_kwargs)
    return fig


def _save_with_latex(out_path, make_fig):
    import matplotlib as mpl
    import matplotlib.pyplot as plt

    # Attempt LaTeX first; if it fails (missing LaTeX), fall back to mathtext.
    mpl.rcParams["text.usetex"] = True
    fig = make_fig()
    try:
        fig.savefig(out_path, dpi=300, bbox_inches="tight")
        plt.close(fig)
        return
    except RuntimeError as e:
        plt.close(fig)
        if "latex" not in str(e).lower():
            raise

    # Fallback without external LaTeX
    mpl.rcParams["text.usetex"] = False
    fig = make_fig()
    fig.savefig(out_path, dpi=300, bbox_inches="tight")
    plt.close(fig)


def _read_matrix_rows(path: Path):
    with path.open("r", newline="") as f:
        reader = csv.reader(f)
        header = None
        rows = []
        for row in reader:
            if not row:
                continue
            if row[0].lstrip().startswith("#"):
                continue
            if header is None:
                header = [h.strip() for h in row]
                continue
            rows.append([float(val) for val in row])

    if header is None:
        raise RuntimeError(f"No header found in {path}")

    return rows


def main(argv):
    if argv:
        raise RuntimeError("This script takes no arguments.")

    pattern_candidates = [
        "porous-baffle-2d-stochastic_out_study*_u_line_*.csv",
        "porous-baffle-2d-stochastic_out_u_line_*.csv",
        "porous-baffle-2d-param-study_out_u_line_*.csv",
        "porous-baffle-2d_out_u_line_*.csv",
    ]

    u_files = []
    p_files = []
    for pattern in pattern_candidates:
        u_files = sorted(Path(".").glob(pattern))
        if not u_files:
            continue
        p_pattern = pattern.replace("_u_line_", "_p_line_")
        p_files = sorted(Path(".").glob(p_pattern))
        if p_files:
            break

    if not u_files or not p_files:
        raise RuntimeError("No line sample CSV files found for the expected patterns.")

    def _sample_key(path: Path):
        study_match = re.search(r"_study(\d+)_", path.name)
        study_index = int(study_match.group(1)) if study_match else -1
        sample_match = re.search(r"_line_(\d+)\.csv$", path.name)
        if not sample_match:
            raise RuntimeError(f"Could not parse sample index from {path}")
        sample_index = int(sample_match.group(1))
        return (study_index, sample_index)

    u_map = {_sample_key(path): path for path in u_files}
    p_map = {_sample_key(path): path for path in p_files}

    if set(u_map.keys()) != set(p_map.keys()):
        raise RuntimeError("Mismatch between velocity and pressure sample files.")

    sample_indices = sorted(u_map.keys())

    u_series = []
    p_series = []
    for idx in sample_indices:
        u_cols = _read_csv(u_map[idx])
        p_cols = _read_csv(p_map[idx])

        _, x_u = _pick_x(u_cols)
        _, x_p = _pick_x(p_cols)

        _, u = _pick_y(u_cols, "superficial_u")
        _, p = _pick_y(p_cols, "pressure")

        u_series.append((x_u, u))
        p_series.append((x_p, p))

    x_label = r"$x$"
    param_names = [r"$U_{in}$", r"$\mu$", r"$K$", r"$F_f$"]
    sample_matrix_files = sorted(
        Path(".").glob("porous-baffle-2d-stochastic_out_sample_matrix_*.csv")
    )
    sample_rows = []
    if sample_matrix_files:
        sample_rows = _read_matrix_rows(sample_matrix_files[-1])

    labels = []
    for idx, key in enumerate(sample_indices):
        row = None
        if sample_rows:
            study_index = key[0]
            if study_index >= 0 and study_index < len(sample_rows):
                row = sample_rows[study_index]
            elif idx < len(sample_rows):
                row = sample_rows[idx]
        if row:
            label = ", ".join(
                f"{name}={value:.6g}" for name, value in zip(param_names, row)
            )
        else:
            label = f"sample {idx + 1}"
        labels.append(label)

    def make_fig():
        base_styles = [
            "-",
            "--",
            "-.",
            ":",
            (0, (5, 1)),
            (0, (3, 1, 1, 1)),
            (0, (1, 1)),
        ]
        u_styles = [base_styles[i % len(base_styles)] for i in range(len(u_series))]
        p_styles = [base_styles[i % len(base_styles)] for i in range(len(p_series))]
        return _build_figure(
            u_series=u_series,
            p_series=p_series,
            x_label=x_label,
            title="",
            u_styles=u_styles,
            p_styles=p_styles,
            labels=labels,
        )

    out_path = Path("line_samples.png")
    _save_with_latex(out_path, make_fig)
    print(f"Wrote {out_path}")


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
