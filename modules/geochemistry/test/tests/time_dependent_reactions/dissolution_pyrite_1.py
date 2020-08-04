#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

# Plotting the results of dissolution_pyrite_1 and the equivalent GWB simulation

import os
import sys
import matplotlib.pyplot as plt

f = open("gold/dissolution_pyrite_1_out.csv", "r")
data = [list(map(float, line.strip().split(","))) for line in f.readlines()[2:]]
f.close()
tim = [x[0] for x in data]
hematite = [x[1] for x in data]
pyrite = [x[2] for x in data]
co2aq = [x[3] * 1000 for x in data]
fe = [x[4] * 1000 for x in data]
hco3 = [x[5] * 1000 for x in data]
o2aq = [x[6] * 1000 for x in data]
so4 = [x[7] * 1000 for x in data]
pH = [x[8] for x in data]

gwb_tim = [0, 1, 2, 3, 4, 5, 6, 7, 8, 8.756, 8.84, 9, 10]
gwb_hematite = [0.9997, 1.665, 2.331, 2.996, 3.662, 4.328, 4.993, 5.659, 6.324, 0, 0, 0, 0]
gwb_pyrite = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8.372e-2, 0.2441, 1.244]
gwb_pH = [6.5, 6.303, 6.102, 5.872, 5.565, 5.058, 4.507, 4.214, 4.035, 5.691, 5.691, 5.691, 5.691]
gwb_co2aq = [0.1229, 0.1560, 0.1890, 0.2218, 0.2537, 0.2808, 0.2908, 0.2929, 0.2936, 0.2420, 0.2420, 0.2420, 0.2420]
gwb_fe = [0, 0, 0, 0, 0, 0, 0, 0, 0, 8.367e-2, 8.367e-2, 8.367e-2, 8.367e-2]
gwb_hco3 = [0.1717, 0.1387, 0.1057, 7.306e-2, 4.122e-2, 1.420e-2, 4.142e-3, 2.126e-3, 1.414e-3, 5.287e-2, 5.287e-2, 5.287e-2, 5.287e-2]
gwb_o2aq = [0.2523, 0.2211, 0.1898, 0.1586, 0.1273, 9.603e-2, 6.477e-2, 3.351e-2, 2.250e-3, 0, 0, 0, 0]
gwb_so4 = [3.053e-2, 4.684e-2, 6.315e-2, 7.945e-2, 9.576e-2, 0.112, 0.1281, 0.144, 0.1598, 0.1718, 0.1718, 0.1718, 0.1718]

plt.figure(0)
plt.plot([0] + tim, [0.9997] + hematite, 'k-', linewidth = 2.0, label = 'Hematite (MOOSE)')
plt.plot([0] + tim, [0] + pyrite, 'r-', linewidth = 2.0, label = 'Pyrite (MOOSE)')
plt.plot(gwb_tim, gwb_hematite, 'ks', label = 'Hematite (GWB)')
plt.plot(gwb_tim, gwb_pyrite, 'rs', label = 'Pyrite (GWB)')
plt.legend()
plt.xlabel("Pyrite reacted (mg)");
plt.ylabel("Precipitate mass (mg)")
plt.title("Minerals precipitated when reacting pyrite in a system closed to O$_{2}$(g)");
plt.savefig("../../../doc/content/media/geochemistry/dissolution_pyrite_1_1.png")

plt.figure(1)
plt.plot([0] + tim, [6.5] + pH, 'k-', linewidth = 2.0, label = 'MOOSE')
plt.plot(gwb_tim, gwb_pH, 'ks', label = 'GWB')
plt.legend()
plt.xlabel("Pyrite reacted (mg)");
plt.ylabel("pH")
plt.title("pH when reacting pyrite in a system closed to O$_{2}$(g)");
plt.savefig("../../../doc/content/media/geochemistry/dissolution_pyrite_1_2.png")

plt.figure(2)
plt.plot(tim, co2aq, 'k-', linewidth = 2.0, label = 'CO2(aq) (MOOSE)')
plt.plot(tim, fe, 'r-', linewidth = 2.0, label = 'Fe$^{2+}$ (MOOSE)')
plt.plot(tim, hco3, 'g-', linewidth = 2.0, label = 'HCO3$^{-}$ (MOOSE)')
plt.plot(tim, o2aq, 'b-', linewidth = 2.0, label = 'O2(aq) (MOOSE)')
plt.plot(tim, so4, 'y-', linewidth = 2.0, label = 'SO$_{4}^{2-}$ (MOOSE)')
plt.plot(gwb_tim, gwb_co2aq, 'ks', linewidth = 2.0, label = 'CO2(aq) (GWB)')
plt.plot(gwb_tim, gwb_fe, 'rs', linewidth = 2.0, label = 'Fe$^{2+}$ (GWB)')
plt.plot(gwb_tim, gwb_hco3, 'gs', linewidth = 2.0, label = 'HCO3$^{-}$ (GWB)')
plt.plot(gwb_tim, gwb_o2aq, 'bs', linewidth = 2.0, label = 'O2(aq) (GWB)')
plt.plot(gwb_tim, gwb_so4, 'ys', linewidth = 2.0, label = 'SO$_{4}^{2-}$ (GWB)')
plt.legend()
plt.xlabel("Pyrite reacted (mg)");
plt.ylabel("Species concentration (mmolal)")
plt.title("Species concentrations when reacting pyrite in a system closed to O$_{2}$(g)");
plt.savefig("../../../doc/content/media/geochemistry/dissolution_pyrite_1_3.png")

sys.exit(0)
