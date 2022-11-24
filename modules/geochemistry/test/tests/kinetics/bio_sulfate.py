#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

# Plotting the results of bio_sulfate.i

import os
import sys
import math
import matplotlib.pyplot as plt


f = open(os.path.join("gold", "bio_sulfate_1_21_out.csv"), "r")
data = [list(map(float, line.strip().split(","))) for line in f.readlines()[2:]]
f.close()
tim1 = [x[0] for x in data]
biomass1 = [x[1] for x in data]
mmoles_acetate1 = [x[2] * 1000 for x in data]

f = open(os.path.join("gold", "bio_sulfate_2_21_out.csv"), "r")
data = [list(map(float, line.strip().split(","))) for line in f.readlines()[2:]]
f.close()
tim2 = [x[0] for x in data]
biomass2 = [x[1] for x in data]
mmoles_acetate2 = [x[2] * 1000 for x in data]

fig, ax = plt.subplots()
ax.plot(tim1, biomass1, 'r--', label = 'Biomass, method 1')
ax.plot(tim2, biomass2, 'r-', label = 'Biomass, method 2')
ax.set_ylabel("Biomass (mg/kg)", color = "red")
ax.grid(axis = 'x')
ax.tick_params(axis = 'y', colors = 'red')
ax.spines['left'].set_color('red')

ax2 = ax.twinx()
ax2.plot(tim1, mmoles_acetate1, 'b--', label = 'Acetate, method 1')
ax2.plot(tim2, mmoles_acetate2, 'b-', label = 'Acetate, method 2')
ax2.set_ylabel("Bulk acetate (mmole)", color = 'blue')
ax2.tick_params(axis = 'y', colors = 'blue')
ax2.spines['right'].set_color('blue')
ax2.spines['left'].set_color('red')

ax2.set_xlabel("Time (days)")
fig.legend(loc="upper right", bbox_to_anchor=(1, 0.6), bbox_transform=ax.transAxes)
plt.xticks([0, 7, 14, 21])
plt.xlim([0, 21])
plt.title("Microbial sulfate reduction")
plt.savefig("../../../doc/content/media/geochemistry/bio_sulfate.png")
plt.show()
sys.exit(0)
