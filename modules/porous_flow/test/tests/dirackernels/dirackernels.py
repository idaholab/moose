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

def bh02_expected(pressure):
    perm = 1.0E-12
    ele_length = 2
    radius = 0.1
    bh_length = 1
    re = 0.28
    r0 = re * np.sqrt(ele_length**2 + ele_length**2) / 2.0
    wc = 2 * np.pi * np.sqrt(perm**2) * bh_length / np.log(r0 / radius)
    density = 1000
    viscosity = 1.0E-3
    return wc * density * pressure / viscosity

def bh02():
    f = open("gold/bh02.csv")
    data = [line.strip().split(",") for line in f.readlines()[1:]]
    f.close()
    data = [list(map(float, line)) for line in data if len(line) > 5]
    pfe = [(data[i][4], data[i][1] / (data[i][0] - data[i - 1][0]), data[i][5]) for i in range(1, len(data))]
    return pfe

def bh03_expected(pressure):
    perm = 1.0E-12
    ele_length = 2
    radius = 0.1
    bh_length = 1
    re = 0.28
    r0 = re * np.sqrt(ele_length**2 + ele_length**2) / 2.0
    wc = 2 * np.pi * np.sqrt(perm**2) * bh_length / np.log(r0 / radius)
    density = 1000
    viscosity = 1.0E-3
    return wc * density * (pressure - 1E7) / viscosity

def bh03():
    f = open("gold/bh03.csv")
    data = [line.strip().split(",") for line in f.readlines()[1:]]
    f.close()
    data = [list(map(float, line)) for line in data if len(line) > 5]
    pfe = [(data[i][4], data[i][1] / (data[i][0] - data[i - 1][0]), data[i][5]) for i in range(1, len(data))]
    return pfe

def bh04_expected(pressure):
    perm = 1.0E-12
    ele_length = 2
    radius = 0.1
    bh_length = 1
    re = 0.28
    r0 = re * np.sqrt(ele_length**2 + ele_length**2) / 2.0
    wc = 2 * np.pi * np.sqrt(perm**2) * bh_length / np.log(r0 / radius)
    alpha = 1.0E-5
    m = 0.8
    n = 2.0
    bottom_p = -1.0E6
    bulk = 2.0E9
    dens0 = 1000
    viscosity = 1.0E-3
    saturation = (1.0 + (- alpha * pressure)**(1.0 / (1.0 - m)))**(- m)
    relperm = (n + 1.0) * saturation**n - n * saturation**(n + 1.0)
    density = dens0 * np.exp(pressure / bulk)
    return wc * density * relperm * (pressure - bottom_p) / viscosity

def bh04():
    f = open("gold/bh04.csv")
    data = [line.strip().split(",") for line in f.readlines()[1:]]
    f.close()
    data = [list(map(float, line)) for line in data if len(line) > 5]
    pfe = [(data[i][4], data[i][1] / (data[i][0] - data[i - 1][0]), data[i][5]) for i in range(1, len(data))]
    return pfe

def bh05_expected(pressure):
    perm = 1.0E-12
    ele_length = 2
    radius = 0.1
    bh_length = 1
    re = 0.28
    r0 = re * np.sqrt(ele_length**2 + ele_length**2) / 2.0
    wc = 2 * np.pi * np.sqrt(perm**2) * bh_length / np.log(r0 / radius)
    alpha = 1.0E-5
    m = 0.8
    n = 2.0
    bottom_p = 0
    bulk = 2.0E9
    dens0 = 1000
    viscosity = 1.0E-3
    saturation = (1.0 + (- alpha * pressure)**(1.0 / (1.0 - m)))**(- m)
    relperm = (n + 1.0) * saturation**n - n * saturation**(n + 1.0)
    density = dens0 * np.exp(pressure / bulk)
    return wc * density * relperm * (pressure - bottom_p) / viscosity

def bh05():
    f = open("gold/bh05.csv")
    data = [line.strip().split(",") for line in f.readlines()[1:]]
    f.close()
    data = [list(map(float, line)) for line in data if len(line) > 5]
    pfe = [(data[i][4], data[i][1] / (data[i][0] - data[i - 1][0]), data[i][5]) for i in range(1, len(data))]
    return pfe

def bh07_expected(r):
    dens0 = 1000.0
    bulk = 2.0E9
    P_bh = 0
    rho_bh = dens0 * np.exp(P_bh / bulk)
    P_R = 1.0E7
    rho_R = dens0 * np.exp(P_R / bulk)
    r_bh = 1.0
    outer_r = 300
    rho = rho_bh + (rho_R - rho_bh) * np.log(r / r_bh) / np.log(outer_r / r_bh)
    return bulk * np.log(rho / dens0)

