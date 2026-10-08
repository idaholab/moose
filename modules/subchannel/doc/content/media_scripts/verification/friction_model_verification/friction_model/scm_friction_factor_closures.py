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
subchannel-opt -i tri_wire_flow_split.i
subchannel-opt -i tri_wire_flow_split.i SCMClosures/Chen/friction_model=Pacio \
    Outputs/file_base=tri_wire_flow_split_pacio_out
subchannel-opt -i XX09_SS17.i
subchannel-opt -i XX09_SS17.i SCMClosures/Chen/friction_model=Pacio \
    SCMClosures/Chen_Todreas/mixing_model=Pacio Outputs/file_base=XX09_SS17_pacio_out
python dassh_XX09_SS17.py

Five figures are written next to this script:

1. scm_friction_quad_bare.png: MATRA and Chen-Todreas, bare pins in a square lattice.
2. scm_friction_tri_bare.png: Upgraded Chen-Todreas in SCM and DASSH, bare pins in a triangular
   lattice, next to the relative difference between the SCM and DASSH friction factors.
3. scm_friction_tri_wire.png: Upgraded and Pacio Chen-Todreas in SCM and Upgraded Chen-Todreas in
   DASSH, wire-wrapped pins in a triangular lattice, next to the relative difference between the
   SCM and DASSH Upgraded Chen-Todreas friction factors.
4. scm_XX09_SS17_mdot.png, scm_XX09_SS17_T.png: subchannel mass flow rate and temperature along the
   TTC traverse of EBR-II XX09 for SHRT-17 from SCM and DASSH.

The rows of the friction factor, developed flow split, and TTC temperature tables in the verification
page are printed.
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


def interpolate_ff(csv, channel, Re):
    """Friction factor of a subchannel at the local Reynolds numbers Re, interpolated in log-log"""
    data = np.genfromtxt(DATA / csv, delimiter=",", names=True)
    # Skip the initial condition and Re < 1, as in plot_channels
    keep = data[f"Re_{channel}"] >= 1.0
    return np.exp(
        np.interp(
            np.log(Re),
            np.log(data[f"Re_{channel}"][keep]),
            np.log(data[f"ff_{channel}"][keep]),
        )
    )


def plot_dassh_comparison(title, scm_csvs, dassh_csv, name):
    """Plot the SCM and DASSH friction factors next to the relative difference between the SCM and
    DASSH UCTD friction factors. scm_csvs maps the SCM CSV files to their color and label, with the
    SCM UCTD CSV first."""
    fig, (ax, ax_err) = plt.subplots(1, 2, figsize=(14.0, 5.0))
    ax.set_xscale("log")
    ax.set_yscale("log")
    ax.set_xlim(1.0, 1.0e6)
    ax.set_xlabel("Subchannel Reynolds number, $Re$")
    ax.set_ylabel("Friction factor, $f$")
    ax.set_title(title)
    ax.grid(True, which="both", color="0.85", linewidth=0.5)
    for csv, (color, label) in scm_csvs.items():
        plot_channels(ax, csv, color, label)
    plot_channels(ax, dassh_csv, "green", "DASSH UCTD")
    ax.legend(frameon=False)
    # Relative difference of SCM UCTD from DASSH UCTD at the local Reynolds numbers of the SCM sweep.
    # The DASSH curves are sampled more finely than the SCM sweep, so they are the ones interpolated.
    scm = np.genfromtxt(DATA / next(iter(scm_csvs)), delimiter=",", names=True)
    dassh = np.genfromtxt(DATA / dassh_csv, delimiter=",", names=True)
    for channel in CHANNELS:
        Re = scm[f"Re_{channel}"]
        # Skip Re < 1, as in plot_channels, and Re outside the DASSH curve, where np.interp
        # extrapolates
        keep = (Re >= max(1.0, dassh[f"Re_{channel}"].min())) & (
            Re <= dassh[f"Re_{channel}"].max()
        )
        ff_dassh = interpolate_ff(dassh_csv, channel, Re[keep])
        ax_err.plot(
            Re[keep],
            100.0 * (scm[f"ff_{channel}"][keep] / ff_dassh - 1.0),
            color="black",
            linestyle=LINESTYLES[channel],
            label=LABELS[channel],
        )
    ax_err.set_xscale("log")
    ax_err.set_xlim(1.0, 1.0e6)
    ax_err.set_xlabel("Subchannel Reynolds number, $Re$")
    ax_err.set_ylabel("$(f_{SCM} - f_{DASSH}) / f_{DASSH}$ [%]")
    ax_err.set_title("SCM UCTD relative to DASSH UCTD")
    ax_err.grid(True, which="both", color="0.85", linewidth=0.5)
    save(fig, ax_err, name)


