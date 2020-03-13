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

def minmod(x):
    return np.minimum(1, x)

def vanleer(x):
    return 2 * np.abs(x) / (1 + np.abs(x))

def mc(x):
    return np.minimum(0.5 * np.abs(1 + x), np.minimum(2, 2 * np.abs(x)))

def superbee(x):
    return np.maximum(np.minimum(2, np.abs(x)), np.minimum(1, 2 * np.abs(x)))

xpoints = np.arange(0, 3.5, 0.01)

plt.figure()

plt.plot(xpoints, minmod(xpoints), 'k-', linewidth = 2.0, label = 'MinMod')
plt.plot(xpoints, vanleer(xpoints), 'r-', linewidth = 2.0, label = 'VanLeer')
plt.plot(xpoints, mc(xpoints), 'g-', linewidth = 2.0, label = 'MC')
plt.plot(xpoints, superbee(xpoints), 'b-', linewidth = 2.0, label = 'superbee')
plt.legend(loc = 'best')
plt.xlabel("r")
plt.ylabel("Phi(r)")
plt.grid()
plt.title("Flux limiters")
plt.savefig("flux_limiters.png")

sys.exit(0)
