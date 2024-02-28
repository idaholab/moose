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
import numpy as np
import matplotlib.pyplot as plt

def expected(fn):
    f = open(fn, "r")
    data = sorted([map(float, line.strip().split(",")[3:6]) for line in f.readlines()[1:]])
    data = [[d[0], d[1], d[2]] for d in data if d[0] >= d[1] and d[1] >= d[2]]
    mean = [(d[0] + d[1] + d[2]) / 3.0 for d in data]
    bar = [np.sqrt(0.5 * ((data[i][0] - mean[i])**2 + (data[i][1] - mean[i])**2 + (data[i][2] - mean[i])**2)) for i in range(len(data))]
    sortit = sorted([[mean[i], bar[i]] for i in range(len(data))], key = lambda x: x[1])
    return zip(*sortit)

def moose(fn):
    f = open(fn, "r")
    data = [map(float, line.strip().split(",")[3:9]) for line in f.readlines()[2:-1]]
    data = [[d[0], d[3], d[5]] for d in data]
    mean = [(d[0] + d[1] + d[2]) / 3.0 for d in data]
    bar = [np.sqrt(0.5 * ((data[i][0] - mean[i])**2 + (data[i][1] - mean[i])**2 + (data[i][2] - mean[i])**2)) for i in range(len(data))]
    return (mean, bar)

plt.figure()
e23 = expected("gold/expected_small_deform_23.csv")
m23 = moose("gold/small_deform23.csv")
e24 = expected("gold/expected_small_deform_24.csv")
m24 = moose("gold/small_deform24.csv")
plt.plot(e23[0], e23[1], 'r-', linewidth = 1.0, label = 'expected')
plt.plot(m23[0], m23[1], 'rs', markersize = 2.0, label = 'MOOSE, Lode=30deg')
plt.plot(e24[0], e24[1], 'b-', linewidth = 1.0, label = 'expected')
plt.plot(m24[0], m24[1], 'bo', markersize = 2.0, label = 'MOOSE, Lode=-30deg')
plt.legend(loc = 'upper right')
plt.xlabel("mean stress")
plt.ylabel("bar stress")
plt.title("Mohr-Coulomb yield surface on meridional plane")
plt.savefig("figures/small_deform_23_24.eps")

sys.exit(0)
