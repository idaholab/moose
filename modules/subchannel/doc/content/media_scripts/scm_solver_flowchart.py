"""Create the SCM iteration-scheme flowchart introduced in MOOSE PR #33446.

The diagram shows the nested solver structure:

1. Outer pressure iteration.
2. Downstream block sweep.
3. One main-flow solve per temperature-loop iteration.
4. One or more enthalpy/temperature/property subcycles per flow solve.
5. Temperature-loop and pressure-loop completion checks.

Run
---
python scm_solver_flowchart.py
"""

from __future__ import annotations

from pathlib import Path
import textwrap
from typing import Sequence

import matplotlib.pyplot as plt
from matplotlib.patches import Ellipse, FancyArrowPatch, FancyBboxPatch, Polygon

# DejaVu Sans ships with Matplotlib and is available consistently across
# Windows, macOS, and Linux installations that include Matplotlib.
plt.rcParams.update(
    {
        "font.family": "sans-serif",
        "font.sans-serif": ["DejaVu Sans", "Arial", "Liberation Sans", "sans-serif"],
        "font.weight": "regular",
    }
)


# ---------------------------------------------------------------------------
# Visual style
# ---------------------------------------------------------------------------
TEXT = "#1F2937"
LINE = "#334155"
PROCESS_FILL = "#EEF4FA"
THERMAL_FILL = "#FFF8E7"
ACTION_FILL = "#EAF6EE"
DECISION_FILL = "#FFF1C7"
TERMINATOR_FILL = "#E8F0FB"
NOTE_FILL = "#F8FAFC"
YES_COLOR = "#247A5A"
NO_COLOR = "#B34A3C"

LINE_WIDTH = 1.85
ARROW_SCALE = 13


def _anchors(
    x: float, y: float, width: float, height: float
) -> dict[str, tuple[float, float]]:
    return {
        "center": (x, y),
        "top": (x, y + height / 2),
        "bottom": (x, y - height / 2),
        "left": (x - width / 2, y),
        "right": (x + width / 2, y),
    }


def add_process(
    ax,
    center: tuple[float, float],
    width: float,
    height: float,
    text: str,
    *,
    fill: str = PROCESS_FILL,
    fontsize: float = 11.3,
    wrap: int = 36,
) -> dict[str, tuple[float, float]]:
    """Add a rounded process block."""
    x, y = center
    patch = FancyBboxPatch(
        (x - width / 2, y - height / 2),
        width,
        height,
        boxstyle="round,pad=0.04,rounding_size=0.10",
        facecolor=fill,
        edgecolor=LINE,
        linewidth=LINE_WIDTH,
        zorder=3,
    )
    ax.add_patch(patch)
    ax.text(
        x,
        y,
        textwrap.fill(text, width=wrap),
        ha="center",
        va="center",
        fontsize=fontsize,
        color=TEXT,
        linespacing=1.10,
        fontweight="regular",
        zorder=4,
    )
    return _anchors(x, y, width, height)


def add_terminator(
    ax,
    center: tuple[float, float],
    width: float,
    height: float,
    text: str,
    *,
    fontsize: float = 13.0,
) -> dict[str, tuple[float, float]]:
    """Add a start/end terminator."""
    x, y = center
    patch = Ellipse(
        (x, y),
        width,
        height,
        facecolor=TERMINATOR_FILL,
        edgecolor=LINE,
        linewidth=LINE_WIDTH,
        zorder=3,
    )
    ax.add_patch(patch)
    ax.text(
        x,
        y,
        text,
        ha="center",
        va="center",
        fontsize=fontsize,
        fontweight="medium",
        color=TEXT,
        zorder=4,
    )
    return _anchors(x, y, width, height)


def add_decision(
    ax,
    center: tuple[float, float],
    width: float,
    height: float,
    text: str,
    *,
    fontsize: float = 10.8,
    wrap: int = 22,
) -> dict[str, tuple[float, float]]:
    """Add a diamond decision block."""
    x, y = center
    vertices = [
        (x, y + height / 2),
        (x + width / 2, y),
        (x, y - height / 2),
        (x - width / 2, y),
    ]
    patch = Polygon(
        vertices,
        closed=True,
        facecolor=DECISION_FILL,
        edgecolor=LINE,
        linewidth=LINE_WIDTH,
        zorder=3,
    )
    ax.add_patch(patch)
    ax.text(
        x,
        y,
        textwrap.fill(text, width=wrap),
        ha="center",
        va="center",
        fontsize=fontsize,
        color=TEXT,
        linespacing=1.00,
        fontweight="medium",
        zorder=4,
    )
    return _anchors(x, y, width, height)


