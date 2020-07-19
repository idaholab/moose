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

f = open("gold/kinetic_albite_out.csv", "r")
data = [list(map(float, line.strip().split(","))) for line in f.readlines()[1:]]
f.close()
tim = [x[0] for x in data]
dis = [x[1] * 1000 for x in data]

gwb_tim = [0, 6, 12, 18, 24, 30]
gwb_dis = [0, -2.578, -5.150, -7.714, -10.27, -12.82]
plt.figure(0)
plt.plot(tim, dis, 'k-', linewidth = 2.0, label = 'MOOSE')
plt.plot(gwb_tim, gwb_dis, 'ks', linewidth = 2.0, label = 'GWB')
plt.grid()
plt.legend()
plt.xlabel("Time (days)")
plt.ylabel("Albite change (mmol)")
plt.title("Kinetic dissolution of albite")
plt.savefig("../../../doc/content/media/geochemistry/kinetic_albite.png")
sys.exit(0)
