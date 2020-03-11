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
    data = sorted([map(float, line.strip().split(",")[3:5]) for line in f.readlines()[1:]])
    data = [d for d in data if d[0] >= d[1]]
    return zip(*data)

def moose(fn):
    f = open(fn, "r")
    data = [map(float, line.strip().split(",")[2:4]) for line in f.readlines()[2:-1]]
    return zip(*data)

plt.figure()
e5 = expected("gold/expected_small_deform_5_update.csv")
e6 = expected("gold/expected_small_deform_6_update.csv")
m5 = moose("gold/small_deform5_update_version.csv")
m6 = moose("gold/small_deform6_update_version.csv")
plt.plot(e5[0], e5[1], 'k-', linewidth = 3.0, label = 'expected (s_III=0)')
plt.plot(m5[0], m5[1], 'ks', label = 'MOOSE (s_III=0')
plt.plot(e6[0], e6[1], 'r-', linewidth = 3.0, label = 'expected (s_III = S_II)')
plt.plot(m6[0], m6[1], 'r^', label = 'MOOSE (s_III approx S_II)')
plt.ylim([0,1])
plt.legend(loc = 'lower left')
plt.xlabel("s_I")
plt.ylabel("s_II")
plt.title("Tensile yield surface")
plt.savefig("small_deform_5_6.pdf")

sys.exit(0)
