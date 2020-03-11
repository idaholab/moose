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
    perm = 1.0E-15
    bulk = 2.0E9
    visc = 1.0E-3
    porosity = 0.1
    dens0 = 1000.0
    pinf = 3.0E6
    pzero = 2.0E6
    t = 1.0E4

    alpha = perm * bulk / visc / porosity
    rhoinf = dens0 * np.exp(pinf / bulk)
    rhozero = dens0 * np.exp(pzero / bulk)

    rho = rhoinf + (rhozero - rhoinf) * erf(x / np.sqrt(4.0 * alpha * t))
    return np.log(rho / dens0) * bulk

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

def pp_1d(fn):
    correct_answer = map(float, "10000,3000000,2601930.2755333,2307719.4747388,2136757.5992144,2054099.8856614,2019434.148209,2006441.9879013,2001996.0817091,2000586.2109877,2000173.4448251,2000086.5298541".split(","))
    check_data(fn, correct_answer)
    return [correct_answer[1 + i] for i in range(11)]

def pp_1d_fully_saturated(fn):
    correct_answer = map(float, "10000,2794616.1618044,2439237.3075785,2204923.546659,2082279.6349733,2029070.5111397,2009223.4025218,2002673.2663601,2000718.1104229,2000182.6134836,2000052.6444801".split(","))
    check_data(fn, correct_answer)
    return [correct_answer[1 + i] for i in range(10)]

def pp_1d_fully_saturated_2(fn):
    correct_answer = map(float, "10000,2795732.9559267,2439991.6439937,2202920.8455393,2078959.1115455,2026416.6558814,2007756.872924,2002037.7891179,2000487.1102026,2000108.1349692,2000026.3605354".split(","))
    check_data(fn, correct_answer)
    return [correct_answer[1 + i] for i in range(10)]

def pp_1d_fully_saturated_3c(fn):
    correct_answer = map(float, "10000,2794613.7550861,2439232.0716679,2204918.7315799,2082276.6376567,2029069.062971,2009222.8198033,2002673.0623698,2000718.0463877,2000182.5948384,2000052.6383607".split(","))
    check_data(fn, correct_answer)
    return [correct_answer[1 + i] for i in range(10)]

def pp_1d_3c(fn):
    correct_answer = map(float, "10000,0.10000057503278,0.29999980834797,3000000,2601931.8259877,2307721.2207858,2136758.588738,2054100.2743483,2019434.2716268,2006442.0223108,2001996.0904859,2000586.2130746,2000173.4453065,2000086.5300463".split(","))
    check_data(fn, correct_answer)
    return [correct_answer[3 + i] for i in range(11)]

def pp_1d_2p(fn):
    correct_answer = map(float, "10000,3000000,2601930.2755333,2307719.4747388,2136757.5992144,2054099.8856614,2019434.148209,2006441.9879013,2001996.0817091,2000586.2109877,2000173.4448251,2000086.5298541".split(","))
    check_data(fn, correct_answer)
    return [correct_answer[1 + i] for i in range(11)]

def pp_1d_md(fn):
    correct_answer = map(float, "10000,3000000.0000801,2601930.2756119,2307719.4748173,2136757.5992928,2054099.8857403,2019434.1482849,2006441.9879799,2001996.0817871,2000586.2110672,2000173.4449043,2000086.5299339".split(","))
    check_data(fn, correct_answer)
    return [correct_answer[1 + i] for i in range(11)]

xpoints = np.arange(0, 101, 1)
moosex = range(0, 110, 10)
moosex5 = range(5, 105, 10)

onephase = pp_1d("../../../../../../test/tests/pressure_pulse/gold/pressure_pulse_1d.csv")
onephase_fully_saturated = pp_1d_fully_saturated("../../../../../../test/tests/pressure_pulse/gold/pressure_pulse_1d_fully_saturated.csv")
onephase_fully_saturated_2 = pp_1d_fully_saturated_2("../../../../../../test/tests/pressure_pulse/gold/pressure_pulse_1d_fully_saturated_2.csv")
onephase_3c_fully_saturated = pp_1d_fully_saturated_3c("../../../../../../test/tests/pressure_pulse/gold/pressure_pulse_1d_3comp_fully_saturated.csv")
threecomp = pp_1d_3c("../../../../../../test/tests/pressure_pulse/gold/pressure_pulse_1d_3comp.csv")
twophase = pp_1d_2p("../../../../../../test/tests/pressure_pulse/gold/pressure_pulse_1d_2phase.csv")
md = pp_1d_md("../../../../../../test/tests/pressure_pulse/gold/pressure_pulse_1d_MD.csv")

plt.figure()
plt.plot(xpoints, expected(xpoints), 'k-', linewidth = 3.0, label = 'expected (single phase)')
plt.plot(moosex, onephase, 'rs', markersize = 10.0, label = 'MOOSE (one component)')
plt.plot(moosex5, onephase_fully_saturated_2, 'bx', markersize = 13.0, label = 'MOOSE (fully saturated)')
plt.plot(moosex5, onephase_3c_fully_saturated, 'k*', markersize = 8.0, label = 'MOOSE (full sat, 3 comp)')
plt.plot(moosex, threecomp, 'b^', label = 'MOOSE (3 component)')
plt.plot(moosex, twophase, 'gx', markersize = 13.0, label = 'MOOSE (two phase)')
plt.plot(moosex, md, 'yo', label = 'MOOSE (one phase, MD formulation)')
plt.legend(loc = 'upper right')
plt.xlabel("x (m)")
plt.ylabel("Porepressure (Pa)")
plt.title("Pressure pulse in 1D")
plt.axis([0, 100, 1.99E6, 3.01E6])
plt.savefig("pressure_pulse_1d.png")

sys.exit(0)
