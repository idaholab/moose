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

def expected():
    ini = 1.0
    res = 0.5
    lim = 1E-5
    lo2 = 0.5 * lim
    alpha = (ini - res) / 4.0 / lo2**3
    beta = -3.0 * alpha * lo2**2
    data = [i*1E-5/100 for i in range(100)]
    data = [(x, alpha * (x - lo2)**3 + beta * (x - lo2) + (ini + res) / 2.0) for x in data]
    return zip(*data)

def moose():
    f = open("gold/small_deform_hard3_update_version.csv")
    data = [line.strip().split(",") for line in f.readlines()[2:-1]]
    data = [(d[2], d[4]) for d in data]
    f.close()
    return zip(*data)


plt.figure()
expect = expected()
m = moose()
plt.plot(expect[0], expect[1], 'k-', linewidth = 3.0, label = 'expected')
plt.plot(m[0], m[1], 'k^', label = 'MOOSE')
plt.legend(loc = 'upper right')
plt.xlabel("internal parameter")
plt.ylabel("Tensile strength")
plt.title("Tensile yield with softening")
plt.savefig("small_deform_hard3.pdf")

sys.exit(0)
