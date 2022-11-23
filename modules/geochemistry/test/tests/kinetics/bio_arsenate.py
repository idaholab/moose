#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

# Plotting the results of bio_arsenate1.i

import os
import sys
import math
import matplotlib.pyplot as plt


f = open(os.path.join("gold", "bio_arsenate1_shortdt_out.csv"), "r")
data = [list(map(float, line.strip().split(","))) for line in f.readlines()[2:]]
f.close()
tim = [x[0] for x in data]
biomass = [x[1] * 1000 for x in data]
reaction_rate = [x[3] * 1E9 / 1000.0 / 24.0 / 3600.0 for x in data]

fig, ax = plt.subplots()
ax.plot(tim, biomass, 'r--', label = 'Biomass')
ax.set_ylabel("Biomass (mg/kg)", color = "red")
ax.grid(axis = 'x')
ax.tick_params(axis = 'y', colors = 'red')
ax.spines['left'].set_color('red')

ax2 = ax.twinx()
ax2.plot(tim, reaction_rate, 'b--', label = 'Reaction rate')
ax2.set_ylabel("Reaction rate (nmol/kg/s)", color = 'blue')
ax2.tick_params(axis = 'y', colors = 'blue')
ax2.spines['right'].set_color('blue')
ax2.spines['left'].set_color('red')

ax2.set_xlabel("Time (days)")
ax.set_xlabel("Time (days)")
plt.xticks([0, 0.5, 1, 1.5, 2])
plt.xlim([0, 2])
plt.title("Microbial arsenate reduction")
plt.savefig("../../../doc/content/media/geochemistry/bio_arsenate.png")
plt.show()
sys.exit(0)
