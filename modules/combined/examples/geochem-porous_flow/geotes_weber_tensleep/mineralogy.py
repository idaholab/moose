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

f = open("gold/exchanger_out_porous_flow_sim0_react0_mineralogy.csv", "r")
header = f.readline().strip().split(",")
data = [list(map(float, line.strip().split(","))) for line in f.readlines()[1:]]
f.close()

index = {}
for i in range(len(header)):
    index[header[i]] = i
all_minerals = ["Siderite", "Pyrrhotite", "Dolomite", "Illite", "Anhydrite", "Calcite", "Quartz", "K-feldspar", "Kaolinite", "Barite", "Celestite", "Fluorite", "Albite", "Chalcedony", "Goethite"]
cm3 = {}
for mineral in all_minerals:
    cm3[mineral] = [x[index["free_cm3_" + mineral]] for x in data]
del_cm3 = {}
for mineral in all_minerals:
    del_cm3[mineral] = [cm - cm3[mineral][0] for cm in cm3[mineral]]
del_rel = {}
for mineral in all_minerals:
    del_rel[mineral] = [(cm - cm3[mineral][0]) / cm3[mineral][0] if (cm3[mineral][0] != 0) else 0 for cm in cm3[mineral]]
days = [x[index["time"]] / 3600.0 / 24.0 for x in data]

change = [del_cm3[mineral][-1] for mineral in all_minerals]
x_pos = [i for i, _ in enumerate(all_minerals)]
plt.figure(0)
plt.bar(x_pos, change, color='blue')
plt.ylabel("Change in volume (cm$^{3}$)")
plt.title("Change in mineral volume around the injection well")
plt.xticks(x_pos, all_minerals)
plt.setp(plt.gca().get_xticklabels(), rotation=-90, horizontalalignment='center')
plt.tight_layout()
plt.savefig("../../../../geochemistry/doc/content/media/geochemistry/geotes_weber_tensleep_mineralogy_abs.png")


change = [del_rel[mineral][-1] * 100.0 for mineral in all_minerals]
x_pos = [i for i, _ in enumerate(all_minerals)]
plt.figure(1)
plt.bar(x_pos, change, color='blue')
plt.ylabel("Fractional change in volume (%)")
plt.title("Fractional change in mineral volume around the injection well")
plt.xticks(x_pos, all_minerals)
plt.setp(plt.gca().get_xticklabels(), rotation=-90, horizontalalignment='center')
plt.tight_layout()
plt.savefig("../../../../geochemistry/doc/content/media/geochemistry/geotes_weber_tensleep_mineralogy_rel.png")
plt.show()

sys.exit(0)
