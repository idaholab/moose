#!/usr/bin/env python3
"""
Plot the centerline LineValueSampler output for
porous-baffle-2d-variable-density-bernoulli.i against an exact 1D reference
read from the input file.

Assumptions:
- two constant-porosity blocks split by a single internal baffle at x = length_left
- a linear density law rho(x)
- steady 1D superficial-velocity formulation
- only a reversible Bernoulli jump across the baffle, with no form-loss term
"""

from __future__ import annotations

import argparse
import ast
import csv
import operator
import os
import re
import sys
import tempfile
from dataclasses import dataclass
from pathlib import Path


SCRIPT_DIR = Path(__file__).resolve().parent
CACHE_DIR = Path(tempfile.gettempdir()) / "moose_plot_cache"
MPLCONFIGDIR = CACHE_DIR / "matplotlib"
XDG_CACHE_HOME = CACHE_DIR / "xdg"
PLACEHOLDER_PATTERN = re.compile(r"\$\{([^{}]+)\}")

MPLCONFIGDIR.mkdir(parents=True, exist_ok=True)
XDG_CACHE_HOME.mkdir(parents=True, exist_ok=True)
os.environ.setdefault("MPLCONFIGDIR", str(MPLCONFIGDIR))
os.environ.setdefault("XDG_CACHE_HOME", str(XDG_CACHE_HOME))

ALLOWED_BINARY_OPERATORS = {
    ast.Add: operator.add,
    ast.Sub: operator.sub,
    ast.Mult: operator.mul,
    ast.Div: operator.truediv,
    ast.Pow: operator.pow,
}
ALLOWED_UNARY_OPERATORS = {
    ast.UAdd: operator.pos,
    ast.USub: operator.neg,
}


@dataclass(frozen=True)
class ProblemData:
    input_path: Path
    length_left: float
    length_right: float
    epsilon_left: float
    epsilon_right: float
    u_in: float
    p_out: float
    mu: float
    density_factor: float
    rho_expression: str
    rho_intercept: float
    rho_slope: float

    @property
    def domain_length(self) -> float:
        return self.length_left + self.length_right

    @property
    def baffle_x(self) -> float:
        return self.length_left

    @property
    def mass_flux(self) -> float:
        return self.rho(0.0) * self.u_in

    def rho(self, x: float) -> float:
        rho = self.rho_intercept + self.rho_slope * x
        if rho <= 0.0:
            raise RuntimeError(f"Exact density is non-positive at x={x}")
        return rho

    def porosity(self, x: float) -> float:
        return self.epsilon_left if x <= self.baffle_x + 1e-12 else self.epsilon_right

    def superficial_u(self, x: float) -> float:
        return self.mass_flux / self.rho(x)

    def superficial_u_prime(self, x: float) -> float:
        rho = self.rho(x)
        return -self.mass_flux * self.rho_slope / (rho * rho)

    def transport_potential(self, x: float, epsilon: float) -> float:
        if epsilon == 0.0:
            raise RuntimeError("Porosity must be non-zero in the analytic solution.")
        return (
            self.mass_flux * self.superficial_u(x) / (epsilon * epsilon)
            - self.mu * self.superficial_u_prime(x) / epsilon
        )

    def bernoulli_jump(self) -> float:
        """
        Return p_left - p_right across the baffle for the Bernoulli-only jump.

        The current input uses a continuous density field, so the face density is
        simply rho(length_left).
        """
        rho_face = self.rho(self.baffle_x)
        u_left = self.mass_flux / (rho_face * self.epsilon_left)
        u_right = self.mass_flux / (rho_face * self.epsilon_right)
        return 0.5 * rho_face * (u_right * u_right - u_left * u_left)

    def pressure(self, x: float) -> float:
        if x <= self.baffle_x + 1e-12:
            p_right_at_baffle = self.p_out + self.transport_potential(
                self.domain_length, self.epsilon_right
            ) - self.transport_potential(self.baffle_x, self.epsilon_right)
            p_left_at_baffle = p_right_at_baffle + self.bernoulli_jump()
            return p_left_at_baffle + self.transport_potential(
                self.baffle_x, self.epsilon_left
            ) - self.transport_potential(x, self.epsilon_left)

        return self.p_out + self.transport_potential(
            self.domain_length, self.epsilon_right
        ) - self.transport_potential(x, self.epsilon_right)


