#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

# Plotting the results of changing_pH_ferric_hydroxide.i

import os
import sys
import matplotlib.pyplot as plt

f = open("gold/changing_pH_ferric_hydroxide_out.csv", "r")
data = [list(map(float, line.strip().split(","))) for line in f.readlines()[2:]]
f.close()
sFeO = [x[1] * 1000 for x in data]
sFeOH = [x[2] * 1000 for x in data]
sFeOH2 = [x[3] * 1000 for x in data]
wFeO = [x[4] * 1000 for x in data]
wFeOH = [x[5] * 1000 for x in data]
wFeOH2 = [x[6] * 1000 for x in data]
pH = [x[7] for x in data]
pot = [x[8] * 1000 for x in data]

plt.figure(0)
plt.plot(pH, sFeO, 'k--', linewidth = 2.0, label = '>(s)FeO-')
plt.plot(pH, sFeOH, 'r-', linewidth = 2.0, label = '>(s)FeOH')
plt.plot(pH, sFeOH2, 'g-', linewidth = 2.0, label = '>(s)FeOH2+')
plt.plot(pH, wFeO, 'b-', linewidth = 2.0, label = '>(w)FeO-')
plt.plot(pH, wFeOH, 'y-', linewidth = 2.0, label = '>(w)FeOH')
plt.plot(pH, wFeOH2, 'k-', linewidth = 2.0, label = '>(w)FeOH2+')
plt.legend()
plt.xlabel("pH")
plt.ylabel("Species concentration (mmolal)")
plt.title("Concentrations of sites on Fe(OH)3(ppd)")
plt.savefig("../../../doc/content/media/geochemistry/changing_pH_ferric_hydroxide_fig1.png")

plt.figure(1)
plt.plot(pH, pot, 'k-', linewidth = 2.0, label = 'potential')
plt.xlabel("pH")
plt.ylabel("Surface potential (mV)")
plt.title("Surface potential of Fe(OH)3(ppd)")
plt.savefig("../../../doc/content/media/geochemistry/changing_pH_ferric_hydroxide_fig2.png")

sys.exit(0)
