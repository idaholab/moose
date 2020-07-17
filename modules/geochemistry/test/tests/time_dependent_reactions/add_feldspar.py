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

f = open("gold/add_feldspar_out.csv", "r")
data = [list(map(float, line.strip().split(","))) for line in f.readlines()[1:]]
f.close()
feldspar_added = [x[0] * 0.15 for x in data]
cm3_Kfeldspar = [x[1] for x in data]
cm3_Kaolinit = [x[2] for x in data]
cm3_Muscovite = [x[3] for x in data]
cm3_Phengite = [x[4] for x in data]
cm3_Quartz = [x[5] for x in data]

plt.figure()
plt.plot(feldspar_added, cm3_Kfeldspar, 'k-', linewidth = 2.0, label = 'K-feldspar')
plt.plot(feldspar_added, cm3_Kaolinit, 'r-', linewidth = 2.0, label = 'Kaolinite')
plt.plot(feldspar_added, cm3_Muscovite, 'g-', linewidth = 2.0, label = 'Muscovite')
plt.plot(feldspar_added, cm3_Phengite, 'b-', linewidth = 2.0, label = 'Phengite')
plt.plot(feldspar_added, cm3_Quartz, 'y-', linewidth = 2.0, label = 'Quartz')
plt.legend()
plt.xlabel("K-feldspar added (cm$^{3}$)")
plt.ylabel("precipitate volume (cm$^{3}$)")
plt.title("Minerals precipitated");
plt.savefig("../../../doc/content/media/geochemistry/add_feldspar.png")

sys.exit(0)
