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
    data = [map(float, line.strip().split(",")[mi:ma]) for line in f.readlines()[2:-1]]
    return zip(*data)

plt.figure()
e5 = expected("gold/expected_small_deform_5.csv", 3, 5)
e6 = expected("gold/expected_small_deform_6.csv", 3, 5)
e7 = expected("gold/expected_small_deform_7.csv", 4, 6)
m5 = moose("gold/small_deform5.csv", 2, 4)
m6 = moose("gold/small_deform6.csv", 2, 4)
m7 = moose("gold/small_deform7.csv", 3, 5)
plt.plot(e5[1], e5[0], 'k-', linewidth = 3.0, label = 'expected (Smin=0)')
plt.plot(m5[1], m5[0], 'ks', label = 'MOOSE (Smin=0')
plt.plot(e6[1], e6[0], 'b-', linewidth = 3.0, label = 'expected (Smid = Smin)')
plt.plot(m6[1], m6[0], 'b^', label = 'MOOSE (Smid approx Smin)')
plt.plot(e7[1], e7[0], 'r-', linewidth = 3.0, label = 'expected (Smax = Smid)')
plt.plot(m7[1], m7[0], 'ro', label = 'MOOSE (Smax = Smid)')
plt.xlim([0,1])
plt.legend(loc = 'lower left')
plt.xlabel("S_mid or S_min")
plt.ylabel("S_max")
plt.title("Tensile yield surface")
plt.savefig("figures/small_deform_5_6_7.eps")

sys.exit(0)
