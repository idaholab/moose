#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

# Plotting the results of gypsum_solubility and the equivalent GWB simulation

import os
import sys
import matplotlib.pyplot as plt

f = open("gold/cooling_out.csv", "r")
data = [list(map(float, line.strip().split(","))) for line in f.readlines()[2:]]
f.close()
albite = [x[1] for x in data]
max_micro = [x[2] for x in data]
muscovite = [x[3] for x in data]
quartz = [x[4] for x in data]
temp = [x[5] for x in data]

gwb_temp = [300, 245, 217.5, 162.5, 135, 80, 52.5, 25]
gwb_albite = [20.04, 14.63, 12.47, 9.2, 8.046, 6.578, 6.205, 6.016]
gwb_max_micro = [10.00, 15.85, 18.17, 21.73, 23, 24.59, 24.98, 25.19]
gwb_muscovite = [4.999, 5.001, 5.002, 5.002, 5.002, 5.002, 5.002, 5.002]
gwb_quartz = [2.000, 2.040, 2.063, 2.1, 2.114, 2.133, 2.138, 2.140]

plt.figure()
plt.plot(temp, albite, 'k-', linewidth = 2.0, label = 'Albite (MOOSE)')
plt.plot(temp, max_micro, 'r-', linewidth = 2.0, label = 'Microcline (MOOSE)')
plt.plot(temp, muscovite, 'g-', linewidth = 2.0, label = 'Muscovite (MOOSE)')
plt.plot(temp, quartz, 'b-', linewidth = 2.0, label = 'Quartz (MOOSE)')
plt.plot(gwb_temp, gwb_albite, 'ks', label = "Albite (GWB)")
plt.plot(gwb_temp, gwb_max_micro, 'rs', label = "Microcline (GWB)")
plt.plot(gwb_temp, gwb_muscovite, 'gs', label = "Muscovite (GWB)")
plt.plot(gwb_temp, gwb_quartz, 'bs', label = "Quartz (GWB)")
ax = plt.gca()
ax.set_xlim(ax.get_xlim()[::-1])
plt.legend()
plt.xlabel("Temperature (degC)")
plt.ylabel("precipitate volume (cm$^{3}$)")
plt.title("Minerals precipitated in a cooling solution");
plt.savefig("../../../doc/content/media/geochemistry/cooling.png")

sys.exit(0)
