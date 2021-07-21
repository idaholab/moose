#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

# Solution to Terzaghi consolidation as presented in
# Section 2.2 of the online manuscript: Arnold Verruijt "Theory and Problems of Poroelasticity" Delft University of Technology 2013.  But note that the "sigma" in that paper is the negative of the stress in PorousFlow.

import os
import sys
import numpy as np
import matplotlib.pyplot as plt

# Base properties
soil_height = 10
soil_lame_lambda = 2
soil_lame_mu = 3
fluid_bulk_modulus = 8
fluid_mobility = 1.5
soil_initial_porosity = 0.1
biot_coefficient = 0.6
normal_stress_on_top = 1

# Derived properties
soil_bulk_modulus = soil_lame_lambda + 2.0 * soil_lame_mu / 3.0
soil_confined_compressibility = 1.0 / (soil_bulk_modulus + 4.0 * soil_lame_mu / 3.0)
soil_bulk_compliance = 1.0 / soil_bulk_modulus
fluid_bulk_compliance = 1.0 / fluid_bulk_modulus
soil_initial_storativity = soil_initial_porosity / fluid_bulk_modulus + (biot_coefficient - soil_initial_porosity) * (1.0 - biot_coefficient) / soil_bulk_modulus
consolidation_coefficient = fluid_mobility / (soil_initial_storativity + soil_confined_compressibility * biot_coefficient**2)
initial_porepressure = biot_coefficient * soil_confined_compressibility * normal_stress_on_top / (soil_initial_storativity + soil_confined_compressibility * biot_coefficient**2)
final_displacement = normal_stress_on_top * soil_height * soil_confined_compressibility
initial_displacement = normal_stress_on_top * soil_height * soil_confined_compressibility * soil_initial_storativity / (soil_initial_storativity + soil_confined_compressibility * biot_coefficient**2)

def expectedU(t):
    ctoverh2 = t * consolidation_coefficient / soil_height**2

    def coeff(n, ctoverh2):
        return np.power(1.0 / (2.0 * n - 1.0), 2) * np.exp(-np.power(2.0 * n - 1.0, 2) * np.pi ** 2 * ctoverh2 / 4.0)

    result = 0.0 * t
    for i in range(1, 11):
        result += coeff(i, ctoverh2)
    return 1.0 - (8.0 / np.pi**2) * result

def expectedP(t, z):
    ctoverh2 = t * consolidation_coefficient / soil_height**2
    zoverh = z / soil_height

    def coeff(n, zoverh):
        return np.power(-1.0, n - 1) / (2.0 * n - 1.0) * np.cos((2.0 * n - 1.0) * np.pi * zoverh / 2.0) * np.exp(-np.power(2 * n - 1, 2) * np.pi**2 * ctoverh2 / 4.0)

    result = 0.0 * z
    for i in range(1, 21):
        result += coeff(i, zoverh)
    approx = round(ctoverh2, -int(np.floor(np.sign(ctoverh2) * np.log10(abs(ctoverh2)))) + 1)
    return (approx, zoverh, (4.0 / np.pi) * result)

def get_moose_results(fn):
    f = open(fn, 'r')
    data = [list(map(float, line.strip().split(","))) for line in f.readlines()[1:] if line.strip()]
    f.close()
    t = [d[0] for d in data]
    u = [(-d[12] - initial_displacement) / (final_displacement - initial_displacement) for d in data]
    p0_029 = [d for d in data if (d[0] > 0.029 and d[0] < 0.03)][0]
    p0_22 = [d for d in data if (d[0] > 0.2 and d[0] < 0.3)][0]
    p1_6 = [d for d in data if (d[0] > 1.6 and d[0] < 1.7)][0]
    p10 = data[-1]
    return (t, u, p0_029, p0_22, p1_6, p10)

moose = get_moose_results("../../../../../../test/tests/poro_elasticity/gold/terzaghi_constM.csv")

plt.figure(0)
tpoints = np.arange(0, 10.1, 0.1)
plt.plot(tpoints, expectedU(tpoints), 'k-', linewidth = 2.0, label = 'expected')
plt.plot(moose[0], moose[1], 'r.', label = 'PorousFlow')
plt.legend(loc = 'best')
plt.xlabel("time")
plt.ylabel("degree of consolidation")
plt.title("Terzaghi's consolidation problem: displacement")
plt.savefig("terzaghi_u.png")

plt.figure(1)
zpoints = np.arange(0, 10.1, 0.1)
zoverh = np.arange(0, 1.1, 0.1)

t = moose[2][0]
ex = expectedP(t, zpoints)
plt.plot(ex[1], ex[2], 'k-', linewidth = 2.0, label = 'expected ' + str(ex[0]))
plt.plot(zoverh, np.array(moose[2][1:-1]) / initial_porepressure, 'ko', label = 'MOOSE ' + str(ex[0]))

t = moose[3][0]
ex = expectedP(t, zpoints)
plt.plot(ex[1], ex[2], 'b-', linewidth = 2.0, label = 'expected ' + str(ex[0]))
plt.plot(zoverh, np.array(moose[3][1:-1]) / initial_porepressure, 'bo', label = 'MOOSE ' + str(ex[0]))
plt.legend(loc = 'best')

t = moose[4][0]
ex = expectedP(t, zpoints)
plt.plot(ex[1], ex[2], 'r-', linewidth = 2.0, label = 'expected ' + str(ex[0]))
plt.plot(zoverh, np.array(moose[4][1:-1]) / initial_porepressure, 'ro', label = 'MOOSE ' + str(ex[0]))
plt.legend(loc = 'best')

t = moose[5][0]
ex = expectedP(t, zpoints)
plt.plot(ex[1], ex[2], 'g-', linewidth = 2.0, label = 'expected ' + str(ex[0]))
plt.plot(zoverh, np.array(moose[5][1:-1]) / initial_porepressure, 'go', label = 'MOOSE ' + str(ex[0]))
plt.legend(loc = 'best')

plt.xlabel("z/h")
plt.ylabel("P/P0")
plt.title("Terzaghi's consolidation problem: porepressure")
plt.savefig("terzaghi_p.png")
sys.exit(0)
