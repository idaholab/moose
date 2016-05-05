#!/usr/bin/env python

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

onephase = pp_1d("../../tests/tests/pressure_pulse/gold/pressure_pulse_1d.csv")
threecomp = pp_1d_3c("../../tests/tests/pressure_pulse/gold/pressure_pulse_1d_3comp.csv")
twophase = pp_1d_2p("../../tests/tests/pressure_pulse/gold/pressure_pulse_1d_2phase.csv")
md = pp_1d_md("../../tests/tests/pressure_pulse/gold/pressure_pulse_1d_MD.csv")

plt.figure()
plt.plot(xpoints, expected(xpoints), 'k-', linewidth = 3.0, label = 'expected (single phase)')
plt.plot(moosex, onephase, 'rs', markersize = 10.0, label = 'MOOSE (one component)')
plt.plot(moosex, threecomp, 'b^', label = 'MOOSE (3 component)')
plt.plot(moosex, twophase, 'gx', markersize = 13.0, label = 'MOOSE (two phase)')
plt.plot(moosex, md, 'yo', label = 'MOOSE (one phase, MD formulation)')
plt.legend(loc = 'upper right')
plt.xlabel("x (m)")
plt.ylabel("Porepressure (Pa)")
plt.title("Pressure pulse in 1D")
plt.axis([0, 100, 1.99E6, 3.01E6])
plt.savefig("pressure_pulse_1d.pdf")

sys.exit(0)
