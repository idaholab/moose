#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

# Plotting the results of dissolution_pyrite_2 and the equivalent GWB simulation

import os
import sys
import matplotlib.pyplot as plt

f = open("gold/dissolution_pyrite_2_out.csv", "r")
data = [list(map(float, line.strip().split(","))) for line in f.readlines()[2:]]
f.close()
tim = [x[0] * 100 for x in data]
hematite = [x[1] for x in data]
pyrite = [x[2] for x in data]
co2aq = [x[3] * 1000 for x in data]
fe = [x[4] * 1000 for x in data]
hco3 = [x[5] * 1000 for x in data]
o2aq = [x[6] * 1000 for x in data]
so4 = [x[7] * 1000 for x in data]
pH = [x[8] for x in data]

gwb_tim = [i * 100 for i in range(11)]
gwb_hematite = [0.9997, 67.53, 133.8, 199.5, 264.3, 328, 390.2, 450.9, 509.9, 567.1, 622.4]
gwb_pH = [6.5, 2.571, 2.291, 2.134, 2.025, 1.943, 1.877, 1.822, 1.776, 1.736, 1.7]

plt.figure(0)
plt.plot([0] + tim, [0.9997] + hematite, 'k-', linewidth = 2.0, label = 'MOOSE')
plt.plot(gwb_tim, gwb_hematite, 'ks', label = 'GWB')
plt.legend()
plt.xlabel("Pyrite reacted (mg)");
plt.ylabel("Hematite mass (mg)")
plt.title("Hematite precipitated when reacting pyrite in a system open to O$_{2}$(g)");
plt.savefig("../../../doc/content/media/geochemistry/dissolution_pyrite_2_1.png")

plt.figure(1)
plt.plot([0] + tim, [6.5] + pH, 'k-', linewidth = 2.0, label = 'MOOSE')
plt.plot(gwb_tim, gwb_pH, 'ks', label = 'GWB')
plt.legend()
plt.xlabel("Pyrite reacted (mg)");
plt.ylabel("pH")
plt.title("pH when reacting pyrite in a system open to O$_{2}$(g)");
plt.savefig("../../../doc/content/media/geochemistry/dissolution_pyrite_2_2.png")

sys.exit(0)