plot_dassh_comparison(
    "Triangular lattice, bare pins",
    {"tri_bare_out.csv": ("black", "UCTD")},
    "dassh_tri_bare_out.csv",
    "scm_friction_tri_bare.png",
)
plot_dassh_comparison(
    "Triangular lattice, wire-wrapped pins",
    {
        "tri_wire_out.csv": ("black", "UCTD"),
        "tri_wire_pacio_out.csv": ("red", "PCTD"),
    },
    "dassh_tri_wire_out.csv",
    "scm_friction_tri_wire.png",
)
dassh_wire = np.genfromtxt(DATA / "dassh_tri_wire_out.csv", delimiter=",", names=True)


# Rows of the friction factor comparison tables in the verification page
TABLE_RE = np.array([1.0e2, 3.0e3, 1.0e4])
for csvs in (
    (("tri_bare_out.csv", "SCM UCTD"), ("dassh_tri_bare_out.csv", "DASSH UCTD")),
    (
        ("tri_wire_out.csv", "SCM UCTD"),
        ("tri_wire_pacio_out.csv", "SCM PCTD"),
        ("dassh_tri_wire_out.csv", "DASSH UCTD"),
    ),
):
    for channel in CHANNELS:
        for csv, name in csvs:
            ff = interpolate_ff(csv, channel, TABLE_RE)
            print(
                f"| {LABELS[channel]} | {name} | "
                + " | ".join(f"{f:.4f}" for f in ff)
                + " |"
            )

# Rows of the developed flow split table in the verification page. DASSH reports the subchannel and
# bundle Reynolds numbers, Re_i = X_i Re_b Dh_i / Dh_b, so its flow split is X_i = (Re_i / Re_b)
# (Dh_b / Dh_i), with the hydraulic diameters of SCM, which defines the same subchannels as DASSH.
scm_split = {
    name: np.genfromtxt(DATA / csv, delimiter=",", names=True)
    for csv, name in (
        ("tri_wire_flow_split_out.csv", "SCM UCTD"),
        ("tri_wire_flow_split_pacio_out.csv", "SCM PCTD"),
    )
}
split = scm_split["SCM UCTD"]
dassh_split = [
    np.interp(
        np.log(split["Re_bulk"]),
        np.log(dassh_wire["Re_bundle"]),
        dassh_wire[f"Re_{channel}"]
        / dassh_wire["Re_bundle"]
        * split["Dh_bulk"]
        / split[f"Dh_{channel}"],
    )
    for channel in CHANNELS
]
for name, data in scm_split.items():
    print(
        f"| {name} | "
        + " | ".join(f"{data[f'X_{channel}']:.4f}" for channel in CHANNELS)
        + " |"
    )
print("| DASSH UCTD | " + " | ".join(f"{X:.4f}" for X in dassh_split) + " |")

