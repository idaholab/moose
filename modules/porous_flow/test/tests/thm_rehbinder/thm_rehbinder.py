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

def rehbinder(r):
    # Results from Rehbinder with parameters used in the MOOSE simulation.
    # Rehbinder's manuscript contains a few typos - I've corrected them here.
    # G Rehbinder "Analytic solutions of stationary coupled thermo-hydro-mechanical problems" Int J Rock Mech Min Sci & Geomech Abstr 32 (1995) 453-463
    poisson = 0.2
    thermal_expansion = 1E-6
    young = 1E10
    fluid_density = 1000
    fluid_specific_heat = 1000
    permeability = 1E-12
    fluid_viscosity = 1E-3
    thermal_conductivity = 1E6
    P0 = 1E6
    T0 = 1E3
    Tref = T0
    r0 = 0.1
    r1 = 1.0
    xi = r / r0
    xi1 = r1 / r0
    Peclet = fluid_density * fluid_specific_heat * thermal_expansion * Tref * young * permeability / fluid_viscosity / thermal_conductivity / (1 - poisson)

    That0 = T0 / Tref
    sigmahat0 = -P0 * (1 - poisson) / thermal_expansion / Tref / young

    Tzeroth = That0 * (1 - np.log(xi) / np.log(xi1))
    Tfirst_pr = 2 * sigmahat0 * That0 * xi * (np.log(xi) - np.log(xi1)) / np.log(xi1)**2
    Cone = 2 * That0 * sigmahat0 * (2 + np.log(xi1)) / np.log(xi1)**2
    Cone = 2 * That0 * sigmahat0 / np.log(xi1)  # Corrected Eqn(87)
    Done = 2 * That0 * sigmahat0 * (2 * (xi1 - 1) / np.log(xi1) - 1) / np.log(xi1)**2
    Done = 2 * That0 * sigmahat0 * (- 1) / np.log(xi1)**2 # Corrected Eqn(87)
    Tfirst_hm = Cone + Done * np.log(xi)
    Tfirst = Tfirst_pr + Tfirst_hm
    That = Tzeroth + Peclet * Tfirst
    T = Tref * That

    Pzeroth = -sigmahat0 * (1 - np.log(xi) / np.log(xi1))
    Pfirst = 0
    Phat = Pzeroth + Peclet * Pfirst
    P = thermal_expansion * Tref * young * Phat / (1 - poisson)

    g0 = Tzeroth + (1 - 2 * poisson) * Pzeroth / (1 - poisson)
    uzeroth_pr = (That0 - sigmahat0 * (1 - 2 * poisson) / (1 - poisson)) * (0.5 * (xi**2 - 1) - 0.25 * (1 - xi**2 + 2 * xi**2 * np.log(xi)) / np.log(xi1)) / xi
    uzeroth_pr_xi1 = (That0 - sigmahat0 * (1 - 2 * poisson) / (1 - poisson)) * (0.5 * (xi1**2 - 1) - 0.25 * (1 - xi1**2 + 2 * xi1**2 * np.log(xi1)) / np.log(xi1)) / xi1
    # fixed outer boundary
    Bzeroth = - ((1 - 2 * poisson) * sigmahat0 + uzeroth_pr_xi1 / xi1) / (1 - 2 * poisson + 1.0 / xi1)
    Azeroth = - Bzeroth / xi1**2 - uzeroth_pr_xi1 / xi1
    fixed_uzeroth_hm = Azeroth * xi + Bzeroth / xi
    fixed_uzeroth = uzeroth_pr + fixed_uzeroth_hm
    # free outer boundary
    Bzeroth = (xi1**2 * sigmahat0 - xi1 * uzeroth_pr_xi1) / (1 - xi1**2)
    Azeroth = (1 - 2 * poisson) * (Bzeroth + sigmahat0)
    free_uzeroth_hm = Azeroth * xi + Bzeroth / xi
    free_uzeroth = uzeroth_pr + free_uzeroth_hm

    ufirst_pr = (1.0 / xi) * (0.5 * (xi**2 - 1) * (2 * Cone - Done) + 0.5 * Done * xi**2 * np.log(xi) + 2 * sigmahat0 * That0 / np.log(xi1)**2 * (xi**3 * np.log(xi) / 3 + (1 - xi**3) / 9 + 0.5 * np.log(xi1) * (1 - xi**2)))
    ufirst_pr_xi1 = (1.0 / xi1) * (0.5 * (xi1**2 - 1) * (2 * Cone - Done) + 0.5 * Done * xi1**2 * np.log(xi1) + 2 * sigmahat0 * That0 / np.log(xi1)**2 * (xi1**3 * np.log(xi1) / 3 + (1 - xi1**3) / 9 + 0.5 * np.log(xi1) * (1 - xi1**2)))
    # fixed outer boundary
    Bfirst = - ufirst_pr_xi1 / xi1 / (1 - 2 * poisson + 1.0 / xi1**2)
    Afirst = - Bfirst / xi1**2 - ufirst_pr_xi1 / xi1
    fixed_ufirst_hm = Afirst * xi + Bfirst / xi
    fixed_ufirst = ufirst_pr + fixed_ufirst_hm
    # free outer boundary
    Bfirst = xi1 * ufirst_pr_xi1 / (1 - xi1**2)
    Afirst = (1 - 2 * poisson) * Bfirst
    free_ufirst_hm = Afirst * xi + Bfirst / xi
    free_ufirst = ufirst_pr + free_ufirst_hm

    fixed_uhat = fixed_uzeroth +  Peclet * fixed_ufirst
    fixed_u = thermal_expansion * Tref * r0 * fixed_uhat * (1 + poisson) / (1 - poisson) # Corrected Eqn(16)
    free_uhat = free_uzeroth +  Peclet * free_ufirst
    free_u = thermal_expansion * Tref * r0 * free_uhat * (1 + poisson) / (1 - poisson) # Corrected Eqn(16)
    return (T, P, fixed_u, free_u)

