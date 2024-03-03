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

def moose(fn):
    coh = 10
    f = open(fn)
    data = [map(float, line.strip().split(",")) for line in f.readlines()[6:-1]]
    f.close()
    intnl = [d[2] for d in data]
    plus_minus = [(0.5 * (d[5] - d[7]), 0.5 * (d[5] + d[7])) for d in data] # (Smax-Smin)/2, (Smax+Smin)/2
    phi = [2.0 * np.arctan((np.sqrt(-pm[0]**2 + pm[1]**2 + coh**2) - pm[1]) / (pm[0] + coh)) for pm in plus_minus]
    return (intnl, phi)


plt.figure()
expect21 = expected(0.174, 0.524, 0.0, 4E-6)
m21 = moose("gold/small_deform_hard22.csv")
plt.plot(expect21[0], expect21[1], 'k-', linewidth = 3.0, label = 'expected')
plt.plot(m21[0], m21[1], 'k^', label = 'MOOSE')
plt.legend(loc = 'lower right')
plt.xlabel("internal parameter")
plt.ylabel("Friction angle")
plt.ticklabel_format(style='sci', axis='x', scilimits=(0,0))
plt.title("Friction-angle hardening")
plt.savefig("figures/small_deform_hard_22.eps")

sys.exit(0)
