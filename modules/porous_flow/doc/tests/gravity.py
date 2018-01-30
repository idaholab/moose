#!/usr/bin/env python
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

def expected(x):
    p0 = 0.0
    bulk = 1.2
    dens0 = 1.0
    grav = 1.0
    return -bulk * np.log(np.exp(-p0 / bulk) + grav * dens0 * x / bulk)

def equal_to_tol(a, b):
    return (np.abs(a) < 1.0E-10 and np.abs(b) < 1.0E-10) or ((a - b)/(a + b) < 1.0E-5)

def grav01():
    correct_answer = map(float, "1,0,0.104414341,0.218787535,0.345221573,0.486563338,0.646804304,0.831790506,1.050585819,1.318376416,1.663636579,2.150319764".split(","))
    try:
        f = open("../../tests/gravity/gold/grav01a.csv")
        data = map(float, f.readlines()[-2].strip().split(","))
        f.close()
        for i in range(len(correct_answer)):
            if not equal_to_tol(data[i], correct_answer[i]):
                sys.stderr.write("grav01a.csv is not giving the correct answer\n")
    except:
        sys.stderr.write("Cannot read grav01a.csv, or it contains erroneous data\n")
    return [correct_answer[1 + i] for i in range(11)]

def grav02():
    correct_answer = map(float, "1,1.3862943611199,2.1519655172855,0,0.5,0.51668379282202,0.53366857148455,0.55097652331752,0.56871553628822,0.64308129448691,0.82737790132889,1.0456015672849,1.3131156740017,1.6591951080272,2.1519655259489,0,0.10359283325068,0.21718593392744,0.34287577130845,0.48350241932577,0.6430520336168,0.82737780219107,1.0456015566574,1.3131156695752,1.6591950986579,2.1519655172855".split(","))
    try:
        f = open("../../tests/gravity/gold/grav02d.csv")
        data = map(float, f.readlines()[-2].strip().split(","))
        f.close()
        for i in range(len(correct_answer)):
            if not equal_to_tol(data[i], correct_answer[i]):
                sys.stderr.write("grav02d.csv is not giving the correct answer\n")
    except:
        sys.stderr.write("Cannot read grav02d.csv, or it contains erroneous data\n")
    gas = [correct_answer[4 + i] for i in range(11)]
    water = [correct_answer[15 + i] for i in range(11)]
    return [gas, water]

water1 = grav01()
(gas, water) = grav02()

xpoints = np.arange(-1.02, 0.05, 0.01)
moosex = [-0.1 * i for i in range(11)]

plt.figure()
plt.plot(xpoints, expected(xpoints), 'k-', linewidth = 3.0, label = 'expected (single phase)')
plt.plot(moosex, water1, 'rs', markersize = 10.0, label = 'MOOSE (single phase)')
plt.plot(moosex, water, 'b^', label = 'MOOSE (2phase, heavy phase)')
plt.plot(moosex, gas, 'k--', label = 'MOOSE (2phase, light phase)')
plt.legend(loc = 'upper right')
plt.xlabel("x (m)")
plt.ylabel("Porepressure (Pa)")
plt.title("Gravity head")
plt.savefig("gravity_fig.pdf")

sys.exit(0)
