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

f = open("gold/mixing_out.csv", "r")
data = [list(map(float, line.strip().split(","))) for line in f.readlines()[2:]]
f.close()
tim = [x[0] * 1.036 for x in data] # 1.036 = mass of seawater
amsil = [x[1] for x in data]
anhydrite = [x[2] for x in data]
pyrite = [x[3] for x in data]
talc = [x[4] for x in data]
fug = [math.log10(x[5]) for x in data]
h2s = [x[6] * 1000 for x in data]
hso4 = [x[7] * 1000 for x in data]
naso4 = [x[8] * 1000 for x in data]
so4 = [x[9] * 1000 for x in data]
temp = [x[10] for x in data]

gwb_mass_reacted = [0, 0, 10.4, 51.8, 93.2, 445, 1.04e+03, 1.61e+03, 2.07e+03, 3.11e+03, 4.14e+03, 5.18e+03, 6.22e+03, 7.25e+03, 8.29e+03, 9.32e+03, 1.03e+04, 1.04e+04]
gwb_mass_reacted = [m / 1000.0 for m in gwb_mass_reacted]
gwb_amsil = [0, 0, 0, 0, 0, 2.458e-05, 0.001705, 0.002422, 0.002700, 0.002932, 0.002883, 0.002647, 0.002272, 0.001792, 0.001234, 0.0006171, 0, 0]
gwb_amsil = [m * 0.01790 / 0.0006171 for m in gwb_amsil]
gwb_anhydrite = [0, 0, 0, 0, 0.004828, 0.3905, 0.4397, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
gwb_pyrite = [0, 0, 0.002788, 0.01614, 0.01833, 0.02157, 0.02162, 0.02162, 0.02162, 0.02162, 0.02162, 0.02162, 0.02162, 0.02162, 0.02162, 0.02162, 0.02162, 0.02162]
gwb_talc = [0, 0, 0, 0.003834, 0.02382, 0.01794, 0.01377, 0.02157, 0.02810, 0.03756, 0.04123, 0.04248, 0.04307, 0.04369, 0.04457, 0.04577, 0.04715, 0.04705]
gwb_temp = [273.0, 273.0, 270.3, 260.2, 250.8, 192.1, 138.5, 109.5, 93.7, 71.3, 57.8, 48.8, 42.4, 37.6, 33.9, 30.9, 28.6, 28.5]
gwb_fug = [2.471e-34, 2.472e-34, 1.177e-33, 4.218e-34, 1.099e-34, 3.848e-40, 7.716e-47, 2.450e-51, 3.998e-54, 1.694e-58, 2.214e-61, 2.019e-63, 6.134e-65, 4.135e-66, 4.851e-67, 8.497e-68, 2.193e-68, 2.005e-68]
gwb_fug = [math.log10(f) for f in gwb_fug]
gwb_h2s = [0.006804, 0.006804, 0.006534, 0.005352, 0.005007, 0.003629, 0.002544, 0.001935, 0.001596, 0.001117, 0.0008434, 0.0006713, 0.0005547, 0.0004710, 0.0004081, 0.0003591, 0.0003222, 0.0003197]
gwb_h2s = [m * 1000 for m in gwb_h2s]
gwb_hso4 = [2.744e-06, 2.743e-06, 0.0002564, 0.001167, 0.001978, 0.001055, 0.0002328, 6.341e-05, 2.360e-05, 5.411e-06, 2.159e-06, 1.155e-06, 7.345e-07, 5.214e-07, 3.987e-07, 3.214e-07, 2.721e-07, 2.689e-07]
gwb_hso4 = [m * 1000 for m in gwb_hso4]
gwb_naso4 = [1.752e-08, 1.749e-08, 3.239e-06, 3.001e-05, 6.868e-05, 0.0005695, 0.003174, 0.005784, 0.006294, 0.006895, 0.007180, 0.007336, 0.007433, 0.007497, 0.007542, 0.007575, 0.007599, 0.007600]
gwb_naso4 = [m * 1000 for m in gwb_naso4]
gwb_so4 = [1.858e-08, 1.855e-08, 3.579e-06, 3.805e-05, 9.658e-05, 0.001067, 0.006204, 0.01168, 0.01299, 0.01492, 0.01615, 0.01699, 0.01761, 0.01809, 0.01846, 0.01876, 0.01899, 0.01901]
gwb_so4 = [m * 1000 for m in gwb_so4]

plt.figure(0)
plt.plot(tim, temp, 'k-', linewidth = 2.0, label = 'MOOSE')
plt.plot(gwb_mass_reacted, gwb_temp, 'ks', label = 'GWB')
plt.legend()
plt.xlabel("Seawater reacted (kg)");
plt.ylabel("Temperature ($^{\circ}$C)")
plt.title("Temperature when mixing seawater with hot hydrothermal fluid");
plt.savefig("../../../doc/content/media/geochemistry/mixing_1.png")

plt.figure(2)
plt.plot(tim, fug, 'k-', linewidth = 2.0, label = 'MOOSE')
plt.plot(gwb_mass_reacted, gwb_fug, 'ks', linewidth = 2.0, label = 'GWB')
plt.legend()
plt.xlabel("Seawater reacted (kg)");
plt.ylabel("log$_{10}$(O$_{2}$(g) fugacity)")
plt.title("Oxygen fugacity when mixing seawater with hot hydrothermal fluid");
plt.savefig("../../../doc/content/media/geochemistry/mixing_3.png")

plt.figure(4)
plt.plot(tim, h2s, 'k-', linewidth = 2.0, label = 'H$_{2}$S(aq) (MOOSE)')
plt.plot(tim, hso4, 'r-', linewidth = 2.0, label = 'HSO$_{4}^{-}$ (MOOSE)')
plt.plot(tim, naso4, 'b-', linewidth = 2.0, label = 'NaSO$_{4}^{-}$ (MOOSE)')
plt.plot(tim, so4, 'g-', linewidth = 2.0, label = 'SO$_{4}^{2-}$ (MOOSE)')
plt.plot(gwb_mass_reacted, gwb_h2s, 'ks', linewidth = 2.0, label = 'H$_{2}$S(aq) (GWB)')
plt.plot(gwb_mass_reacted, gwb_hso4, 'rs', linewidth = 2.0, label = 'HSO$_{4}^{-}$ (GWB)')
plt.plot(gwb_mass_reacted, gwb_naso4, 'bs', linewidth = 2.0, label = 'NaSO$_{4}^{-}$ (GWB)')
plt.plot(gwb_mass_reacted, gwb_so4, 'gs', linewidth = 2.0, label = 'SO$_{4}^{2-}$ (GWB)')
plt.legend()
plt.xlabel("Seawater reacted (kg)");
plt.ylabel("Concentration (mmolal)")
plt.title("Species concentration when mixing seawater with hot hydrothermal fluid");
plt.savefig("../../../doc/content/media/geochemistry/mixing_4.png")

plt.figure(1)
plt.plot(tim, amsil, 'k-', linewidth = 2.0, label = 'AmSilica (MOOSE)')
plt.plot(tim, anhydrite, 'r-', linewidth = 2.0, label = 'Anhydrite (MOOSE)')
plt.plot(tim, pyrite, 'g-', linewidth = 2.0, label = 'Pyrite (MOOSE)')
plt.plot(tim, talc, 'b-', linewidth = 2.0, label = 'Talc (MOOSE)')
plt.plot(gwb_mass_reacted, gwb_amsil, 'ks', linewidth = 2.0, label = 'AmSilica (GWB)')
plt.plot(gwb_mass_reacted, gwb_anhydrite, 'rs', linewidth = 2.0, label = 'Anhydrite (GWB)')
plt.plot(gwb_mass_reacted, gwb_pyrite, 'gs', linewidth = 2.0, label = 'Pyrite (GWB)')
plt.plot(gwb_mass_reacted, gwb_talc, 'bs', linewidth = 2.0, label = 'Talc (GWB)')
plt.legend()
plt.xlabel("Seawater reacted (kg)");
plt.ylabel("Precipitated volume (cm$^{3}$)")
plt.title("Minerals precipitated when mixing seawater with hot hydrothermal fluid");
plt.savefig("../../../doc/content/media/geochemistry/mixing_2.png")

sys.exit(0)
