#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

# Plotting the results of the two geotes_2D simulations

import os
import sys
import matplotlib.pyplot as plt

f = open("gold/exchanger_out.csv", "r")
data = [list(map(float, line.strip().split(","))) for line in f.readlines()[1:]]
f.close()
time = [x[0] / 24.0 / 3600.0 for x in data]
quartzlike = [x[1] for x in data]

f = open("gold/exchanger_un_quartz_out.csv", "r")
data = [list(map(float, line.strip().split(","))) for line in f.readlines()[1:]]
f.close()
quartzunlike = [x[1] for x in data]

plt.figure()
plt.plot(time, quartzlike, 'k-', linewidth = 2.0, label = 'QuartzLike')
plt.plot(time, quartzunlike, 'r-', linewidth = 2.0, label = 'QuartzUnlike')
plt.legend()
plt.ylabel("Precipitate (moles)")
plt.xlabel("Time (days)")
plt.title("Minerals precipitated in the heat exchanger");
plt.savefig("../../doc/content/media/geochemistry/geotes_2D.png")

sys.exit(0)
