#!/usr/bin/env python3
from __future__ import annotations

import argparse
import csv
from pathlib import Path


def _read_csv_columns(path: Path) -> dict[str, list[float]]:
    with path.open(newline="") as f:
        reader = csv.DictReader(f)
        if not reader.fieldnames:
            raise ValueError(f"No header found in {path}")

        cols: dict[str, list[float]] = {name: [] for name in reader.fieldnames}
        for row in reader:
            for name in reader.fieldnames:
                value = row.get(name, "")
                if value is None or value == "":
                    cols[name].append(float("nan"))
                else:
                    cols[name].append(float(value))

    return cols


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Plot MOOSE LineValueSampler CSV output (e.g. diagonal-step-2d_out_diag_sample_0001.csv)."
    )
    parser.add_argument(
        "csv",
        nargs="?",
        default="diagonal-step-2d_out_diag_sample_0001.csv",
        help="Path to the CSV file (default: diagonal-step-2d_out_diag_sample_0001.csv)",
    )
    parser.add_argument("--x", default="id", help="Column name for the x-axis (default: id)")
    parser.add_argument(
        "--y",
        default=["u"],
        help="Column name for the y-axis (default: u). Repeatable.",
    )
    parser.add_argument(
        "-o",
        "--output",
        help="Output image path (default: <csv stem>.png next to the CSV)",
    )
    parser.add_argument("--title", default=None, help="Plot title (default: CSV filename)")
    parser.add_argument(
        "--show",
        action="store_true",
        help="Show an interactive window instead of saving (still saves if --output is set)",
    )
    args = parser.parse_args()

    csv_path = Path(args.csv)
    if not csv_path.exists():
        raise SystemExit(f"CSV not found: {csv_path}")

    cols = _read_csv_columns(csv_path)

    if args.x not in cols:
        raise SystemExit(f"Missing x column '{args.x}'. Available: {', '.join(cols.keys())}")
    for y in args.y:
        if y not in cols:
            raise SystemExit(f"Missing y column '{y}'. Available: {', '.join(cols.keys())}")

    try:
        import matplotlib.pyplot as plt
    except ModuleNotFoundError as e:
        raise SystemExit(
            "matplotlib is required for plotting. Activate the moose conda env (conda activate moose) "
            "and try again."
        ) from e

    x = cols[args.x]
    order = sorted(range(len(x)), key=lambda i: x[i])

    fig, ax = plt.subplots()
    for y in args.y:
        ax.plot([x[i] for i in order], [cols[y][i] for i in order], ms=3, label=y)

    ax.set_xlabel(args.x)
    ax.set_ylabel(", ".join(args.y))
    ax.grid(True, alpha=0.3)
    ax.set_title(args.title or csv_path.name)
    if len(args.y) > 1:
        ax.legend()

    fig.tight_layout()
    if args.output or not args.show:
        output = Path(args.output) if args.output else csv_path.with_suffix(".png")
        fig.savefig(output, dpi=200)
        print(f"Wrote {output}")

    if args.show:
        plt.show()

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