def add_note(
    ax, center: tuple[float, float], width: float, height: float, text: str
) -> None:
    """Add a compact explanatory note."""
    x, y = center
    patch = FancyBboxPatch(
        (x - width / 2, y - height / 2),
        width,
        height,
        boxstyle="round,pad=0.04,rounding_size=0.06",
        facecolor=NOTE_FILL,
        edgecolor="#94A3B8",
        linewidth=1.2,
        linestyle="--",
        zorder=3,
    )
    ax.add_patch(patch)
    ax.text(
        x,
        y,
        textwrap.fill(text, width=34),
        ha="center",
        va="center",
        fontsize=9.6,
        color="#475569",
        linespacing=1.12,
        zorder=4,
    )


def add_arrow(
    ax, start, end, *, linewidth: float = LINE_WIDTH, color: str = LINE
) -> None:
    """Draw a straight arrow between two points."""
    ax.add_patch(
        FancyArrowPatch(
            start,
            end,
            arrowstyle="-|>",
            mutation_scale=ARROW_SCALE,
            linewidth=linewidth,
            color=color,
            shrinkA=0,
            shrinkB=0,
            zorder=2,
        )
    )


def add_polyline(
    ax,
    points: Sequence[tuple[float, float]],
    *,
    arrow: bool = False,
    linewidth: float = LINE_WIDTH,
    color: str = LINE,
) -> None:
    """Draw an orthogonal/polyline connector, optionally ending in an arrow."""
    if len(points) < 2:
        raise ValueError("At least two points are required")

    if arrow and len(points) == 2:
        add_arrow(ax, points[0], points[1], linewidth=linewidth, color=color)
        return

    segment_pairs = list(zip(points[:-1], points[1:]))
    line_pairs = segment_pairs[:-1] if arrow else segment_pairs

    for p0, p1 in line_pairs:
        ax.plot(
            [p0[0], p1[0]],
            [p0[1], p1[1]],
            color=color,
            linewidth=linewidth,
            solid_capstyle="round",
            solid_joinstyle="round",
            zorder=1,
        )

    if arrow:
        add_arrow(ax, points[-2], points[-1], linewidth=linewidth, color=color)


def add_branch_label(
    ax,
    point: tuple[float, float],
    text: str,
    *,
    is_yes: bool,
    align: str = "center",
) -> None:
    """Add a compact Yes/No branch label."""
    ax.text(
        point[0],
        point[1],
        text,
        ha=align,
        va="center",
        fontsize=10.2,
        fontweight="bold",
        color=YES_COLOR if is_yes else NO_COLOR,
        bbox={"facecolor": "white", "edgecolor": "none", "pad": 0.7},
        zorder=6,
    )


