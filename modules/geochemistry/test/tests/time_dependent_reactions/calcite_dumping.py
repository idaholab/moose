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
import math
import matplotlib.pyplot as plt

f = open("gold/calcite_dumping_dump.csv", "r")
data = [list(map(float, line.strip().split(","))) for line in f.readlines()[2:]]
f.close()
tim = [x[0] for x in data]
fug = [x[2] for x in data]
ph = [x[7] for x in data]

f = open("gold/calcite_dumping_no_dump.csv", "r")
data = [list(map(float, line.strip().split(","))) for line in f.readlines()[2:]]
f.close()
fugc = [x[2] for x in data]
co2 = [x[3] * 1000 for x in data]
ca = [x[4] * 1000 for x in data]
cacl = [x[5] * 1000 for x in data]
hco3 = [x[6] * 1000 for x in data]
phc = [x[7] for x in data]


gwb_moles_reacted = [i * 10 for i in range(11)]
gwb_ph = [8.000, 2.095, 1.790, 1.613, 1.488, 1.391, 1.313, 1.246, 1.189, 1.138, 1.092]
gwb_fug = [0.0001072, 0.007040, 0.007040, 0.007041, 0.007041, 0.007041, 0.007041, 0.007041, 0.007041, 0.007041, 0.007041]

gwb_phc = [8.000, 6.479, 6.223, 6.074, 5.969, 5.887, 5.820, 5.763, 5.714, 5.671, 5.632]
gwb_fugc = [0.0001072, 0.07318, 0.1837, 0.3043, 0.4297, 0.5579, 0.6878, 0.8190, 0.9510, 1.084, 1.217]
gwb_ca = [0.008393, 0.01411, 0.01870, 0.02296, 0.02703, 0.03096, 0.03480, 0.03856, 0.04225, 0.04587, 0.04944]
gwb_ca = [m * 1000 for m in gwb_ca]
gwb_co2 = [3.784e-6, 0.002582, 0.006480, 0.01074, 0.01516, 0.01969, 0.02427, 0.02890, 0.03356, 0.03824, 0.04295]
gwb_co2 = [m * 1000 for m in gwb_co2]
gwb_cacl = [0.001840, 0.003188, 0.004388, 0.005591, 0.006825, 0.008097, 0.009409, 0.01076, 0.01216, 0.01359, 0.01507]
gwb_cacl = [m * 1000 for m in gwb_cacl]
gwb_hco3 = [0.0002108, 0.004374, 0.006147, 0.007273, 0.008095, 0.008743, 0.009280, 0.009738, 0.01014, 0.01050, 0.01082]
gwb_hco3 = [m * 1000 for m in gwb_hco3]

plt.figure(0)
plt.plot(tim, fug, 'k-', linewidth = 2.0, label = 'No calcite (MOOSE)')
plt.plot(tim, fugc, 'r-', linewidth = 2.0, label = 'With calcite (MOOSE)')
plt.plot(gwb_moles_reacted, gwb_fug, 'ks', linewidth = 2.0, label = 'No calcite (GWB)')
plt.plot(gwb_moles_reacted, gwb_fugc, 'rs', linewidth = 2.0, label = 'With calcite (GWB)')
plt.legend()
plt.xlabel("HCl reacted (mmol)");
plt.ylabel("CO$_{2}$(g) fugacity");
plt.title("CO$_{2}$(g) fugacity: add HCl to fluid with/without calcite")
plt.savefig("../../../doc/content/media/geochemistry/calcite_dumping_1.png")

plt.figure(1)
plt.plot([0] + tim, [8] + ph, 'k-', linewidth = 2.0, label = 'No calcite (MOOSE)')
plt.plot([0] + tim, [8] + phc, 'r-', linewidth = 2.0, label = 'With calcite (MOOSE)')
plt.plot(gwb_moles_reacted, gwb_ph, 'ks', linewidth = 2.0, label = 'No calcite (GWB)')
plt.plot(gwb_moles_reacted, gwb_phc, 'rs', linewidth = 2.0, label = 'With calcite (GWB)')
plt.legend()
plt.xlabel("HCl reacted (mmol)")
plt.ylabel("pH")
plt.title("pH: add HCl to fluid with/without calcite")
plt.savefig("../../../doc/content/media/geochemistry/calcite_dumping_2.png")

plt.figure(2)
plt.plot(tim, co2, 'k-', linewidth = 2.0, label = 'CO$_{2}$(aq) (MOOSE)')
plt.plot(tim, ca, 'r-', linewidth = 2.0, label = 'Ca$^{2+}$ (MOOSE)')
plt.plot(tim, cacl, 'g-', linewidth = 2.0, label = 'CaCl$^{-}$ (MOOSE)')
plt.plot(tim, hco3, 'b-', linewidth = 2.0, label = 'HCO3$^{-}$ (MOOSE)')
plt.plot(gwb_moles_reacted, gwb_co2, 'ks', linewidth = 2.0, label = 'CO$_{2}$(aq) (GWB)')
plt.plot(gwb_moles_reacted, gwb_ca, 'rs', linewidth = 2.0, label = 'Ca$^{2+}$ (GWB)')
plt.plot(gwb_moles_reacted, gwb_cacl, 'gs', linewidth = 2.0, label = 'CaCl$^{-}$ (GWB)')
plt.plot(gwb_moles_reacted, gwb_hco3, 'bs', linewidth = 2.0, label = 'HCO3$^{-}$ (GWB)')
plt.legend()
plt.xlabel("HCl reacted (mmol)")
plt.ylabel("Species concentrations (mmol)")
plt.title("Concentration: add HCl to fluid with calcite")
plt.savefig("../../../doc/content/media/geochemistry/calcite_dumping_3.png")

sys.exit(0)