def bh07():
    f = open("gold/bh07_csv_pp_0003.csv")
    data = [line.strip().split(",") for line in f.readlines()[1:]]
    f.close()
    data = [list(map(float, line)) for line in data if len(line) > 3]
    xp = [(data[i][2], data[i][1]) for i in range(0, len(data), 10)]
    return xp


ppoints = np.arange(0, 1.01E7, 1E6)
bh02 = bh02()

plt.figure()
plt.plot(ppoints/1E6, bh02_expected(ppoints), 'k-', linewidth = 3.0, label = 'expected')
plt.plot([x[0]/1E6 for x in bh02], [x[1] for x in bh02], 'rs', markersize = 10.0, label = 'MOOSE')
plt.legend(loc = 'lower right')
plt.xlabel("Porepressure (MPa)")
plt.ylabel("flow rate (kg/s)")
plt.title("Fully-saturated production well: flow")
plt.savefig("bh02_flow.png")

plt.figure()
plt.plot([x[0]/1E6 for x in bh02], [x[2]*1E15 for x in bh02], 'rs', markersize = 10.0, label = 'MOOSE')
plt.xlabel("Porepressure (MPa)")
plt.ylabel("Mass-balance error (units 1E-15)")
plt.title("Fully-saturated production well: mass-balance error")
plt.savefig("bh02_error.png")

ppoints = np.arange(0, 1.01E7, 1E6)
bh03 = bh03()

plt.figure()
plt.plot(ppoints/1E6, bh03_expected(ppoints), 'k-', linewidth = 3.0, label = 'expected')
plt.plot([x[0]/1E6 for x in bh03], [x[1] for x in bh03], 'rs', markersize = 10.0, label = 'MOOSE')
plt.legend(loc = 'lower right')
plt.xlabel("Porepressure (MPa)")
plt.ylabel("flow rate (kg/s)")
plt.title("Fully-saturated injection well: flow")
plt.savefig("bh03_flow.png")

plt.figure()
plt.plot([x[0]/1E6 for x in bh03], [x[2]*1E15 for x in bh03], 'rs', markersize = 10.0, label = 'MOOSE')
plt.xlabel("Porepressure (MPa)")
plt.ylabel("Mass-balance error (units 1E-15)")
plt.title("Fully-saturated injection well: mass-balance error")
plt.savefig("bh03_error.png")

ppoints = np.arange(-2.0E5, 0, 1E3)
bh04 = bh04()

plt.figure()
plt.plot(ppoints/1E3, bh04_expected(ppoints), 'k-', linewidth = 3.0, label = 'expected')
plt.plot([x[0]/1E3 for x in bh04], [x[1] for x in bh04], 'rs', markersize = 10.0, label = 'MOOSE')
plt.legend(loc = 'lower right')
plt.xlabel("Porepressure (kPa)")
plt.ylabel("flow rate (kg/s)")
plt.title("Unsaturated production well: flow")
plt.savefig("bh04_flow.png")

plt.figure()
plt.plot([x[0]/1E3 for x in bh04], [x[2]*1E13 for x in bh04], 'rs', markersize = 10.0, label = 'MOOSE')
plt.xlabel("Porepressure (kPa)")
plt.ylabel("Mass-balance error (units 1E-13)")
plt.title("Unsaturated production well: mass-balance error")
plt.savefig("bh04_error.png")

ppoints = np.arange(-2.0E5, 0, 1E3)
bh05 = bh05()

plt.figure()
plt.plot(ppoints/1E3, bh05_expected(ppoints), 'k-', linewidth = 3.0, label = 'expected')
plt.plot([x[0]/1E3 for x in bh05], [x[1] for x in bh05], 'rs', markersize = 10.0, label = 'MOOSE')
plt.legend(loc = 'lower right')
plt.xlabel("Porepressure (kPa)")
plt.ylabel("flow rate (kg/s)")
plt.title("Unsaturated injection well: flow")
plt.savefig("bh05_flow.png")

plt.figure()
plt.plot([x[0]/1E3 for x in bh05], [x[2]*1E10 for x in bh05], 'rs', markersize = 10.0, label = 'MOOSE')
plt.xlabel("Porepressure (kPa)")
plt.ylabel("Mass-balance error (units 1E-10)")
plt.title("Unsaturated injection well: mass-balance error")
plt.savefig("bh05_error.png")

rpoints = np.arange(1, 301, 3)
bh07 = bh07()

plt.figure()
plt.plot(rpoints, bh07_expected(rpoints)/1E6, 'k-', linewidth = 3.0, label = 'expected')
plt.plot([x[0] for x in bh07], [x[1]/1E6 for x in bh07], 'rs', markersize = 10.0, label = 'MOOSE')
plt.legend(loc = 'lower right')
plt.xlabel("radius (m)")
plt.ylabel("Porepressure (MPa)")
plt.title("Steadystate porepressure distribution due to production borehole")
plt.savefig("bh07.png")

sys.exit(0)