def parse_args(argv: list[str]) -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description=(
            "Plot the porous-baffle variable-density Bernoulli line sampler "
            "against the exact 1D solution extracted from the input file."
        )
    )
    parser.add_argument(
        "--csv",
        type=Path,
        help=(
            "Path to the LineValueSampler CSV. Defaults to the newest matching "
            "porous-baffle-2d-variable-density-bernoulli centerline file."
        ),
    )
    parser.add_argument(
        "--input",
        type=Path,
        help=(
            "Path to the corresponding MOOSE input file. Defaults to the input "
            "inferred from the CSV filename."
        ),
    )
    parser.add_argument(
        "--output",
        type=Path,
        help="Output PNG path. Defaults to <input_stem>_line_samples.png.",
    )
    parser.add_argument(
        "--postprocessor-csv",
        type=Path,
        help="Path to the scalar postprocessor CSV. Defaults to <input_stem>_out.csv.",
    )
    parser.add_argument(
        "--density-factor",
        type=float,
        help=(
            "Override density_factor when building the analytic solution from the input file."
        ),
    )
    parser.add_argument(
        "--input-override",
        action="append",
        default=[],
        metavar="NAME=VALUE",
        help=(
            "Override additional scalar definitions from the input file when building "
            "the analytic solution. May be provided multiple times."
        ),
    )
    parser.add_argument(
        "--show",
        action="store_true",
        help="Display the figure interactively after saving it.",
    )
    return parser.parse_args(argv)


def parse_scalar_overrides(
    raw_overrides: list[str], density_factor: float | None
) -> dict[str, str] | None:
    overrides: dict[str, str] = {}
    for raw in raw_overrides:
        if "=" not in raw:
            raise ValueError(
                f"Invalid --input-override {raw!r}; expected NAME=VALUE format."
            )
        name, value = raw.split("=", 1)
        name = name.strip()
        value = value.strip()
        if not name or not value:
            raise ValueError(
                f"Invalid --input-override {raw!r}; expected NAME=VALUE format."
            )
        overrides[name] = value

    if density_factor is not None:
        overrides["density_factor"] = repr(density_factor)

    return overrides or None


def resolve_default_csv() -> Path:
    candidates = sorted(
        SCRIPT_DIR.glob(
            "porous-baffle-2d-variable-density-bernoulli_out_centerline_solution_*.csv"
        )
    )
    if not candidates:
        raise RuntimeError(
            "No centerline_solution CSV found for "
            "porous-baffle-2d-variable-density-bernoulli."
        )
    return candidates[-1]


def infer_input_path(csv_path: Path) -> Path:
    match = re.match(r"(.+)_out_centerline_solution_\d+\.csv$", csv_path.name)
    if not match:
        raise RuntimeError(
            f"Could not infer an input file from CSV name '{csv_path.name}'."
        )
    return csv_path.with_name(f"{match.group(1)}.i")


def infer_postprocessor_csv(input_path: Path) -> Path:
    return input_path.with_name(f"{input_path.stem}_out.csv")


def strip_inline_comment(line: str) -> str:
    in_single_quote = False
    in_double_quote = False
    for index, char in enumerate(line):
        if char == "'" and not in_double_quote:
            in_single_quote = not in_single_quote
        elif char == '"' and not in_single_quote:
            in_double_quote = not in_double_quote
        elif char == "#" and not in_single_quote and not in_double_quote:
            return line[:index]
    return line


def unquote(text: str) -> str:
    stripped = text.strip()
    if len(stripped) >= 2 and stripped[0] == stripped[-1] and stripped[0] in {"'", '"'}:
        return stripped[1:-1]
    return stripped


def evaluate_numeric_expression(expression: str, names: dict[str, float] | None = None) -> float:
    node = ast.parse(expression, mode="eval")
    available_names = names or {}

    def _evaluate(current: ast.AST) -> float:
        if isinstance(current, ast.Expression):
            return _evaluate(current.body)
        if isinstance(current, ast.Constant) and isinstance(current.value, (int, float)):
            return float(current.value)
        if isinstance(current, ast.Name) and current.id in available_names:
            return float(available_names[current.id])
        if isinstance(current, ast.UnaryOp) and type(current.op) in ALLOWED_UNARY_OPERATORS:
            return ALLOWED_UNARY_OPERATORS[type(current.op)](_evaluate(current.operand))
        if isinstance(current, ast.BinOp) and type(current.op) in ALLOWED_BINARY_OPERATORS:
            return ALLOWED_BINARY_OPERATORS[type(current.op)](
                _evaluate(current.left), _evaluate(current.right)
            )
        raise RuntimeError(f"Unsupported numeric expression: {expression}")

    return float(_evaluate(node))