# EBR-II XX09 SHRT-17 steady state along the TTC traverse at the TTC height
TTC = np.arange(27, 36)
scm_uctd = np.genfromtxt(DATA / "XX09_SS17_out.csv", delimiter=",", names=True)
scm_pctd = np.genfromtxt(DATA / "XX09_SS17_pacio_out.csv", delimiter=",", names=True)
dassh = np.genfromtxt(
    DATA / "dassh_XX09_SS17_out.csv",
    delimiter=",",
    names=True,
    dtype=None,
    encoding=None,
)
# DASSH orients the hexagonal lattice 30 degrees apart from SCM, and both codes sweep the wire-wrap
# flow counterclockwise, so rotate (not reflect) the DASSH subchannel
# positions and take the DASSH subchannel closest to each TTC subchannel of SCM
angle = np.pi / 6
rotation = np.array([[np.cos(angle), -np.sin(angle)], [np.sin(angle), np.cos(angle)]])
dassh_xy = np.column_stack((dassh["x"], dassh["y"])) @ rotation.T
dassh_ttc = [
    np.argmin(np.hypot(*(dassh_xy - [scm_uctd[f"x{n}"][-1], scm_uctd[f"y{n}"][-1]]).T))
    for n in TTC
]


def plot_ttc(scm_name, dassh_name, ylabel, name, dassh_ctd=None, experiment=None):
    fig, ax = plt.subplots(figsize=(7.0, 5.0))
    ax.plot(
        TTC,
        [scm_uctd[f"{scm_name}{n}"][-1] for n in TTC],
        "o-",
        color="black",
        label="SCM UCTD",
    )
    ax.plot(
        TTC,
        [scm_pctd[f"{scm_name}{n}"][-1] for n in TTC],
        "s--",
        color="red",
        label="SCM PCTD",
    )
    ax.plot(TTC, dassh[dassh_name][dassh_ttc], "^:", color="green", label="DASSH UCTD")
    if dassh_ctd is not None:
        ax.plot(
            TTC,
            dassh_ctd,
            "v-.",
            color="limegreen",
            label="DASSH CTD, published",
        )
    if experiment is not None:
        ax.plot(
            TTC,
            experiment,
            "D",
            color="blue",
            label="Experiment",
        )
    ax.set_xlabel("TTC thermocouple")
    ax.set_ylabel(ylabel)
    ax.set_title("EBR-II XX09, SHRT-17 steady state, $z = 0.322$ m")
    ax.grid(True, color="0.85", linewidth=0.5)
    save(fig, ax, name)


plot_ttc("mdot", "mdot", "Subchannel mass flow rate [kg/s]", "scm_XX09_SS17_mdot.png")
# Published DASSH temperatures (DASSH Example-3, CTD friction and flow split) of the EBR-II
# validation, in Celsius, one row per TTC thermocouple 27 to 35
dassh_published = np.genfromtxt(
    DATA / "../../../validation/EBR-II/TTC_DASSH.csv", delimiter=",", skip_header=1
)
# Measured TTC temperatures of the EBR-II validation, in Celsius
experiment = np.genfromtxt(
    DATA / "../../../validation/EBR-II/TTC_EXP.csv", delimiter=",", skip_header=1
)
plot_ttc(
    "TTC",
    "T",
    "Subchannel temperature [K]",
    "scm_XX09_SS17_T.png",
    dassh_ctd=dassh_published[:, 1] + 273.15,
    experiment=experiment[:, 1] + 273.15,
)

# Rows of the TTC temperature table in the verification page, with the error against the experiment
ttc_T = {
    "SCM UCTD": np.array([scm_uctd[f"TTC{n}"][-1] for n in TTC]),
    "SCM PCTD": np.array([scm_pctd[f"TTC{n}"][-1] for n in TTC]),
    "DASSH UCTD": dassh["T"][dassh_ttc],
    "DASSH CTD, published": dassh_published[:, 1] + 273.15,
    "Experiment": experiment[:, 1] + 273.15,
}
for i, n in enumerate(TTC):
    print(f"| TTC-{n} | " + " | ".join(f"{T[i]:.1f}" for T in ttc_T.values()) + " |")
error = {
    name: T - ttc_T["Experiment"] for name, T in ttc_T.items() if name != "Experiment"
}
print(
    "| Mean error | " + " | ".join(f"{e.mean():.1f}" for e in error.values()) + " | |"
)
print(
    "| Root mean square error | "
    + " | ".join(f"{np.sqrt(np.mean(e**2)):.1f}" for e in error.values())
    + " | |"
)
