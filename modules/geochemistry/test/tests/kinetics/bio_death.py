#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

# Plotting the results of bio_death.i

import os
import sys
import math
import matplotlib.pyplot as plt

f = open("gold/bio_death_small_dt_out.csv", "r")
data = [list(map(float, line.strip().split(","))) for line in f.readlines()[2:]]
f.close()
tim = [0] + [x[0] for x in data]
biomass = [1] + [x[1] for x in data]

plt.figure(0)
plt.scatter(tim, biomass, label = 'MOOSE')
plt.plot([0.1 * i for i in range(100)], [math.exp(-0.5 * 0.1 * i) for i in range(100)], 'k', label = 'Exponential')
plt.legend()
plt.grid()
plt.xlabel("Time")
plt.ylabel("Moles of microbe")
plt.title("Microbe mortality")
plt.savefig("../../../doc/content/media/geochemistry/bio_death.png")
plt.show()
sys.exit(0)
