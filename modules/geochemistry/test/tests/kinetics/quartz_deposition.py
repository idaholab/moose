#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

# Plotting the results of quartz_dissolution.i

import os
import sys
import matplotlib.pyplot as plt

f = open("gold/quartz_deposition_out.csv", "r")
data = [list(map(float, line.strip().split(","))) for line in f.readlines()[2:]]
f.close()
si02 = [x[1] for x in data]
rate = [-x[2] * 1000 for x in data]
temp = [x[3] for x in data]

gwb_temp = [300, 272.5, 245, 217.5, 190.0, 162.5, 135, 107.5, 80, 52.5, 25]
gwb_si02 = [583.9, 563.2, 527.8, 497.0, 476.3, 464.7, 459.2, 457.0, 456.2, 456.0, 455.9]
gwb_rate = [0, 1.745e-10, 1.842e-10, 1.37e-10, 8.291e-11, 4.227e-11, 1.834e-11, 6.788e-12, 2.141e-12, 5.713e-13, 1.269e-13]
gwb_rate = [1000 * 3600 * 24 * 365 * r for r in gwb_rate]

plt.figure(0)
plt.plot(temp, si02, 'k-', linewidth = 2.0, label = 'MOOSE')
plt.plot(gwb_temp, gwb_si02, 'ks', linewidth = 2.0, label = 'GWB')
plt.grid()
plt.legend()
plt.xlabel("Temperature ($^{\circ}$C)")
plt.ylabel("SiO$_{2}$(aq) (mg/kg(soln))")
plt.title("Kinetic deposition of quartz in a fracture")
ax = plt.gca()
ax.set_xlim(ax.get_xlim()[::-1])
plt.savefig("../../../doc/content/media/geochemistry/quartz_deposition1.png")

plt.figure(1)
plt.plot([300] + temp, [0] + rate, 'k-', linewidth = 2.0, label = 'MOOSE')
plt.plot(gwb_temp, gwb_rate, 'ks', linewidth = 2.0, label = 'GWB')
plt.grid()
plt.legend()
plt.xlabel("Temperature ($^{\circ}$C)")
plt.ylabel("Quartz deposition rate (mmol/yr)")
plt.title("Kinetic deposition of quartz in a fracture")
ax = plt.gca()
ax.set_xlim(ax.get_xlim()[::-1])
plt.savefig("../../../doc/content/media/geochemistry/quartz_deposition2.png")
sys.exit(0)
