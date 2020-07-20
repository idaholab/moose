#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

# Plotting the results of changing_fugacity_calcite and the equivalent GWB simulation

import os
import sys
import matplotlib.pyplot as plt

f = open("gold/changing_fugacity_calcite_out.csv", "r")
data = [list(map(float, line.strip().split(","))) for line in f.readlines()[2:]]
f.close()
calcite = [x[1] for x in data]
fug = [x[2] for x in data]
co2aq = [x[3] * 1000 for x in data]
ca = [x[4] * 1000 for x in data]
hco3 = [x[5] * 1000 for x in data]
ph = [x[6] for x in data]

gwb_tim = [0, 0.1, 0.2, 0.4, 0.6, 0.8, 1.0]
gwb_calcite = [0.5001, 0.3809, 0.3409, 0.2883, 0.2499, 0.2185, 0.1915]
gwb_pH = [8.262, 6.613, 6.417, 6.221, 6.107, 6.026, 5.963]
gwb_co2aq = [1.116e-2, 3.539, 7.066, 14.12, 21.18, 28.23, 35.29]
gwb_ca = [0.4874, 3.398, 4.317, 5.494, 6.329, 6.999, 7.569]
gwb_hco3 = [0.9767, 7.158, 9.165, 11.77, 13.65, 15.17, 16.47]

plt.figure(0)
plt.plot([3.1622777e-4] + fug, [0.5] + calcite, 'k-', linewidth = 2.0, label = 'MOOSE')
plt.plot(gwb_tim, gwb_calcite, 'ks', label = 'GWB')
plt.legend()
plt.xlabel("CO$_{2}$ fugacity");
plt.ylabel("Calcite volume (cm$^{3}$)")
plt.title("Calcite volume as CO$_{2}$ fugacity is varied");
plt.savefig("../../../doc/content/media/geochemistry/changing_fugacity_calcite_1.png")

plt.figure(1)
plt.plot([3.1622777e-4] + fug, [8.262] + ph, 'k-', linewidth = 2.0, label = 'MOOSE')
plt.plot(gwb_tim, gwb_pH, 'ks', label = 'GWB')
plt.legend()
plt.xlabel("CO$_{2}$ fugacity");
plt.ylabel("pH")
plt.title("pH as CO$_{2}$ fugacity is varied");
plt.savefig("../../../doc/content/media/geochemistry/changing_fugacity_calcite_2.png")

plt.figure(2)
plt.plot(fug, co2aq, 'k-', linewidth = 2.0, label = 'CO2(aq) (MOOSE)')
plt.plot(fug, ca, 'r-', linewidth = 2.0, label = 'Ca$^{2+}$ (MOOSE)')
plt.plot(fug, hco3, 'g-', linewidth = 2.0, label = 'HCO3$^{-}$ (MOOSE)')
plt.plot(gwb_tim, gwb_co2aq, 'ks', linewidth = 2.0, label = 'CO2(aq) (GWB)')
plt.plot(gwb_tim, gwb_ca, 'rs', linewidth = 2.0, label = 'Ca$^{2+}$ (GWB)')
plt.plot(gwb_tim, gwb_hco3, 'gs', linewidth = 2.0, label = 'HCO3$^{-}$ (GWB)')
plt.legend()
plt.xlabel("CO$_{2}$ fugacity");
plt.ylabel("Species concentration (mmolal)")
plt.title("Species concentrations as CO$_{2}$ fugacity is varied");
plt.savefig("../../../doc/content/media/geochemistry/changing_fugacity_calcite_3.png")

sys.exit(0)
