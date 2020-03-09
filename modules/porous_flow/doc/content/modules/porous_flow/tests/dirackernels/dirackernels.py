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
from scipy.special import expi
import matplotlib.pyplot as plt

def squarepulse1_expected():
    initial_mass = 202.01003341683
    t = np.array([0, 100, 300, 600, 1400, 1500, 2000])
    m = np.array([initial_mass, initial_mass, initial_mass - 0.1 * 200, initial_mass - 0.1 * 200, initial_mass - 0.1 * 1000, initial_mass - 0.1 * 1000, initial_mass])
    return (t, m)

def squarepulse1():
    f = open("../../../../../../test/tests/dirackernels/gold/squarepulse1.csv")
    data = [map(float, line.strip().split(",")) for line in f.readlines()[1:] if line.strip()]
    f.close()
    return ([d[0] for d in data], [d[1] for d in data])


plt.figure(0)
plt.plot(squarepulse1_expected()[0], squarepulse1_expected()[1], 'k-', linewidth = 3.0, label = 'expected')
plt.plot(squarepulse1()[0], squarepulse1()[1], 'rs', markersize = 10.0, label = 'MOOSE')
plt.legend(loc = 'best')
plt.xlabel("time (s)")
plt.ylabel("Fluid mass (kg)")
plt.title("Square-pulsed point source")
plt.savefig("squarepulse1.png")

def theis_expected(r, t):
    P0 = 20.0E6
    q = 0.16E-3
    viscosity = 0.001
    fluid_bulk = 2.0E9
    porosity = 0.05
    permeability = 1.0E-14
    reciprocal_biot_modulus = porosity / fluid_bulk # Biot coefficient is assumed to be unity
    conductivity = permeability / viscosity
    alpha = conductivity / reciprocal_biot_modulus
    return P0 + q / (4 * np.pi * conductivity) * expi(- r * r / (4 * alpha * t))

def theis():
    f = open("../../../../../../test/tests/dirackernels/gold/theis_rz_csv_pp_0006.csv")
    data = [map(float, line.strip().split(",")) for line in f.readlines()[1:] if line.strip()]
    f.close()
    return ([d[0] for d in data], [d[4] for d in data])

plt.figure(1)
rpoints = np.arange(0.5, 100.5, 0.5)
plt.plot(rpoints, theis_expected(rpoints, 1000.0), 'k-', linewidth = 3.0, label = 'expected')
plt.plot(theis()[0], theis()[1], 'rs', markersize = 8.0, label = 'MOOSE')
plt.legend(loc = 'best')
plt.xlabel("r (m)")
plt.ylabel("Porepressure (Pa)")
plt.title("Theis pumping test")
plt.savefig("theis.png")


# The two phase radial injection problem has a similarity solution (r^2/t)
#
# Read MOOSE simulation data for constant time (tdata) and constant
# radial distance (rdata)
tdata = np.genfromtxt('../../../../../../test/tests/dirackernels/gold/theis3_line_0011.csv', delimiter = ',', names = True, dtype = float)
rdata = np.genfromtxt('../../../../../../test/tests/dirackernels/gold/theis3.csv', delimiter = ',', names = True, dtype = float)

# Distance where data is sampled as a function of time
r = 4
# Time where data is sampled along the spatial dimension
t = 1e4

plt.figure(2)
fig, axes = plt.subplots(1, 2, figsize = (15, 4))
# Water pressure vs similarity solution
axes[0].plot(tdata['x']**2 / t, tdata['ppwater'] * 1e-6, label = 'Fixed $t$')
axes[0].plot(r**2 / rdata['time'], rdata['ppwater'] * 1e-6, 'o', label = 'Fixed $r$')
axes[0].set_xscale('log')
axes[0].set_xlim([5e-4, 5e1])
axes[0].set_xlabel('$\zeta = r^2/t$')
axes[0].set_ylabel('Liquid pressure (MPa)')
axes[0].legend()
# Gas saturation vs similarity solution
axes[1].plot(tdata['x']**2 / t, tdata['sgas'], label = 'Fixed $t$')
axes[1].plot(r**2 / rdata['time'], rdata['sgas'], 'o', label = 'Fixed $r$')
axes[1].set_xscale('log')
axes[1].set_xlim([5e-4, 5e1])
axes[1].set_ylim([-0.1, 1.1])
axes[1].set_xlabel('$\zeta = r^2/t$')
axes[1].set_ylabel('Gas saturation (-)')
axes[1].legend()
plt.tight_layout()
plt.savefig("theis3_similarity_fig.png")


sys.exit(0)