def resolve_text(text: str, scalars: dict[str, str]) -> str:
    resolved = text.strip()

    while True:
        match = PLACEHOLDER_PATTERN.search(resolved)
        if not match:
            return resolved

        token = match.group(1).strip()
        if token.startswith("fparse "):
            numeric_value = evaluate_numeric_expression(
                unquote(resolve_text(token[len("fparse ") :], scalars))
            )
            replacement = repr(numeric_value)
        else:
            if token not in scalars:
                raise RuntimeError(f"Unknown input variable '{token}' in '{text}'.")
            replacement = resolve_text(scalars[token], scalars)

        resolved = resolved[: match.start()] + replacement + resolved[match.end() :]


def resolve_float(name: str, scalars: dict[str, str]) -> float:
    if name not in scalars:
        raise RuntimeError(f"Required input variable '{name}' not found.")
    return evaluate_numeric_expression(unquote(resolve_text(scalars[name], scalars)))


def parse_moose_input(path: Path) -> tuple[dict[str, str], dict[tuple[str, ...], dict[str, str]]]:
    scalars: dict[str, str] = {}
    sections: dict[tuple[str, ...], dict[str, str]] = {}
    stack: list[str] = []

    for raw_line in path.read_text().splitlines():
        line = strip_inline_comment(raw_line).strip()
        if not line:
            continue

        if line == "[]":
            if stack:
                stack.pop()
            continue

        section_match = re.fullmatch(r"\[(.+)\]", line)
        if section_match:
            stack.append(section_match.group(1).strip())
            sections.setdefault(tuple(stack), {})
            continue

        if "=" not in line:
            continue

        key, value = [piece.strip() for piece in line.split("=", 1)]
        if stack:
            sections.setdefault(tuple(stack), {})[key] = value
        else:
            scalars[key] = value

    return scalars, sections


def parse_problem_data(
    path: Path, scalar_overrides: dict[str, str] | None = None
) -> ProblemData:
    scalars, sections = parse_moose_input(path)
    if scalar_overrides:
        scalars.update(scalar_overrides)

    rho_section = sections.get(("Functions", "rho"))
    if rho_section is None or "expression" not in rho_section:
        raise RuntimeError(
            f"Could not find a [Functions]/[rho] expression in {path}."
        )

    rc_section = sections.get(("UserObjects", "rc"), {})
    if "baffle_form_loss" in rc_section:
        resolved_form_loss = unquote(resolve_text(rc_section["baffle_form_loss"], scalars))
        tokens = resolved_form_loss.split()
        if any(abs(evaluate_numeric_expression(token)) > 1e-14 for token in tokens):
            raise RuntimeError(
                "This plotter assumes a Bernoulli-only jump, but non-zero "
                "baffle_form_loss was found in the input file."
            )

    rho_expression = unquote(resolve_text(rho_section["expression"], scalars))
    rho_at_zero = evaluate_numeric_expression(rho_expression, {"x": 0.0})
    rho_at_half = evaluate_numeric_expression(rho_expression, {"x": 0.5})
    rho_at_one = evaluate_numeric_expression(rho_expression, {"x": 1.0})
    rho_slope = rho_at_one - rho_at_zero
    rho_half_expected = rho_at_zero + 0.5 * rho_slope
    linearity_tolerance = 1e-12 + 1e-10 * max(
        abs(rho_at_zero), abs(rho_at_half), abs(rho_at_one), 1.0
    )

    if abs(rho_at_half - rho_half_expected) > linearity_tolerance:
        raise RuntimeError(
            "The rho(x) expression is not linear in x. "
            "This script currently supports only linear density laws."
        )

    return ProblemData(
        input_path=path,
        length_left=resolve_float("length_left", scalars),
        length_right=resolve_float("length_right", scalars),
        epsilon_left=resolve_float("epsilon_left", scalars),
        epsilon_right=resolve_float("epsilon_right", scalars),
        u_in=resolve_float("u_in", scalars),
        p_out=resolve_float("p_out", scalars),
        mu=resolve_float("mu", scalars),
        density_factor=resolve_float("density_factor", scalars),
        rho_expression=rho_expression,
        rho_intercept=rho_at_zero,
        rho_slope=rho_slope,
    )


def read_csv(path: Path) -> dict[str, list[float]]:
    with path.open("r", newline="") as handle:
        reader = csv.reader(handle)
        header = None
        rows = []
        for row in reader:
            if not row:
                continue
            if row[0].lstrip().startswith("#"):
                continue
            if header is None:
                header = [entry.strip() for entry in row]
                continue
            rows.append(row)

    if header is None:
        raise RuntimeError(f"No CSV header found in {path}")

    columns = {name: [] for name in header}
    for row in rows:
        if len(row) != len(header):
            raise RuntimeError(f"Row length mismatch in {path}")
        for name, value in zip(header, row):
            columns[name].append(float(value))

    required = {"x", "rho_aux", "superficial_u", "pressure"}
    missing = required.difference(columns)
    if missing:
        missing_columns = ", ".join(sorted(missing))
        raise RuntimeError(f"Missing required column(s) in {path}: {missing_columns}")

    return columns


