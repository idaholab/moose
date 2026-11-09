#!/usr/bin/env python3
# * This file is part of the MOOSE framework
# * https://mooseframework.inl.gov
# *
# * All rights reserved, see COPYRIGHT for full restrictions
# * https://github.com/idaholab/moose/blob/master/COPYRIGHT
# *
# * Licensed under LGPL 2.1, please see LICENSE for details
# * https://www.gnu.org/licenses/lgpl-2.1.html
"""
Plot the production history of the 2D dead-oil waterflood in waterflood_2d.i.

Run waterflood_2d.i first, then run this script from the same directory. It saves the figure
as waterflood_2d.png.
"""

import numpy as np
import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt

data = np.genfromtxt("waterflood_2d.csv", delimiter=",", names=True)
days = data["time"] / 86400.0
keep = days > 0

fig, (ax_rate, ax_cut) = plt.subplots(2, 1, figsize=(7, 6), sharex=True)
ax_rate.plot(days[keep], data["oil_rate"][keep], label="Oil")
ax_rate.plot(days[keep], data["water_rate"][keep], label="Water")
ax_rate.set_ylabel("Production rate (m$^3$/day, surface)")
ax_rate.legend(loc="center right")
ax_rate.grid(alpha=0.3)

ax_cut.plot(days[keep], data["water_cut"][keep], color="tab:green")
ax_cut.set_xlabel("Time (days)")
ax_cut.set_ylabel("Water cut")
ax_cut.set_ylim(0, 1)
ax_cut.grid(alpha=0.3)

plt.tight_layout()
plt.savefig("waterflood_2d.png", dpi=150)
print(
    "Cumulative oil produced: {:.0f} m3 (surface conditions)".format(
        data["cumulative_oil"][-1]
    )
)
print("Figure saved to waterflood_2d.png")
