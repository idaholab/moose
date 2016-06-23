#!/usr/bin/env python

import os
import sys
import numpy as np
import matplotlib.pyplot as plt

def expected(x):
    ee = 1.2
    nu = 0.3
    t = 0.0002
    ll = 10
    z = 0.5
    c = 0.5
    gg = ee / 2 / (1+nu)
    beta = 3 * t * (1 - nu * nu) / 4 / c / c / c / ee
    dd = - beta * nu / (1 - nu)
    delta = beta / 3
    gamma = -3 * t / 4 / c / gg
    alpha = dd / 3 + t / 4 / c / c / c / gg
    ux = beta * x * z * (2 * ll - x) + alpha * pow(z, 3)
    uz = delta * x * x * (x - 3 * ll) + gamma * x + dd * z * z * (ll - x)
    return (ux, uz)

def expected2(x):
    ee = 1.2
    nu = 0.3
    aa = 1.11E-2
    ll = 10
    z = 0.5
    c = 0.5
    y = 0
    gg = ee / 2 / (1+nu)
    dd = -nu * aa
    ux = aa * x * z
    uy = dd * z * y
    uz = -0.5 * aa * x * x + 0.5 * dd * (z * z - y * y)
    return (ux, uy, uz)

def solid_bar():
    f = open("../../tests/static_deformations/gold/beam_cosserat_01_soln_0001.csv")
    data = [map(float, line.strip().split(",")) for line in f.readlines()[1:12]]
    f.close()
    return data

def solid_bar2():
    f = open("../../tests/static_deformations/gold/beam_cosserat_02_apply_stress_soln_0001.csv")
    data = [map(float, line.strip().split(",")) for line in f.readlines()[1:12]]
    f.close()
    return data



xpoints = np.arange(0, 10.05, 0.1)
moosex = [i for i in range(11)]
moose = solid_bar()
moose2 = solid_bar2()

plt.figure()
plt.plot(xpoints, expected(xpoints)[0], 'k-', linewidth = 1.0, label = 'expected u_x')
plt.plot(xpoints, expected(xpoints)[1], 'r-', linewidth = 1.0, label = 'expected u_z')
plt.plot(moosex, [d[4] for d in moose], 'ks', markersize = 10.0, label = 'MOOSE disp_x')
plt.plot(moosex, [d[5] for d in moose], 'r^', markersize = 10.0, label = 'MOOSE disp_z')
plt.legend(loc = 'lower left')
plt.xlabel("x (m)")
plt.ylabel("displacement (m)")
plt.title("Beam deformation")
plt.savefig("cosserat_beam_disp.pdf")

plt.figure()
plt.plot(xpoints, expected2(xpoints)[0], 'k-', linewidth = 1.0, label = 'expected u_x')
plt.plot(xpoints, expected2(xpoints)[2], 'r-', linewidth = 1.0, label = 'expected u_z')
plt.plot(moosex, [d[9] for d in moose2], 'ks', markersize = 10.0, label = 'MOOSE disp_x')
plt.plot(moosex, [d[11] for d in moose2], 'r^', markersize = 10.0, label = 'MOOSE disp_z')
plt.legend(loc = 'lower left')
plt.xlabel("x (m)")
plt.ylabel("displacement (m)")
plt.title("Beam deformation")
plt.savefig("cosserat_beam_disp_2.pdf")

sys.exit(0)
