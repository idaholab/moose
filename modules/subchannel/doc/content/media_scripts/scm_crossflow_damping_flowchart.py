"""Create the flowchart of one damped monolithic solve of the SCM monolithic algorithm.

The diagram follows SubChannel1PhaseProblem::implicitPetscSolve with
crossflow_pseudo_time_damping on:

1. The crossflow residual of the current iterate sets the pseudo-time step.
2. The damping and its right-hand-side compensation are added, then the equation relaxation.
3. The monolithic system is solved with FGMRES; a solve that does not converge within
   monolithic_retry_maxit iterations is retried with an increment of damping added to the relaxed
   system, doubling the block factor gamma, up to eight times.
4. The post-solve relaxation is applied to the converged solution.

Run
---
python scm_crossflow_damping_flowchart.py
"""

from __future__ import annotations

from pathlib import Path
import sys

import matplotlib.pyplot as plt

# The drawing helpers and style of the SCM iteration-scheme flowchart in this directory
sys.path.insert(0, str(Path(__file__).resolve().parent))
from scm_solver_flowchart import (  # noqa: E402
    ACTION_FILL,
    TEXT,
    add_arrow,
    add_branch_label,
    add_decision,
    add_note,
    add_polyline,
    add_process,
    add_terminator,
)


def create_flowchart(output_path: str | Path) -> Path:
    """Create the flowchart of one damped monolithic solve."""
    output_path = Path(output_path)

    fig, ax = plt.subplots(figsize=(12.4, 16.0))
    ax.set_xlim(-7.4, 7.4)
    ax.set_ylim(-0.9, 18.5)
    ax.set_aspect("equal")
    ax.axis("off")

    x = -1.4
    w = 6.6

    start = add_terminator(
        ax,
        (x, 17.75),
        7.4,
        1.25,
        "Monolithic solve of the block\nwith pseudo-time step damping",
    )
    residual = add_process(ax, (x, 16.0), w, 1.45, "")
    ax.text(
        x,
        16.0,
        "Crossflow residual $\\|r_w\\|$ of the current iterate;\n"
        "reference $\\|r_w^0\\|$: the largest residual\n"
        "of the SCM solve so far",
        ha="center",
        va="center",
        fontsize=11.3,
        color=TEXT,
        linespacing=1.5,
        zorder=4,
    )
    step = add_process(ax, (x, 13.7), w, 2.05, "")
    # A tall formula does not get room from linespacing, so each line is placed separately
    for dy, line in [
        (0.68, "Pseudo-time step"),
        (
            0.0,
            "$\\Delta\\tau = \\max\\left(\\Delta\\tau_0,\\ \\Delta\\tau_0\\,"
            "\\dfrac{\\|r_w^0\\|}{\\|r_w\\|}\\,\\dfrac{1}{\\gamma}\\right)$",
        ),
        (-0.68, "with the block factor $\\gamma$, raised by the retries"),
    ]:
        ax.text(
            x,
            13.7 + dy,
            line,
            ha="center",
            va="center",
            fontsize=11.3,
            color=TEXT,
            zorder=4,
        )
    # add_process wraps its text, which would split the math expressions, so this label is drawn
    # separately with explicit line breaks
    damping = add_process(ax, (x, 10.95), w, 1.9, "", fill=ACTION_FILL)
    ax.text(
        x,
        10.95,
        "Add the damping $\\mathrm{added\\_K}_{ij} = m_{ij}\\,/\\,\\Delta\\tau$, "
        "$m_{ij} = L_{ij}\\,S_{ij}\\,\\rho^*_{ij}$,\n"
        "to the crossflow diagonal and\n"
        "$\\mathrm{added\\_K}_{ij}\\,W_{ij}^{it}$ to the right-hand side,\n"
        "with $W_{ij}^{it}$ the current iterate",
        ha="center",
        va="center",
        fontsize=11.3,
        color=TEXT,
        linespacing=1.5,
        zorder=4,
    )
    relax = add_process(ax, (x, 9.2), w, 0.95, "Apply the equation relaxation")
    solve = add_process(
        ax,
        (x, 7.7),
        w,
        1.15,
        "Solve the monolithic system with FGMRES, at most monolithic_retry_maxit iterations",
    )
    converged = add_decision(ax, (x, 5.65), 3.9, 2.1, "Converged?")
    post = add_process(ax, (x, 3.35), w, 0.95, "Apply the post-solve relaxation")
    end = add_terminator(ax, (x, 1.8), 4.6, 0.95, "Updated block solution")

    last = add_decision(ax, (5.0, 5.65), 3.6, 2.1, "Eighth retry?")
    retry = add_process(
        ax,
        (5.0, 9.15),
        4.6,
        2.75,
        "",
        fill=ACTION_FILL,
        wrap=24,
    )
    # Drawn separately, like the damping box, so the math is not wrapped across lines
    for dy, line in [
        (0.95, "Increase the inverse step"),
        (
            0.45,
            "by $\\delta_r = \\max(1/\\Delta\\tau^{\\ell,r-1},\\ 0.05/\\Delta\\tau_0)$,",
        ),
        (-0.05, "add $m_{ij}\\,\\delta_r$ to the diagonal"),
        (-0.5, "and $m_{ij}\\,\\delta_r\\,W_{ij}^{it}$ to the right-hand side,"),
        (-0.95, "double $\\gamma$, and retry"),
    ]:
        ax.text(
            5.0,
            retry["center"][1] + dy,
            line,
            ha="center",
            va="center",
            fontsize=10.6,
            color=TEXT,
            zorder=4,
        )
    error = add_terminator(ax, (5.0, 3.15), 3.6, 0.95, "Error", fontsize=12.0)

    for a, b in [
        (start["bottom"], residual["top"]),
        (residual["bottom"], step["top"]),
        (step["bottom"], damping["top"]),
        (damping["bottom"], relax["top"]),
        (relax["bottom"], solve["top"]),
        (solve["bottom"], converged["top"]),
        (converged["bottom"], post["top"]),
        (post["bottom"], end["top"]),
    ]:
        add_arrow(ax, a, b)
    add_branch_label(ax, (x + 0.35, 4.25), "Yes", is_yes=True, align="left")

    add_arrow(ax, converged["right"], last["left"])
    add_branch_label(ax, (2.0, 6.0), "No", is_yes=False)

    add_arrow(ax, last["top"], retry["bottom"])
    add_branch_label(ax, (5.35, 7.15), "No", is_yes=False, align="left")
    add_polyline(
        ax,
        [(retry["left"][0], 8.1), (2.45, 8.1), (2.45, 7.7), solve["right"]],
        arrow=True,
    )

    add_arrow(ax, last["bottom"], error["top"])
    add_branch_label(ax, (5.35, 4.2), "Yes", is_yes=True, align="left")

    add_note(
        ax,
        (x, 0.1),
        w,
        1.45,
        "$\\gamma$ and the reference residual reset at the start of every SCM solve. With lu "
        "the solve converges in one iteration and is not retried.",
    )

    fig.savefig(
        output_path, dpi=300, bbox_inches="tight", pad_inches=0.18, facecolor="white"
    )
    plt.close(fig)
    return output_path


if __name__ == "__main__":
    destination = Path(__file__).with_suffix(".png")
    print(create_flowchart(destination))
