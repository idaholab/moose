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
    data = [d for d in data if (d[0] >= d[1] and d[1] >= d[2])] # physical region only
    bar = [np.sqrt(0.5 * (d[0]**2 + d[1]**2 + d[2]**2)) for d in data]
    third_inv = [d[0] * d[1] * d[2] for d in data]
    lode = [np.arcsin(-1.5 * np.sqrt(3) * third_inv[i] / bar[i]**3) / 3.0 for i in range(len(data))]
    # extend data to non-physical region for clearer visualisation
    theta_r = [[lode[i], bar[i]] for i in range(len(data))] + [[np.pi / 3.0 - lode[i], bar[i]] for i in range(len(data))]
    theta_r = sorted([[d[0], d[1]] for d in theta_r] + [[d[0] + 2.0 * np.pi / 3.0, d[1]] for d in theta_r] + [[d[0] - 2.0 * np.pi / 3.0, d[1]] for d in theta_r])
    x = [d[1] * np.cos(d[0]) for d in theta_r]
    y = [d[1] * np.sin(d[0]) for d in theta_r]
    return (x, y)

def moose(fn):
    f = open(fn, "r")
    data = [map(float, line.strip().split(",")[4:10]) for line in f.readlines()[3:-1]]
    bar = [np.sqrt(0.5 * (d[0]**2 + 2*d[1]**2 + 2*d[2]**2 + d[3]**2 + 2*d[4]**2 + d[5]**2)) for d in data]
    third_inv = [d[0] * d[3] * d[5] for d in data]
    lode = [np.arcsin(-1.5 * np.sqrt(3) * third_inv[i] / bar[i]**3) / 3.0 for i in range(len(data))]
    x = [bar[i] * np.cos(lode[i]) for i in range(len(data))]
    y = [bar[i] * np.sin(lode[i]) for i in range(len(data))]
    return (x, y)

plt.figure()
e21 = expected("gold/expected_small_deform_21.csv")
m21 = moose("gold/small_deform21.csv")
plt.plot(e21[0], e21[1], 'k-', linewidth = 1.0, label = 'expected')
plt.plot(m21[0], m21[1], 'ks', markersize = 2.0, label = 'MOOSE')
plt.legend(loc = 'lower right')
plt.axes().set_aspect('equal')
plt.title("Mohr-Coulomb yield surface on octahedral plane")
plt.savefig("figures/small_deform_21.eps")

sys.exit(0)
