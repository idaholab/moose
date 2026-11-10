"""Plot the SCM Pacio-Chen-Todreas (PCTD) results against the measured data shown in the PCTD paper,
Pacio et al. (2022), Nuclear Engineering and Design 388, 111607.

The data are the CSV outputs of the SCM inputs in
modules/subchannel/verification/friction_model_verification/friction_model, which sweep the inlet
mass flux of the wire-wrapped assemblies of the paper. Rerun those inputs to regenerate the data:

subchannel-opt -i pacio_liang.i Outputs/file_base=pacio_liang_pctd_out
subchannel-opt -i pacio_kennedy.i Outputs/file_base=pacio_kennedy_pctd_out

Two figures are written next to this script:

1. scm_pacio_liang.png: flow split of the interior, edge, and corner subchannels of the 37-pin
   assembly of Liang et al. (2020) versus the bulk Reynolds number, Fig. 7 of the paper. Besides SCM,
   the PCTD curve of the paper, and the measurements, it shows a lumped PCTD calculation with the
   local flow split in the transition regime, solved by this script.
2. scm_pacio_kennedy.png: bulk friction factor of the 127-pin assembly of Kennedy et al. (2015)
   versus the bulk Reynolds number, Fig. 6 of the paper.

The measured points and the PCTD curves of the paper are read from its figures, so they are accurate
to about the size of the markers, 0.01 in the flow split and 2% in the friction factor. The PCTD curves
of the flow split are the complete curves of Fig. 7: the constant laminar level, the transition, and
the constant turbulent level.
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
# between the laminar and turbulent boundaries, 700 < Re_b < 10^4, where the flow split changes
# steeply near the laminar boundary, and bulk Reynolds number and bulk friction factor
PAPER_LIANG = {
    "interior": (
        [
            691,
            704,
            719,
            734,
            749,
            763,
            779,
            794,
            811,
            820,
            846,
            865,
            879,
            893,
            928,
            933,
            954,
            971,
            977,
            1053,
            1059,
            1075,
            1081,
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
            0.712,
            0.736,
            0.754,
            0.762,
            0.767,
            0.771,
            0.775,
            0.778,
            0.781,
            0.783,
            0.788,
            0.791,
            0.794,
            0.797,
            0.793,
            0.794,
            0.797,
            0.799,
            0.8,
            0.81,
            0.81,
            0.812,
            0.813,
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
            691,
            704,
            719,
            733,
            749,
            763,
            780,
            795,
            813,
            828,
            845,
            864,
            879,
            896,
            916,
            934,
            955,
            972,
            1055,
            1057,
            1080,
            1082,
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
            1.393,
            1.357,
            1.329,
            1.317,
            1.308,
            1.302,
            1.296,
            1.291,
            1.287,
            1.283,
            1.279,
            1.275,
            1.272,
            1.269,
            1.265,
            1.262,
            1.259,
            1.257,
            1.246,
            1.245,
            1.24,
            1.24,
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
            692,
            704,
            719,
            733,
            748,
            763,
            779,
            796,
            811,
            829,
            846,
            862,
            880,
            899,
            918,
            934,
            955,
            975,
            994,
            1012,
            1036,
            1055,
            1078,
            1085,
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
            0.541,
            0.611,
            0.67,
            0.693,
            0.71,
            0.724,
            0.735,
            0.745,
            0.754,
            0.763,
            0.77,
            0.777,
            0.784,
            0.79,
            0.797,
            0.802,
            0.809,
            0.814,
            0.82,
            0.824,
            0.829,
            0.834,
            0.842,
            0.845,
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

# Constant flow split of the PCTD curves of Fig. 7 of the paper in the laminar regime, Re_b <= 700,
# and in the turbulent regime, Re_b >= 10^4; the flow split changes only between them. The laminar
# and turbulent boundaries are cbL1 = 700 and cbT1 = 10^4 of Table 5 of the paper.
PAPER_LAMINAR = {"interior": 0.714, "edge": 1.391, "corner": 0.542}
PAPER_TURBULENT = {"interior": 0.878, "edge": 1.133, "corner": 1.100}
RE_LAMINAR, RE_TURBULENT = 700.0, 1.0e4
# Range of the bulk Reynolds number over which the flat parts of the curves are drawn
RE_PLOT = (400.0, 3.0e4)


def paper_liang_curve(channel):
    """Full PCTD curve of the paper for a type of subchannel: the constant laminar level up to the
    laminar boundary, the transition read from the figure, and the constant turbulent level from the
    turbulent boundary"""
    Re, X = PAPER_LIANG[channel]
    return (
        [RE_PLOT[0]] + list(Re) + [RE_TURBULENT, RE_PLOT[1]],
        [PAPER_LAMINAR[channel]] + list(X) + [PAPER_TURBULENT[channel]] * 2,
    )


# Lumped PCTD model of Pacio et al. (2022): one flow split per type of subchannel, interior (1), edge
# (2), and corner (3), from the governing Eqs. (8)-(11) with the constitutive equations of Section 4
# and the parameters of Table 5. In the transition regime the example calculation of the paper,
# Section 6.1.3, evaluates the friction factors of Eq. (35) and the mixing of Eq. (36) with the
# laminar and turbulent flow splits. This calculation evaluates them with the flow split of the
# solution instead, at the local Reynolds number of Eq. (14), as SCM does. It differs from SCM only by
# the lumping of the subchannels of each type.
PCTD_M, PCTD_GAMMA, PCTD_LAMBDA = 0.18, 0.362, 6.7
# Wire sweep a and b, Eq. (1); wire drag cwT1 to cwT4, Eq. (21); laminar scalings cwL1 and cwL2,
# Eqs. (22) and (26); turbulent mixing WmT, Eq. (32); all from Table 5 of the paper
PCTD_SWEEP, PCTD_DRAG, PCTD_CWL1, PCTD_CWL2, PCTD_WMT = (
    (-6.9, 12.0),
    (15.2, -48.0, 148.6, -0.547),
    0.8,
    1.2,
    8.8,
)
# Bare-bundle friction constants k1, k2, k3 of Eq. (29), Table 3 of the paper, for interior, edge,
# and corner, for P/D (or W/D) below 1.1 and from 1.1
PCTD_BARE_L = (
    ((26.00, 888.2, -3334.0), (62.97, 216.9, -190.2)),
    ((26.18, 554.5, -1480.0), (44.40, 256.7, -267.6)),
    ((26.98, 1636.0, -10050.0), (87.26, 38.59, -55.12)),
)
PCTD_BARE_T = (
    ((0.09378, 1.398, -8.664), (0.1458, 0.03632, -0.03333)),
    ((0.09377, 0.8732, -3.341), (0.1430, 0.04199, -0.04428)),
    ((0.1004, 1.625, -11.85), (0.1499, 0.006706, -0.009567)),
)


def pctd_geometry(D, Dw, P, H, W, n):
    """Geometry, friction constants, and mixing coefficients of the lumped PCTD model for pin
    diameter D, wire diameter Dw, pitch P, wire lead H, edge pitch W, and n rings of pins around the
    center pin, Appendix A and Section 4 of the paper"""
    theta = np.arctan(np.pi * (D + Dw) / H)
    # Bare flow areas and wetted perimeters, Eqs. (A.7)-(A.12), with sharp corners
    Ap = np.array(
        [
            np.sqrt(3.0) / 4.0 * P**2 - np.pi / 8.0 * D**2,
            P * (W - D / 2.0) - np.pi / 8.0 * D**2,
            (W - D / 2.0) ** 2 / np.sqrt(3.0) - np.pi / 24.0 * D**2,
        ]
    )
    Pwp = np.array(
        [
            np.pi * D / 2.0,
            P + np.pi * D / 2.0,
            np.pi * D / 6.0 + 2.0 / np.sqrt(3.0) * (W - D / 2.0),
        ]
    )
    # Flow areas and wetted perimeters with the wires, Eqs. (A.13)-(A.18); the wetted perimeters
    # add the wire perimeter pi Dw, which Eqs. (A.16)-(A.18) of the paper omit
    A = Ap - np.array([1.0 / 8.0, 1.0 / 8.0, 1.0 / 24.0]) * np.pi * Dw**2 / np.cos(
        theta
    )
    Pw = Pwp + np.array([1.0 / 2.0, 1.0 / 2.0, 1.0 / 6.0]) * np.pi * Dw / np.cos(theta)
    N = np.array([6 * n * n, 6 * n, 6])
    De = 4.0 * A / Pw
    Deb = 4.0 * (N @ A) / (N @ Pw)
    # Projected wire areas, Eqs. (A.21)-(A.23)
    Ar = np.array([1.0 / 6.0, 1.0 / 4.0, 1.0 / 6.0]) * np.pi * (D + Dw) * Dw
    # Bare-bundle friction constants, Eq. (29), with W/D for the edge and corner subchannels
    ratio = np.array([P / D, W / D, W / D])
    bare = lambda table, i: (
        lambda k: k[0] + k[1] * (ratio[i] - 1.0) + k[2] * (ratio[i] - 1.0) ** 2
    )(table[i][int(ratio[i] >= 1.1)])
    bare_L = np.array([bare(PCTD_BARE_L, i) for i in range(3)])
    bare_T = np.array([bare(PCTD_BARE_T, i) for i in range(3)])
    WsT = PCTD_SWEEP[0] * np.log10(H / D) + PCTD_SWEEP[1]
    WdT = (PCTD_DRAG[0] + PCTD_DRAG[1] * Dw / D + PCTD_DRAG[2] * (Dw / D) ** 2) * (
        H / D
    ) ** PCTD_DRAG[3]
    sweep = WsT * Ar / Ap * np.tan(theta) ** 2
    drag = 3.0 * Ar[0] / Ap[0] * De[0] / H
    # Subchannel friction constants, Eqs. (19), (20), (24), (25), (27), and (28)
    C_T = np.array(
        [
            bare_T[0] * Pwp[0] / Pw[0] + WdT * drag * (De[0] / Dw) ** PCTD_M,
            bare_T[1] * (1.0 + sweep[1]) ** ((3.0 - PCTD_M) / 2.0),
            bare_T[2] * (1.0 + sweep[2]) ** ((3.0 - PCTD_M) / 2.0),
        ]
    )
    C_L = np.array(
        [
            bare_L[0] * Pwp[0] / Pw[0] + PCTD_CWL1 * WdT * drag * De[0] / Dw,
            bare_L[1] * (1.0 + PCTD_CWL2 * sweep[1]),
            bare_L[2] * (1.0 + PCTD_CWL2 * sweep[2]),
        ]
    )
    # Turbulent mixing W'_ij / (rho V_b) of Eq. (32) without the Re_b and flow-split factors, with
    # the contact perimeters of Eqs. (A.24) and (A.25)
    Pi = np.array(
        [6.0 * n * (P - D - Dw / 6.0), (6.0 * (W - D) * 2.0 - Dw / 6.0) / 2.0]
    )
    k_mix = Pi / 2.0 * np.sqrt(Ar[1] / Ap[1]) * np.tan(theta) * PCTD_WMT
    return {
        "De": De,
        "Deb": Deb,
        "NA": N * A,
        "S": N * A / (N @ A),
        "C_L": C_L,
        "C_T": C_T,
        "k_mix": k_mix,
    }


def pctd_flow_split(geometry, Re_b):
    """Lumped PCTD flow split of the interior, edge, and corner subchannels at the bulk Reynolds
    number Re_b, with the local flow split in Eqs. (35) and (36) of the transition regime
    """
    g = geometry
    psi = np.clip(
        np.log(Re_b / RE_LAMINAR) / np.log(RE_TURBULENT / RE_LAMINAR), 0.0, 1.0
    )
    w_L = (1.0 - psi) ** PCTD_GAMMA * (1.0 - psi**PCTD_LAMBDA)
    w_T = psi**PCTD_GAMMA

    def residual(v):
        X, f_b = v[:3], v[3]
        Re_i = Re_b * X * g["De"] / g["Deb"]
        # Eq. (35), which reduces to Eqs. (16) and (15) in the laminar and turbulent regimes
        f = g["C_L"] / Re_i * w_L + g["C_T"] / Re_i**PCTD_M * w_T
        # Eq. (36) with WmL = 0; the factor (X_i^(2-m) - X_j^(2-m)) / (X_i - X_j) of Eq. (32)
        ratio = lambda a, b: (
            (a ** (2.0 - PCTD_M) - b ** (2.0 - PCTD_M)) / (a - b)
            if abs(a - b) > 1e-10
            else (2.0 - PCTD_M) * a ** (1.0 - PCTD_M)
        )
        W12, W23 = (
            g["k_mix"]
            / Re_b**PCTD_M
            * w_T
            * np.array([ratio(X[0], X[1]), ratio(X[1], X[2])])
        )
        c = 2.0 * g["Deb"] / g["NA"]
        # Eqs. (8)-(10): equal bulk friction factor in the three types of subchannels; Eq. (11)
        return np.array(
            [
                f[0] * X[0] ** 2 * g["Deb"] / g["De"][0]
                + c[0] * W12 * (X[0] - X[1])
                - f_b,
                f[1] * X[1] ** 2 * g["Deb"] / g["De"][1]
                + c[1] * (W12 * (X[1] - X[0]) + W23 * (X[1] - X[2]))
                - f_b,
                f[2] * X[2] ** 2 * g["Deb"] / g["De"][2]
                + c[2] * W23 * (X[2] - X[1])
                - f_b,
                g["S"] @ X - 1.0,
            ]
        )

    # Newton iterations with a finite-difference Jacobian
    v = np.array([1.0, 1.0, 1.0, 0.05])
    for _ in range(100):
        r = residual(v)
        if np.max(np.abs(r)) < 1e-13:
            return v[:3]
        J = np.empty((4, 4))
        for k in range(4):
            dv = np.zeros(4)
            dv[k] = 1e-7 * max(abs(v[k]), 1e-3)
            J[:, k] = (residual(v + dv) - r) / dv[k]
        v = v - np.linalg.solve(J, r)
    raise RuntimeError(f"Lumped PCTD flow split did not converge at Re_b = {Re_b}")


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
# Geometry of Liang et al., Table 6 of the paper: n = 3 rings of pins around the center pin
LIANG_GEOMETRY = pctd_geometry(
    D=10.0e-3, Dw=0.9e-3, P=10.9e-3, H=95.5e-3, W=11.4e-3, n=3
)
Re_lumped = np.geomspace(RE_PLOT[0], RE_PLOT[1], 300)
X_lumped = np.array([pctd_flow_split(LIANG_GEOMETRY, Re_b) for Re_b in Re_lumped]).T
fig, ax = plt.subplots(figsize=(7.0, 5.0))
for k, channel in enumerate(COLORS):
    ax.plot(Re, X[channel], color=COLORS[channel], label=f"SCM PCTD, {channel}")
    ax.plot(
        *paper_liang_curve(channel),
        color=COLORS[channel],
        linestyle="--",
        label=f"PCTD paper, {channel}",
    )
    ax.plot(
        Re_lumped,
        X_lumped[k],
        color=COLORS[channel],
        linestyle=":",
        label=f"PCTD local $Re_i$, {channel}",
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


# Rows of the flow split table of the verification page: difference between SCM and the PCTD curve
# of the paper, absolute and as a percentage of the paper, at the SCM points of each band of the bulk
# Reynolds number. The mean over the band is followed by the maximum absolute difference.
Re, X, _ = load("pacio_liang_pctd_out.csv", n_rings=4)
BANDS = (
    ("Laminar", 0.0, RE_LAMINAR),
    ("Transition", RE_LAMINAR, 1500.0),
    ("Transition", 1500.0, 3000.0),
    ("Transition", 3000.0, RE_TURBULENT),
    ("Turbulent", RE_TURBULENT, np.inf),
)
for regime, lo, hi in BANDS:
    keep = (Re > lo) & (Re <= hi)
    cells = []
    for channel in COLORS:
        Re_paper, X_paper = (np.array(v) for v in paper_liang_curve(channel))
        order = np.argsort(Re_paper)
        paper = np.interp(np.log(Re[keep]), np.log(Re_paper[order]), X_paper[order])
        diff = X[channel][keep] - paper
        rel = 100.0 * diff / paper
        cells.append(
            f"{diff.mean():+.3f} ({np.abs(diff).max():.3f}), "
            f"{rel.mean():+.1f}% ({np.abs(rel).max():.1f}%)"
        )
    print(f"| {regime} | {lo:.0f} to {hi:.0f} | " + " | ".join(cells) + " |")
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