def create_flowchart(output_path: str | Path) -> Path:
    """Create the PR #33446 iteration-scheme flowchart."""
    output_path = Path(output_path)

    fig, ax = plt.subplots(figsize=(13.6, 16.8))
    ax.set_xlim(-8.7, 8.7)
    ax.set_ylim(0.15, 17.15)
    ax.set_aspect("equal")
    ax.axis("off")

    # ------------------------------------------------------------------
    # Main central flow
    # ------------------------------------------------------------------
    start = add_terminator(ax, (0.0, 16.55), 2.1, 0.62, "Start")
    initialize = add_process(
        ax,
        (0.0, 15.48),
        5.85,
        0.98,
        "Divide domain into blocks and set boundary and initial conditions",
        fontsize=10.7,
        wrap=38,
    )

    solver_merge = (0.0, 14.42)
    flow_solve = add_process(
        ax,
        (0.0, 13.57),
        5.85,
        1.08,
        "Solve the main flow variables in the current block (with equation and update relaxation)",
        fontsize=10.7,
        wrap=39,
    )

    power = add_decision(ax, (0.0, 12.12), 3.0, 1.00, "Power?", fontsize=12.0)
    enthalpy_temperature = add_process(
        ax,
        (0.0, 10.88),
        5.45,
        0.98,
        "Calculate enthalpy and relax the temperature update",
        fill=THERMAL_FILL,
        fontsize=10.7,
        wrap=34,
    )

    property_merge = (0.0, 10.08)
    properties = add_process(
        ax,
        (0.0, 9.34),
        5.45,
        0.92,
        "Update density and viscosity in the block",
        fill=THERMAL_FILL,
        fontsize=10.7,
        wrap=34,
    )

    subcycle = add_decision(
        ax,
        (0.0, 7.92),
        4.45,
        1.26,
        "More enthalpy subcycles?",
        fontsize=10.6,
        wrap=22,
    )
    temperature_loop = add_decision(
        ax,
        (0.0, 6.20),
        4.25,
        1.16,
        "Temperature converged?",
        fontsize=10.8,
        wrap=22,
    )
    last_block = add_decision(
        ax,
        (0.0, 4.52),
        3.25,
        1.04,
        "Last block?",
        fontsize=11.2,
    )
    pressure_loop = add_decision(
        ax,
        (0.0, 2.88),
        4.10,
        1.14,
        "Pressure converged?",
        fontsize=10.8,
        wrap=20,
    )
    returned = add_process(
        ax,
        (0.0, 1.32),
        4.35,
        0.76,
        "RETURN",
        fill=ACTION_FILL,
        fontsize=12.5,
    )

    # ------------------------------------------------------------------
    # Side actions
    # ------------------------------------------------------------------
    next_block = add_process(
        ax,
        (-4.90, 4.52),
        2.75,
        1.12,
        "Move to next block downstream",
        fill=ACTION_FILL,
        fontsize=10.0,
        wrap=16,
    )
    next_timestep = add_process(
        ax,
        (-6.85, 1.32),
        2.55,
        1.00,
        "Go to next timestep",
        fill=ACTION_FILL,
        fontsize=10.0,
        wrap=15,
    )
    first_block = add_process(
        ax,
        (6.65, 2.88),
        2.75,
        1.12,
        "Move to first block upstream",
        fill=ACTION_FILL,
        fontsize=10.0,
        wrap=16,
    )

    # ------------------------------------------------------------------
    # Main downward path
    # ------------------------------------------------------------------
    add_arrow(ax, start["bottom"], initialize["top"])
    add_polyline(ax, [initialize["bottom"], solver_merge])
    add_arrow(ax, solver_merge, flow_solve["top"])
    add_arrow(ax, flow_solve["bottom"], power["top"])

    # Power branch: both alternatives merge before the property update.
    add_arrow(ax, power["bottom"], enthalpy_temperature["top"])
    add_branch_label(ax, (0.50, 11.47), "Yes", is_yes=True)
    add_polyline(ax, [enthalpy_temperature["bottom"], property_merge])

    power_bypass_x = 3.25
    add_polyline(
        ax,
        [
            power["right"],
            (power_bypass_x, power["right"][1]),
            (power_bypass_x, property_merge[1]),
            property_merge,
        ],
    )
    add_branch_label(ax, (2.12, 12.12), "No", is_yes=False)
    add_arrow(ax, property_merge, properties["top"])

    add_arrow(ax, properties["bottom"], subcycle["top"])

    # Enthalpy subcycle loop. It remains inside the current block and does not
    # repeat the main flow solve until the subcycle group is finished.
    subcycle_rail_x = 4.55
    subcycle_entry_y = power["top"][1] + 0.22
    add_polyline(
        ax,
        [
            subcycle["right"],
            (subcycle_rail_x, subcycle["right"][1]),
            (subcycle_rail_x, subcycle_entry_y),
            (0.0, subcycle_entry_y),
        ],
        arrow=True,
    )
    add_branch_label(ax, (2.62, 7.92), "Yes", is_yes=True)
    add_arrow(ax, subcycle["bottom"], temperature_loop["top"])
    add_branch_label(ax, (0.52, 7.10), "No", is_yes=False)

    add_arrow(ax, temperature_loop["bottom"], last_block["top"])
    add_branch_label(ax, (0.53, 5.38), "Yes", is_yes=True)

    add_arrow(ax, last_block["bottom"], pressure_loop["top"])
    add_branch_label(ax, (0.50, 3.70), "Yes", is_yes=True)

    add_arrow(ax, pressure_loop["bottom"], returned["top"])
    add_branch_label(ax, (0.53, 2.04), "Yes", is_yes=True)

    # ------------------------------------------------------------------
    # Left-side returns: temperature iteration, downstream block sweep,
    # and next timestep share one clean return rail.
    # ------------------------------------------------------------------
    block_rail_x = -4.90
    timestep_rail_x = -6.85
    left_return_y = solver_merge[1]

    # Temperature loop is not complete: repeat the flow solve in this block.
    add_polyline(
        ax, [temperature_loop["left"], (block_rail_x, temperature_loop["left"][1])]
    )
    add_branch_label(ax, (-2.82, 6.20), "No", is_yes=False)

    # Continue the downstream block sweep.
    add_arrow(ax, last_block["left"], next_block["right"])
    add_branch_label(ax, (-2.18, 4.52), "No", is_yes=False)
    add_polyline(ax, [next_block["top"], (block_rail_x, left_return_y)])

    # At the next timestep the outer return bends at the top. The block rail
    # joins its horizontal segment vertically from below, as requested.
    add_arrow(ax, returned["left"], next_timestep["right"])
    add_polyline(
        ax,
        [next_timestep["top"], (timestep_rail_x, left_return_y), solver_merge],
    )

    # ------------------------------------------------------------------
    # Right-side pressure return. It is kept outside the subcycle loop.
    # ------------------------------------------------------------------
    pressure_rail_x = 6.65
    add_arrow(ax, pressure_loop["right"], first_block["left"])
    add_branch_label(ax, (2.72, 2.88), "No", is_yes=False)
    add_polyline(
        ax,
        [
            first_block["top"],
            (pressure_rail_x, left_return_y),
            solver_merge,
        ],
        arrow=True,
    )

    fig.savefig(
        output_path, dpi=300, bbox_inches="tight", pad_inches=0.18, facecolor="white"
    )
    plt.close(fig)
    return output_path


if __name__ == "__main__":
    destination = Path(__file__).with_suffix(".png")
    create_flowchart(destination)
    print(f"Saved flowchart to: {destination}")
