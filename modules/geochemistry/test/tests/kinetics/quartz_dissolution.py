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

f = open("gold/quartz_dissolution_out.csv", "r")
data = [list(map(float, line.strip().split(","))) for line in f.readlines()[1:]]
f.close()
tim = [x[0] for x in data]
dis = [-x[1] * 1000 for x in data]

gwb_tim = [0, 0.5, 1, 1.5, 2, 3, 4, 5]
gwb_dis = [0, -0.333, -0.5275, -0.6414, -0.7081, -0.77, -0.7912, -0.7985]
plt.figure(0)
plt.plot(tim, dis, 'k-', linewidth = 2.0, label = 'MOOSE')
plt.plot(gwb_tim, gwb_dis, 'ks', linewidth = 2.0, label = 'GWB')
plt.legend()
plt.grid()
plt.xlabel("Time (days)")
plt.ylabel("Quartz change (mmol)")
plt.title("Kinetic dissolution of quartz")
plt.savefig("../../../doc/content/media/geochemistry/quartz_dissolution.png")
sys.exit(0)
