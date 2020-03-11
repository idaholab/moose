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

def expected(fn, mi, ma):
    f = open(fn, "r")
    data = sorted([map(float, line.strip().split(",")[mi:ma]) for line in f.readlines()[1:]])
    data = [d for d in data if d[0] >= d[1]]
    return zip(*data)

def moose(fn, mi, ma):
    f = open(fn, "r")
    data = [map(float, line.strip().split(",")[mi:ma]) for line in f.readlines()[3:-1]]
    return zip(*data)

plt.figure()
e15 = expected("gold/expected_small_deform_15.csv", 4, 6)
e16 = expected("gold/expected_small_deform_16.csv", 4, 6)
e17 = expected("gold/expected_small_deform_17.csv", 3, 5)
m15 = moose("gold/small_deform15.csv", 3, 5)
m16 = moose("gold/small_deform16.csv", 3, 5)
m17 = moose("gold/small_deform17.csv", 2, 4)
plt.plot(e15[0], e15[1], 'k-', linewidth = 3.0, label = 'expected (Smax=0)')
plt.plot(m15[0], m15[1], 'ks', label = 'MOOSE (Smax=0)')
plt.plot(e16[0], e16[1], 'r-', linewidth = 3.0, label = 'expected (Smax = Smid)')
plt.plot(m16[0], m16[1], 'r^', label = 'MOOSE (Smax approx Smid)')
plt.plot(e17[0], e17[1], 'b-', linewidth = 3.0, label = 'expected (Smid = Smin)')
plt.plot(m17[0], m17[1], 'bo', label = 'MOOSE (Smid = Smin)')
plt.xlim([-1,0])
plt.gca().invert_yaxis()
plt.gca().invert_xaxis()
plt.legend(loc = 'lower left')
plt.xlabel("Smid or Smax")
plt.ylabel("Smin")
plt.title("Compressive yield surface")
plt.savefig("figures/small_deform_15_16_17.eps")

sys.exit(0)
