"""Plot the axial friction factor versus the local subchannel Reynolds number for the SCM friction
closures.

The data are the CSV outputs of the SCM inputs in
modules/subchannel/verification/friction_model_verification/friction_model, which sweep the inlet
mass flux and report the friction factor and Reynolds number of one interior, edge, and corner
subchannel. Rerun those inputs to regenerate the data:

subchannel-opt -i quad_bare.i
subchannel-opt -i quad_bare.i SubChannel/friction_closure=Chen Outputs/file_base=quad_bare_chen_out
subchannel-opt -i tri_bare.i
subchannel-opt -i tri_wire.i
subchannel-opt -i tri_wire.i SCMClosures/Chen/friction_model=Pacio \
    Outputs/file_base=tri_wire_pacio_out

Three figures are written next to this script:

1. scm_friction_quad_bare.png: MATRA and Chen-Todreas, bare pins in a square lattice.
2. scm_friction_tri_bare.png: Upgraded Chen-Todreas, bare pins in a triangular lattice.
3. scm_friction_tri_wire.png: Upgraded and Pacio Chen-Todreas, wire-wrapped pins in a triangular
   lattice.
"""

from pathlib import Path

import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt
import numpy as np

SCRIPT_DIR = Path(__file__).resolve().parent
DATA = (
    SCRIPT_DIR
    / "../../../../../../verification/friction_model_verification/friction_model"
)

CHANNELS = ("center", "edge", "corner")
LABELS = {"center": "interior", "edge": "edge", "corner": "corner"}
LINESTYLES = {"center": "-", "edge": "--", "corner": ":"}


def plot_channels(ax, csv, color, name, channels=CHANNELS):
    data = np.genfromtxt(DATA / csv, delimiter=",", names=True)
    for channel in channels:
        Re = data[f"Re_{channel}"]
        # Skip the initial condition and Re < 1, below which the closures hold the friction factor
        keep = Re >= 1.0
        label = (
            f"{name}, {LABELS[channel]}"
            if len(channels) > 1
            else f"{name}, all subchannels"
        )
        ax.plot(
            Re[keep],
            data[f"ff_{channel}"][keep],
            color=color,
            linestyle=LINESTYLES[channel],
            label=label,
        )


def new_axes(title):
    fig, ax = plt.subplots(figsize=(7.0, 5.0))
    ax.set_xscale("log")
    ax.set_yscale("log")
    ax.set_xlim(1.0, 1.0e6)
    ax.set_xlabel("Subchannel Reynolds number, $Re$")
    ax.set_ylabel("Friction factor, $f$")
    ax.set_title(title)
    ax.grid(True, which="both", color="0.85", linewidth=0.5)
    return fig, ax


def save(fig, ax, name):
    ax.legend(frameon=False)
    fig.savefig(SCRIPT_DIR / name, dpi=200, bbox_inches="tight", facecolor="white")
    plt.close(fig)


fig, ax = new_axes("Square lattice, bare pins")
# MATRA does not distinguish between subchannel types, so only the interior subchannel is shown
plot_channels(ax, "quad_bare_out.csv", "black", "MATRA", channels=("center",))
plot_channels(ax, "quad_bare_chen_out.csv", "red", "Chen-Todreas")
save(fig, ax, "scm_friction_quad_bare.png")

fig, ax = new_axes("Triangular lattice, bare pins")
plot_channels(ax, "tri_bare_out.csv", "black", "UCTD")
save(fig, ax, "scm_friction_tri_bare.png")

fig, ax = new_axes("Triangular lattice, wire-wrapped pins")
plot_channels(ax, "tri_wire_out.csv", "black", "UCTD")
plot_channels(ax, "tri_wire_pacio_out.csv", "red", "PCTD")
save(fig, ax, "scm_friction_tri_wire.png")
