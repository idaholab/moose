"""Plot the SCM Pacio-Chen-Todreas (PCTD) results against the measured data shown in the PCTD paper,
Pacio et al. (2022), Nuclear Engineering and Design 388, 111607.

The data are the CSV outputs of the SCM inputs in
modules/subchannel/verification/friction_model_verification/friction_model, which sweep the inlet
mass flux of the wire-wrapped assemblies of the paper. Rerun those inputs to regenerate the data:

subchannel-opt -i pacio_liang.i Outputs/file_base=pacio_liang_pctd_out
subchannel-opt -i pacio_kennedy.i Outputs/file_base=pacio_kennedy_pctd_out

Two figures are written next to this script:

1. scm_pacio_liang.png: flow split of the interior, edge, and corner subchannels of the 37-pin
   assembly of Liang et al. (2020) versus the bulk Reynolds number, Fig. 7 of the paper.
2. scm_pacio_kennedy.png: bulk friction factor of the 127-pin assembly of Kennedy et al. (2015)
   versus the bulk Reynolds number, Fig. 6 of the paper.

The measured points and the PCTD curves of the paper are read from its figures, so they are accurate
to about the size of the markers, 0.01 in the flow split and 2% in the friction factor.
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

# Measured flow splits of Liang et al. (2020), read from Fig. 7 of the paper: bulk Reynolds number
# and flow split of the interior, edge, and corner subchannels
LIANG = {
    "interior": (
        [1289, 2668, 3473, 4506, 5575, 6171, 7514, 9414, 18695],
        [0.716, 0.731, 0.791, 0.812, 0.833, 0.859, 0.842, 0.858, 0.840],
    ),
    "edge": (
        [1570, 2269, 3078, 4097, 6190, 6576, 7781, 8920, 18814],
        [1.263, 1.216, 1.222, 1.231, 1.201, 1.198, 1.189, 1.188, 1.172],
    ),
    "corner": (
        [1249, 2233, 3249, 4283, 5417, 6451, 7586, 8478, 18636],
        [1.065, 1.136, 1.127, 1.140, 1.156, 1.165, 1.161, 1.171, 1.165],
    ),
}

# Measured bulk friction factors of Kennedy et al. (2015), read from Fig. 6 of the paper: bulk
# Reynolds number and bulk friction factor
KENNEDY = (
    [4194, 8546, 10000, 13109, 16398, 19973, 22906, 26007, 29137, 32209],
    [
        0.0511,
        0.0352,
        0.0331,
        0.0304,
        0.0288,
        0.0275,
        0.0267,
        0.0261,
        0.0253,
        0.0248,
    ],
)

# PCTD curves of Pacio et al. (2022), read from Figs. 7 and 6 of the paper by following the line of
# each curve: bulk Reynolds number and flow split of the interior, edge, and corner subchannels
# between the laminar and turbulent boundaries, 700 < Re_b < 10^4, where the flow split changes, and
# bulk Reynolds number and bulk friction factor
PAPER_LIANG = {
    "interior": (
        [
            710,
            852,
            1022,
            1226,
            1472,
            1766,
            2118,
            2542,
            3050,
            3659,
            4391,
            5268,
            6321,
            7584,
            9100,
        ],
        [
            0.741,
            0.788,
            0.805,
            0.819,
            0.829,
            0.838,
            0.846,
            0.852,
            0.857,
            0.862,
            0.866,
            0.869,
            0.871,
            0.875,
            0.877,
        ],
    ),
    "edge": (
        [
            710,
            852,
            1022,
            1226,
            1472,
            1766,
            2118,
            2542,
            3050,
            3659,
            4391,
            5268,
            6321,
            7584,
            9100,
        ],
        [
            1.346,
            1.276,
            1.25,
            1.23,
            1.213,
            1.199,
            1.187,
            1.177,
            1.168,
            1.161,
            1.154,
            1.149,
            1.143,
            1.138,
            1.134,
        ],
    ),
    "corner": (
        [
            710,
            852,
            1022,
            1226,
            1472,
            1766,
            2118,
            2542,
            3050,
            3659,
            4391,
            5268,
            6321,
            7584,
            9100,
        ],
        [
            0.627,
            0.774,
            0.827,
            0.868,
            0.903,
            0.934,
            0.961,
            0.984,
            1.005,
            1.024,
            1.042,
            1.058,
            1.072,
            1.085,
            1.097,
        ],
    ),
}
PAPER_KENNEDY = (
    [
        520,
        711,
        973,
        1332,
        1822,
        2493,
        3411,
        4667,
        6386,
        8737,
        11954,
        16356,
        22378,
        30617,
        41891,
    ],
    [
        0.1671,
        0.13,
        0.1082,
        0.0867,
        0.0705,
        0.0586,
        0.0496,
        0.0427,
        0.037,
        0.0325,
        0.0303,
        0.0287,
        0.0273,
        0.0256,
        0.0242,
    ],
)

COLORS = {"interior": "black", "edge": "red", "corner": "blue"}
MARKERS = {"interior": "s", "edge": "o", "corner": "^"}


def load(name, n_rings):
    """SCM results of an assembly versus the bulk Reynolds number: the flow split of each type of
    subchannel, lumped over all subchannels of that type as in the PCTD model, and the bulk friction
    factor"""
    data = np.genfromtxt(DATA / name, delimiter=",", names=True)
    n = n_rings - 1
    n_center, n_edge, n_corner = 6 * n * n, 6 * n, 6
    n_channels = n_center + n_edge + n_corner
    # Skip the initial condition
    data = data[data["Re_bulk"] > 0.0]
    mdot = np.array([data[f"mdot_{i}"] for i in range(n_channels)])
    S = np.array([data[f"S_{i}"] for i in range(n_channels)])
    # The interior subchannels come first. The edge and corner subchannels of the outermost ring
    # are interleaved, and the corner subchannels have the smallest flow areas.
    outer = np.arange(n_center, n_channels)
    corner = outer[np.argsort(S[outer, 0])[:n_corner]]
    types = {
        "interior": np.arange(n_center),
        "edge": np.setdiff1d(outer, corner),
        "corner": corner,
    }
    X = {
        name: mdot[idx].sum(axis=0) / S[idx].sum(axis=0) / data["mass_flux"]
        for name, idx in types.items()
    }
    return data["Re_bulk"], X, data["f_bulk"]


Re, X, _ = load("pacio_liang_pctd_out.csv", n_rings=4)
fig, ax = plt.subplots(figsize=(7.0, 5.0))
for channel in COLORS:
    ax.plot(Re, X[channel], color=COLORS[channel], label=f"SCM PCTD, {channel}")
    ax.plot(
        *PAPER_LIANG[channel],
        color=COLORS[channel],
        linestyle="--",
        label=f"PCTD paper, {channel}",
    )
    ax.plot(
        *LIANG[channel],
        linestyle="none",
        marker=MARKERS[channel],
        markerfacecolor="none",
        color=COLORS[channel],
        label=f"Liang et al., {channel}",
    )
ax.set_xscale("log")
ax.set_xlim(400.0, 3.0e4)
ax.set_xlabel("Bulk Reynolds number, $Re_b$")
ax.set_ylabel("Flow split")
ax.set_title("37-pin wire-wrapped assembly of Liang et al.")
ax.grid(True, which="both", color="0.85", linewidth=0.5)
ax.legend(frameon=False, ncol=3, fontsize=7)
fig.savefig(
    SCRIPT_DIR / "scm_pacio_liang.png", dpi=200, bbox_inches="tight", facecolor="white"
)
plt.close(fig)

Re, _, f_bulk = load("pacio_kennedy_pctd_out.csv", n_rings=7)
fig, ax = plt.subplots(figsize=(7.0, 5.0))
ax.plot(Re, f_bulk, color="red", label="SCM PCTD")
ax.plot(*PAPER_KENNEDY, color="red", linestyle="--", label="PCTD paper")
ax.plot(
    *KENNEDY,
    linestyle="none",
    marker="o",
    color="blue",
    label="Kennedy et al.",
)
ax.set_xscale("log")
ax.set_yscale("log")
ax.set_xlim(500.0, 5.0e4)
ax.set_xlabel("Bulk Reynolds number, $Re_b$")
ax.set_ylabel("Bulk friction factor, $f_b$")
ax.set_title("127-pin wire-wrapped assembly of Kennedy et al.")
ax.grid(True, which="both", color="0.85", linewidth=0.5)
ax.legend(frameon=False)
fig.savefig(
    SCRIPT_DIR / "scm_pacio_kennedy.png",
    dpi=200,
    bbox_inches="tight",
    facecolor="white",
)
plt.close(fig)

# Difference between SCM and the measured data at the Reynolds numbers of the measurements, to
# report in the verification page
for channel, (Re_data, X_data) in LIANG.items():
    Re, X, _ = load("pacio_liang_pctd_out.csv", n_rings=4)
    scm = np.interp(np.log(Re_data), np.log(Re), X[channel])
    print(
        f"Liang {channel}: mean {np.mean(scm - X_data):+.3f}, "
        f"rms {np.sqrt(np.mean((scm - X_data) ** 2)):.3f}"
    )
Re, _, f_bulk = load("pacio_kennedy_pctd_out.csv", n_rings=7)
scm = np.exp(np.interp(np.log(KENNEDY[0]), np.log(Re), np.log(f_bulk)))
error = 100.0 * (scm / np.array(KENNEDY[1]) - 1.0)
print(
    f"Kennedy f_b: mean {error.mean():+.1f}%, rms {np.sqrt(np.mean(error**2)):.1f}%, "
    f"range {error.min():+.1f}% to {error.max():+.1f}%"
)


# Difference between SCM and the PCTD curves of the paper, over the transition regime for the flow
# split and over the range of the SCM sweep for the friction factor
Re, X, _ = load("pacio_liang_pctd_out.csv", n_rings=4)
for channel in COLORS:
    Re_paper, X_paper = (np.array(v) for v in PAPER_LIANG[channel])
    scm = np.interp(np.log(Re_paper), np.log(Re), X[channel])
    print(
        f"Liang {channel} vs paper curve: mean {np.mean(scm - X_paper):+.3f}, "
        f"max {np.max(np.abs(scm - X_paper)):.3f}"
    )
Re, _, f_bulk = load("pacio_kennedy_pctd_out.csv", n_rings=7)
Re_paper, f_paper = (np.array(v) for v in PAPER_KENNEDY)
keep = (Re_paper > Re.min()) & (Re_paper < Re.max())
error = 100.0 * (
    np.exp(np.interp(np.log(Re_paper[keep]), np.log(Re), np.log(f_bulk)))
    / f_paper[keep]
    - 1.0
)
print(
    f"Kennedy f_b vs paper curve: mean {error.mean():+.1f}%, "
    f"range {error.min():+.1f}% to {error.max():+.1f}%"
)
