#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

# Plotting the results of natural_reservoir.i

import os
import sys
import matplotlib.pyplot as plt

f = open("natural_reservoir_out.csv", "r")
header = f.readline().strip().split(",")
data = [list(map(float, line.strip().split(","))) for line in f.readlines()[1:]]
f.close()

index = {}
for i in range(len(header)):
    index[header[i]] = i
years = [d[index["time"]] / 3600.0 / 24.0 / 365.0 for d in data]
pH = [d[index["pH"]] for d in data]
kg_solvent_H2O = [d[index["kg_solvent_H2O"]] for d in data]
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
plt.title("Natural mineral volume change in the reservoir")
plt.tight_layout()
plt.savefig("../../../../geochemistry/doc/content/media/geochemistry/natural_reservoir_minerals.png")

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
plt.title("Natural mineral volume change in the reservoir")
plt.tight_layout()
plt.savefig("../../../../geochemistry/doc/content/media/geochemistry/natural_reservoir_mineral_percentage.png")

plt.figure(2)
fig, ax1 = plt.subplots()
ax2 = ax1.twinx()
ax1.semilogx(years, kg_solvent_H2O, 'g-')
ax2.semilogx(years, pH, 'b-')
ax1.set_xlabel('Years')
ax1.set_ylabel('Solvent water mass (kg)', color='g')
ax2.set_ylabel('pH', color='b')
plt.title("Water changes in the reservoir")
plt.tight_layout()
plt.savefig("../../../../geochemistry/doc/content/media/geochemistry/natural_reservoir_solution.png")

plt.show()

sys.exit(0)