def read_postprocessor_row(path: Path) -> dict[str, float]:
    with path.open("r", newline="") as handle:
        reader = csv.DictReader(handle)
        rows = list(reader)

    if not rows:
        raise RuntimeError(f"No postprocessor rows found in {path}")

    last_row = rows[-1]
    result: dict[str, float] = {}
    for name, value in last_row.items():
        if value is None or not value.strip():
            continue
        result[name] = float(value)

    return result


def max_abs_error(values: list[float], reference: list[float]) -> float:
    return max(abs(value - ref) for value, ref in zip(values, reference))


def dense_x_points(problem: ProblemData, count: int = 600) -> list[float]:
    if count < 2:
        return [0.0]
    step = problem.domain_length / (count - 1)
    return [i * step for i in range(count)]


def build_plot(
    problem: ProblemData,
    sample_x: list[float],
    columns: dict[str, list[float]],
    csv_path: Path,
    postprocessor_csv_path: Path | None,
    postprocessor_values: dict[str, float] | None,
    output_path: Path,
    show: bool,
) -> None:
    import matplotlib as mpl
    import matplotlib.pyplot as plt

    mpl.rcParams.update(
        {
            "font.family": "serif",
            "font.serif": ["Computer Modern Roman", "CMU Serif", "DejaVu Serif"],
            "font.size": 15,
            "mathtext.fontset": "cm",
            "text.usetex": True,
            "axes.titlesize": 17,
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

    exact_x = dense_x_points(problem)
    exact_density = [problem.rho(x) for x in exact_x]
    exact_porosity = [problem.porosity(x) for x in exact_x]
    exact_pressure = [problem.pressure(x) for x in exact_x]
    exact_velocity = [problem.superficial_u(x) for x in exact_x]

    sampled_density_exact = [problem.rho(x) for x in sample_x]
    sampled_porosity_exact = [problem.porosity(x) for x in sample_x]
    sampled_pressure_exact = [problem.pressure(x) for x in sample_x]
    sampled_velocity_exact = [problem.superficial_u(x) for x in sample_x]

    sampled_porosity = columns.get("porosity_aux")

    density_error = max_abs_error(columns["rho_aux"], sampled_density_exact)
    pressure_error = max_abs_error(columns["pressure"], sampled_pressure_exact)
    velocity_error = max_abs_error(columns["superficial_u"], sampled_velocity_exact)
    porosity_error = (
        max_abs_error(sampled_porosity, sampled_porosity_exact)
        if sampled_porosity is not None
        else None
    )

    fig, axes = plt.subplots(3, 1, sharex=True, figsize=(10, 11))
    density_ax, pressure_ax, velocity_ax = axes
    density_rho_ax = density_ax.twinx()

    def add_annotation(
        ax,
        lines: list[str],
        edgecolor: str,
        xpos: float = 0.02,
        ypos: float = 0.98,
        va: str = "top",
    ) -> None:
        if not lines:
            return
        ax.text(
            xpos,
            ypos,
            "\n".join(lines),
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

    density_rho_ax.plot(
        exact_x,
        exact_density,
        color="#1f77b4",
        linewidth=2.0,
        label="Density analytic",
    )
    density_rho_ax.plot(
        sample_x,
        columns["rho_aux"],
        color="#1f77b4",
        marker="o",
        markersize=4,
        linewidth=1.1,
        label="Density sampled",
    )
    density_ax.plot(
        exact_x,
        exact_porosity,
        color="#2ca02c",
        linewidth=2.0,
        linestyle="--",
        label="Porosity analytic",
    )
    if sampled_porosity is not None:
        density_ax.plot(
            sample_x,
            sampled_porosity,
            color="#2ca02c",
            marker="s",
            markersize=3.5,
            linewidth=1.0,
            linestyle="-.",
            label="Porosity sampled",
        )
    else:
        density_ax.text(
            0.02,
            0.05,
            "CSV has no sampled porosity column.\nShowing analytic porosity only.",
            transform=density_ax.transAxes,
            fontsize=11,
            bbox={
                "boxstyle": "round",
                "facecolor": "white",
                "alpha": 0.85,
                "edgecolor": "0.8",
            },
        )
    density_ax.set_ylabel(r"$\epsilon$")
    density_rho_ax.set_ylabel(r"$\rho$")
    pressure_ax.plot(
        exact_x,
        exact_pressure,
        color="#d62728",
        linewidth=2.0,
        label="Pressure analytic",
    )
    pressure_ax.plot(
        sample_x,
        columns["pressure"],
        color="#111111",
        marker="o",
        markersize=4,
        linewidth=1.1,
        label="Pressure sampled",
    )
    pressure_ax.set_ylabel(r"$p$")

    velocity_ax.plot(
        exact_x,
        exact_velocity,
        color="#ff7f0e",
        linewidth=2.0,
        label="Velocity analytic",
    )
    velocity_ax.plot(
        sample_x,
        columns["superficial_u"],
        color="#111111",
        marker="o",
        markersize=4,
        linewidth=1.1,
        label="Velocity sampled",
    )
    velocity_ax.set_ylabel(r"$U_s$")
    velocity_ax.set_xlabel(r"$x$")

    if postprocessor_values is not None:
        add_annotation(
            pressure_ax,
            [
                rf"$L_2(p)$ = {postprocessor_values['L2_pressure']:.3e}"
                for _ in [0]
                if "L2_pressure" in postprocessor_values
            ],
            "#d62728",
            ypos=0.06,
            va="bottom",
        )
        add_annotation(
            velocity_ax,
            [
                rf"$L_2(U_s)$ = {postprocessor_values['L2_superficial_u']:.3e}"
                for _ in [0]
                if "L2_superficial_u" in postprocessor_values
            ],
            "#ff7f0e",
        )

    for ax in axes:
        ax.axvline(
            problem.baffle_x,
            color="0.35",
            linestyle=":",
            linewidth=1.2,
            label="Baffle" if ax is density_ax else None,
        )
        ax.set_xlim(0.0, problem.domain_length)
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

    fig.tight_layout()
    output_path.parent.mkdir(parents=True, exist_ok=True)
    fig.savefig(output_path, dpi=300, bbox_inches="tight")

    print(f"Input  {problem.input_path}")
    print(f"Read   {csv_path}")
    if postprocessor_values is not None and postprocessor_csv_path is not None:
        print(f"Read   {postprocessor_csv_path}")
    print(f"Wrote  {output_path}")
    print(f"rho(x) = {problem.rho_expression}")
    print(
        "eps_left = {:.6g}, eps_right = {:.6g}, density_factor = {:.6g}".format(
            problem.epsilon_left, problem.epsilon_right, problem.density_factor
        )
    )
    print(f"Bernoulli jump (p_left - p_right) = {problem.bernoulli_jump():.6e}")
    if postprocessor_values is not None:
        for key in ("L2_rho", "L2_porosity", "L2_pressure", "L2_superficial_u"):
            if key in postprocessor_values:
                print(f"{key} = {postprocessor_values[key]:.6e}")
    print(f"max |rho - rho_exact| = {density_error:.6e}")
    if porosity_error is not None:
        print(f"max |eps - eps_exact| = {porosity_error:.6e}")
    else:
        print("max |eps - eps_exact| = not available (no porosity_aux column in CSV)")
    print(f"max |p - p_exact|   = {pressure_error:.6e}")
    print(f"max |u - u_exact|   = {velocity_error:.6e}")

    if show:
        plt.show()
    else:
        plt.close(fig)


def main(argv: list[str]) -> int:
    args = parse_args(argv)
    csv_path = args.csv.resolve() if args.csv else resolve_default_csv()
    input_path = args.input.resolve() if args.input else infer_input_path(csv_path)
    postprocessor_csv_path = (
        args.postprocessor_csv.resolve()
        if args.postprocessor_csv
        else infer_postprocessor_csv(input_path)
    )
    output_path = (
        args.output.resolve()
        if args.output
        else input_path.with_name(f"{input_path.stem}_line_samples.png")
    )

    problem = parse_problem_data(
        input_path, parse_scalar_overrides(args.input_override, args.density_factor)
    )
    columns = read_csv(csv_path)
    postprocessor_values = (
        read_postprocessor_row(postprocessor_csv_path)
        if postprocessor_csv_path.exists()
        else None
    )

    pairs = sorted(zip(columns["x"], range(len(columns["x"]))))
    sort_indices = [index for _, index in pairs]
    sorted_columns = {
        name: [values[index] for index in sort_indices]
        for name, values in columns.items()
    }
    sample_x = sorted_columns["x"]

    build_plot(
        problem,
        sample_x,
        sorted_columns,
        csv_path,
        postprocessor_csv_path if postprocessor_values is not None else None,
        postprocessor_values,
        output_path,
        args.show,
    )
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