def moose(fn, rcol, datacol):
    try:
        f = open(fn)
        data = f.readlines()[1:-1]
        data = [list(map(float, d.strip().split(","))) for d in data]
        data = ([d[rcol] for d in data], [d[datacol] for d in data])
        f.close()
    except:
        sys.stderr.write("Cannot read " + fn + ", or it contains erroneous data\n")
        sys.exit(1)
    return data

mooser = [0.1 * i for i in range(1, 11)]
fixedT = moose("gold/fixed_outer_T_0001.csv", 0, 4)
fixedP = moose("gold/fixed_outer_P_0001.csv", 0, 4)
fixedu = moose("gold/fixed_outer_U_0001.csv", 0, 4)
freeu = moose("gold/free_outer_U_0001.csv", 0, 4)

rpoints = np.arange(0.1, 1.0, 0.01)
expected = list(zip(*[rehbinder(r) for r in rpoints]))

plt.figure()
plt.plot(rpoints, expected[0], 'k-', linewidth = 3.0, label = 'expected')
plt.plot(fixedT[0], fixedT[1], 'rs', markersize = 10.0, label = 'MOOSE')
plt.legend(loc = 'upper right')
plt.xlabel("r (m)")
plt.ylabel("Temperature (K)")
plt.title("Temperature around cavity")
plt.savefig("temperature_fig.pdf")

plt.figure()
plt.plot(rpoints, [1E-6 * p for p in expected[1]], 'k-', linewidth = 3.0, label = 'expected')
plt.plot(fixedP[0], [1E-6 * p for p in fixedP[1]], 'rs', markersize = 10.0, label = 'MOOSE')
plt.legend(loc = 'upper right')
plt.xlabel("r (m)")
plt.ylabel("Porepressure (MPa)")
plt.title("Porepressure around cavity")
plt.savefig("porepressure_fig.pdf")

plt.figure()
plt.plot(rpoints, [1000 * u for u in expected[2]], 'k-', linewidth = 3.0, label = 'expected (fixed)')
plt.plot(fixedu[0], [1000 * u for u in fixedu[1]], 'rs', markersize = 10.0, label = 'MOOSE (fixed)')
plt.plot(rpoints, [1000 * u for u in expected[3]], 'b-', linewidth = 2.0, label = 'expected (free)')
plt.plot(freeu[0], [1000 * u for u in freeu[1]], 'g*', markersize = 13.0, label = 'MOOSE (free)')
plt.legend(loc = 'center right')
plt.xlabel("r (m)")
plt.ylabel("displacement (mm)")
plt.title("Radial displacement around cavity")
plt.savefig("displacement_fig.pdf")

sys.exit(0)
