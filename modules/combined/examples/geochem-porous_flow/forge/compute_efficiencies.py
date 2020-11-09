#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html


import os
import sys
import matplotlib.pyplot as plt
import numpy as np

# Plotting the results of multiple runs of FORGE aquifer_geochemistry.i with 15251 nodes and no exodus output

cpus = np.array([1, 1, 1, 2, 2, 2, 4, 4, 4, 6, 6, 6, 8, 8, 8, 10, 10, 10, 16, 16, 16, 20, 20, 20, 20])
times = np.array([2001, 2000, 2000, 1010, 1011, 1010, 534, 533, 531, 387, 409, 399, 298, 313, 317, 233, 236, 241, 156, 157, 158, 144, 135, 132, 132])

logcp = np.log(cpus)
logt = np.log(times)
m, c = np.polyfit(logcp, logt, 1)
tfit = np.exp(m * logcp + c)
strm = '%s' % float('%.2g' % m)

plt.figure(0)
plt.loglog(cpus, times, 'k.', markersize = 8.0)
plt.loglog([1, 20], [2000, 2000.0 / 20.0], label = 'Perfect scaling')
plt.loglog(cpus, tfit, 'o--', markersize = 0.0, label = 'Actual scaling: time $\propto$ CPUs^' + strm)
plt.legend()
plt.ylabel("CPU time (s)")
plt.xlabel("Number of CPUs")
plt.title("Scaling of pure a geochemistry simulation")
plt.tight_layout()
plt.savefig("../../../../geochemistry/doc/content/media/geochemistry/geochem_scaling.png")


# Plotting the results of multiple runs of FORGE porous_flow.i with 180851 nodes and 100 timesteps with hypre and boomeramg

cpus = np.array([10, 20, 40, 60, 80, 100, 140, 200])
times = np.array([6253, 3310, 2028, 1216, 960, 830, 606, 470])
logcp = np.log(cpus)
logt = np.log(times)
m, c = np.polyfit(logcp, logt, 1)
tfit = np.exp(m * logcp + c)
strm = '%s' % float('%.2g' % m)
plt.figure(1)
plt.loglog(cpus, times, 'k.', markersize = 8.0)
plt.loglog([10, 200], [6253, 6253 * 10.0 / 200.0], label = 'Perfect scaling from 10 CPUs')
plt.loglog(cpus, tfit, 'o--', markersize = 0.0, label = 'Actual scaling: time $\propto$ CPUs^' + strm)
plt.legend()
plt.ylabel("CPU time (s)")
plt.xlabel("Number of CPUs")
plt.title("Scaling in a reactive-transport simulation")
plt.tight_layout()
plt.savefig("../../../../geochemistry/doc/content/media/geochemistry/reactive_transport_scaling.png")

plt.show()
sys.exit(0)
