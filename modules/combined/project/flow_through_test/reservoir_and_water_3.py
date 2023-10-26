#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

# Plotting the results of scaling.i

import os
import sys
import matplotlib.pyplot as plt

f = open("reservoir_and_water_3_out.csv", "r")
header = f.readline().strip().split(",")
data = [list(map(float, line.strip().split(","))) for line in f.readlines()[1:]]
f.close()

index = {}
for i in range(len(header)):
    index[header[i]] = i
years = [d[index["time"]] / 3600.0 / 24.0 / 365.0 for d in data]
all_minerals = ["Albite", "Anhydrite", "Anorthite", "Calcite", "Chalcedony", "Clinochl-7A", "Illite", "K-feldspar", "Kaolinite", "Quartz", "Paragonite", "Phlogopite", "Zoisite", "Laumontite", "mineral"]
cm3 = {}
for mineral in all_minerals:
    cm3[mineral] = [x[index["cm3_" + mineral]] for x in data]
change = {}
for mineral in all_minerals:
    change[mineral] = [c - cm3[mineral][0] for c in cm3[mineral]]
percentage_change = {}
for mineral in all_minerals:
    percentage_change[mineral] = [100 * (c - cm3[mineral][0]) / cm3[mineral][0] for c in cm3[mineral]]

# Plot the absolute changes in mineral volume
sortit = sorted([[change[mineral][-1], mineral] for mineral in all_minerals[:-1]])
plotorder = [m[1] for m in sortit]

plt.figure(0)
for mineral in reversed(plotorder):
    plt.semilogx(years, change[mineral], label=mineral)
plt.semilogx(years, change["mineral"], 'k--', label="Sum")
plt.legend()
plt.ylabel("Mineral volume change (cm$^{3}$)")
plt.xlabel("Years")
plt.title("Reservoir mineral volume when in contact with Water3 at 70$^{\circ}$C")
plt.tight_layout()
plt.savefig("../../../../geochemistry/doc/content/media/geochemistry/reservoir_and_water_3.png")

# Plot the percentage changes in mineral volume
sortit = sorted([[percentage_change[mineral][-1], mineral] for mineral in all_minerals[:-1]])
plotorder = [m[1] for m in sortit]

plt.figure(1)
for mineral in reversed(plotorder):
    plt.semilogx(years, percentage_change[mineral], label=mineral)
plt.semilogx(years, percentage_change["mineral"], 'k--', label="Sum")
plt.ylim(-100, 100)
plt.legend()
plt.ylabel("Percentage volume change (%)")
plt.xlabel("Years")
plt.title("Reservoir mineral volume when in contact with Water3 at 70$^{\circ}$C")
plt.tight_layout()
plt.savefig("../../../../geochemistry/doc/content/media/geochemistry/reservoir_and_water_3_percentage.png")
plt.show()

sys.exit(0)
