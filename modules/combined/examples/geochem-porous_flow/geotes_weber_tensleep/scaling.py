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

f = open("gold/scaling_out.csv", "r")
data = [list(map(float, line.strip().split(","))) for line in f.readlines()[2:]]
f.close()
anhydrite_mol = [x[1] for x in data]
anhydrite = [(a - anhydrite_mol[0]) * 45.94 for a in anhydrite_mol]
barite_mol = [x[2] for x in data]
calcite_mol = [x[3] for x in data]
dolomite_mol = [x[4] for x in data]
illite_mol = [x[5] for x in data]
kfeldspar_mol = [x[6] for x in data]
kaolinite_mol = [x[7] for x in data]
kaolinite = [(a - kaolinite_mol[0]) * 99.52 for a in kaolinite_mol]
pyrrhotite_mol = [x[8] for x in data]
quartz_mol = [x[9] for x in data]
temperature = [x[10] for x in data]

plt.figure()
plt.semilogy(temperature, anhydrite, 'k-', linewidth = 2.0, label = 'Anhydrite')
plt.semilogy(temperature, kaolinite, 'r-', linewidth = 2.0, label = 'Kaolinite')
plt.legend()
plt.ylabel("Precipitate (cm$^{3}$) per 1$\,$L of formation water")
plt.xlabel("Temperature")
plt.title("Minerals precipitated in the heat exchanger")
plt.savefig("../../../../geochemistry/doc/content/media/geochemistry/geotes_weber_tensleep_scaling.png")

sys.exit(0)
