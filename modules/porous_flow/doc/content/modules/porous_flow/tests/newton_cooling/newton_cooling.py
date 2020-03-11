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

def nc04():
    f = open("../../../../../../test/tests/newton_cooling/gold/nc04_temp_0001.csv")
    data = [map(float, line.strip().split(",")) for line in f.readlines()[1:] if line.strip()]
    t = [d[1] for d in data]
    x = [d[2] for d in data]
    return (x, t)

def expected_nc04(x):
    lam = 100.0
    cc = 1.0
    ll = 100.0
    t0 = 2.0
    te = 1.0
    return t0 + (te - t0) * cc * x / (lam + cc * ll)

def nc08_pp():
    f = open("../../../../../../test/tests/newton_cooling/gold/nc08_porepressure_0001.csv")
    data = [map(float, line.strip().split(",")) for line in f.readlines()[1:] if line.strip()]
    p = [d[1] for d in data]
    x = [d[2] for d in data]
    return (x, p)

def nc08_tt():
    f = open("../../../../../../test/tests/newton_cooling/gold/nc08_temperature_0001.csv")
    data = [map(float, line.strip().split(",")) for line in f.readlines()[1:] if line.strip()]
    t = [d[1] for d in data]
    x = [d[2] for d in data]
    return (x, t)

def expected_nc08_pp(x):
    ll = 100.0
    p0 = 200.0
    return p0 * np.sqrt(1.0 - x / 2 / ll)

def expected_nc08_tt(x):
    return 180.0 + 0.0 * x

plt.figure(0)
xpoints = np.arange(0, 101, 1)
plt.plot(xpoints, expected_nc04(xpoints), 'b-', linewidth = 2.0, label = 'Expected')
plt.plot(nc04()[0], nc04()[1], 'bo', markersize = 6.0, label = 'MOOSE')
plt.legend(loc = 'best')
plt.xlabel("x (m)")
plt.ylabel("Temperature (K)")
plt.title("Temperature in a bar with sink flux")
plt.savefig("nc04.png")

plt.figure(1)
xpoints = np.arange(0, 100.01, 0.01)
plt.plot(xpoints, expected_nc08_pp(xpoints), 'b-', linewidth = 2.0, label = 'Expected, P')
plt.plot(nc08_pp()[0], nc08_pp()[1], 'bo', markersize = 6.0, label = 'MOOSE, P')
plt.plot(xpoints, expected_nc08_tt(xpoints), 'r-', linewidth = 2.0, label = 'Expected, T')
plt.plot(nc08_tt()[0], nc08_tt()[1], 'ro', markersize = 6.0, label = 'MOOSE, T')
plt.legend(loc = 'best')
plt.xlabel("x (m)")
plt.ylabel("Temperature (K) and Porepressure (Pa)")
plt.title("Temperature and gas porepressure in a bar with sink flux")
#plt.axis([0, 100, 0, 6])
plt.savefig("nc08.png")
