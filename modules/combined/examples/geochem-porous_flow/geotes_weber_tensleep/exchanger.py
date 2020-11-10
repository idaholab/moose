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

f = open("gold/exchanger_out.csv", "r")
header = f.readline().strip().split(",")
data = [list(map(float, line.strip().split(","))) for line in f.readlines()[1:]]
f.close()

index = {}
for i in range(len(header)):
    index[header[i]] = i
all_minerals = ["Siderite", "Pyrrhotite", "Dolomite", "Illite", "Anhydrite", "Calcite", "Quartz", "K-feldspar", "Kaolinite", "Barite", "Celestite", "Fluorite", "Albite", "Chalcedony", "Goethite"]
mol_vol = [28.63, 17.62, 64.365, 138.94, 45.94, 36.934, 22.688, 108.87, 99.52, 52.1, 46.25, 24.54, 100.07, 22.688, 20.82]
cum_mols = {}
for i in range(len(all_minerals)):
    cum_mols[all_minerals[i]] = [x[index["cumulative_moles_precipitated_" + all_minerals[i]]] for x in data]
cum_cm3 = {}
for i in range(len(all_minerals)):
    cum_cm3[all_minerals[i]] = [x * mol_vol[i] for x in cum_mols[all_minerals[i]]]
grams_heated = [x[index["mass_heated_this_timestep"]] for x in data]
total_litres_heated = sum(grams_heated) / 1000.0

precip = [cum_cm3[mineral][-1] / total_litres_heated for mineral in all_minerals]
x_pos = [i for i, _ in enumerate(all_minerals)]
plt.bar(x_pos, precip, color='blue')
plt.ylabel("Precipitate (cm$^{3}$/litre heated)")
plt.title("Precipitated minerals in the heat exchanger")
plt.xticks(x_pos, all_minerals)
plt.yscale("log")
plt.setp(plt.gca().get_xticklabels(), rotation=-90, horizontalalignment='center')
plt.tight_layout()
plt.savefig("../../../../geochemistry/doc/content/media/geochemistry/geotes_weber_tensleep_exchanger.png")

sys.exit(0)
