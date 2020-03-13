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
from scipy.special import erf
import matplotlib.pyplot as plt

def expected(x):
    cond = 2.2
    porosity = 0.1
    rock_density = 0.5
    rock_specific_heat_cap = 2.2
    tinf = 300
    tzero = 200
    t = 1.0E2

    alpha = cond / (1.0 - porosity) / rock_density / rock_specific_heat_cap
    temp = tinf + (tzero - tinf) * erf(x / np.sqrt(4.0 * alpha * t))
    return temp

def equal_to_tol(a, b):
    return (np.abs(a) < 1.0 and np.abs(b) < 1.0) or ((a - b)/(a + b) < 1.0E-5)

def check_data(fn, correct_answer):
    try:
        f = open(fn, 'r')
        data = map(float, f.readlines()[-2].strip().split(","))
        f.close()
        for i in range(len(correct_answer)):
            if not equal_to_tol(data[i], correct_answer[i]):
                sys.stderr.write(fn + "is not giving the correct answer\n")
    except:
        sys.stderr.write("Cannot read " + fn + ", or it contains erroneous data\n")

def no_fluid(fn):
    correct_answer = map(float, "100,300,262.05620419362,233.16444061125,215.5562816469,206.53657081398,202.5048010279,200.88831039136,200.29514030239,200.09319946286,200.02989431659,200.0158846078".split(","))
    check_data(fn, correct_answer)
    return [correct_answer[1 + i] for i in range(11)]

def twoph(fn):
    correct_answer = map(float, "100,300,262.05620419362,233.16444061125,215.5562816469,206.53657081398,202.5048010279,200.88831039136,200.29514030239,200.09319946286,200.02989431659,200.0158846078".split(","))
    check_data(fn, correct_answer)
    return [correct_answer[1 + i] for i in range(11)]

xpoints = np.arange(0, 101, 1)
moosex = range(0, 110, 10)

zero_phase = no_fluid("../../../../../../test/tests/heat_conduction/gold/no_fluid.csv")
two_phase = twoph(    "../../../../../../test/tests/heat_conduction/gold/two_phase.csv")

plt.figure()
plt.plot(xpoints, expected(xpoints), 'k-', linewidth = 3.0, label = 'expected')
plt.plot(moosex, zero_phase, 'rs', markersize = 10.0, label = 'MOOSE (no fluid)')
plt.plot(moosex, two_phase, 'b^', label = 'MOOSE (2 phase)')
plt.legend(loc = 'upper right')
plt.xlabel("x (m)")
plt.ylabel("Temperature (K)")
plt.title("Heat conduction in 1D")
plt.axis([0, 100, 199, 301])
plt.savefig("heat_conduction_1d.png")

sys.exit(0)
