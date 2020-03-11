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

def expected(ini, res, ini_x, res_x):
    lo2 = 0.5 * (res_x - ini_x)
    alpha = (ini - res) / 4.0 / lo2**3
    beta = -3.0 * alpha * lo2**2
    data = [ini_x + i*(res_x - ini_x)/100 for i in range(100)]
    data = [(x, alpha * (x - ini_x - lo2)**3 + beta * (x - ini_x - lo2) + (ini + res) / 2.0) for x in data]
    return zip(*data)

def moose(fn, i, j):
    f = open(fn)
    data = [line.strip().split(",") for line in f.readlines()[2:-1]]
    data = [(d[i], d[j]) for d in data]
    f.close()
    return zip(*data)


plt.figure()
expect3 = expected(1.0, 0.5, 0.0, 1.0E-5)
expect13 = expected(0.9, 0.5, -1.0E-5, 0.0)
m3 = moose("gold/small_deform_hard3.csv", 2, 4)
m13 = moose("gold/small_deform_hard13.csv", 2, 6)
plt.plot(expect3[0], expect3[1], 'k-', linewidth = 3.0, label = 'expected, tensile')
plt.plot(m3[0], m3[1], 'k^', label = 'MOOSE, tensile')
plt.plot(expect13[0], expect13[1], 'r-', linewidth = 3.0, label = 'expected, compressive')
plt.plot(m13[0], [-float(y) for y in m13[1]], 'ro', label = 'MOOSE, compressive')
plt.legend(loc = 'upper right')
plt.xlabel("internal parameter")
plt.ylabel("Strength")
plt.ticklabel_format(style='sci', axis='x', scilimits=(0,0))
plt.title("Tensile and compressive strengths with softening")
plt.savefig("figures/small_deform_hard_3_13.eps")

sys.exit(0)
