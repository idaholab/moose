"""Plot the outlet subchannel temperatures of the radial heat conduction verification case.

The data are written by modules/subchannel/verification/radial_conduction_verification/
test_radial_conduction.py, which runs radial_conduction_7pin.i and solves the gap conductance
network independently. Rerun that test to regenerate radial_conduction_7pin_comparison.csv:

cd modules/subchannel/verification/radial_conduction_verification
python test_radial_conduction.py

The figure scm_radial_conduction_7pin.png is written next to this script.
"""

from pathlib import Path

import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt
import numpy as np

SCRIPT_DIR = Path(__file__).resolve().parent
DATA = SCRIPT_DIR / "../../../../../verification/radial_conduction_verification"
T_IN = 660.0

data = np.genfromtxt(
    DATA / "radial_conduction_7pin_comparison.csv", delimiter=",", names=True
)
order = np.argsort(data["x"])
x = data["x"][order] * 1e3
T_scm = data["T_solver"][order] - T_IN
T_net = data["T_network"][order] - T_IN

fig, (ax, ax_err) = plt.subplots(1, 2, figsize=(11, 4))
ax.plot(
    x,
    T_net,
    "s",
    markerfacecolor="none",
    markersize=9,
    color="black",
    label="Network solution",
)
ax.plot(x, T_scm, "o", markersize=4, color="red", label="SCM")
ax.set_xlabel("Subchannel centroid $x$ [mm]")
ax.set_ylabel("Outlet $T - T_{in}$ [K]")
ax.grid(True, alpha=0.3)
ax.legend()
ax_err.plot(x, T_scm - T_net, "o", color="red")
ax_err.set_xlabel("Subchannel centroid $x$ [mm]")
ax_err.set_ylabel("$T_{SCM} - T_{network}$ [K]")
ax_err.grid(True, alpha=0.3)
fig.tight_layout()
fig.savefig(
    SCRIPT_DIR / "scm_radial_conduction_7pin.png",
    dpi=200,
    bbox_inches="tight",
    facecolor="white",
)
print(f"Largest |T - T_in|: {np.abs(T_scm).max():.3f} K")
print(f"Largest |T_SCM - T_network|: {np.abs(T_scm - T_net).max():.2e} K")
