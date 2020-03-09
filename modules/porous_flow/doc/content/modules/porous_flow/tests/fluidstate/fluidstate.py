#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import matplotlib.pyplot as plt
import numpy as np


#
# The similarity solution (r^2/t) is applicable even when dissolution is included
#
# Read MOOSE simulation data for constant time (tdata) and constant
# radial distance (rdata) using the water-ncg fluid state
tdata = np.genfromtxt('../../../../../../test/tests/fluidstate/gold/theis_tabulated_csvout_line_0010.csv', delimiter = ',', names = True, dtype = float)
rdata = np.genfromtxt('../../../../../../test/tests/fluidstate/gold/theis_tabulated_csvout.csv', delimiter = ',', names = True, dtype = float)

# Distance where data is sampled as a function of time
r = 1
# Time where data is sampled along the spatial dimension
t = 8e2

plt.figure(0)
fig, axes = plt.subplots(1, 2, figsize = (15, 4))
# Gas pressure vs similarity solution
axes[0].plot(tdata['x']**2 / t, tdata['pgas'] * 1e-6, label = 'Fixed $t$')
axes[0].plot(r**2 / rdata['time'], rdata['pgas'] * 1e-6, 'o', label = 'Fixed $r$')
axes[0].set_xscale('log')
#axes[0].set_xlim([1e-4, 5e1])
axes[0].set_xlabel('$\zeta = r^2/t$')
axes[0].set_ylabel('Gas pressure (MPa)')
axes[0].legend()
# Total mass fraction vs similarity solution
axes[1].plot(tdata['x']**2 / t, tdata['zi'], label = 'Fixed $t$')
axes[1].plot(r**2 / rdata['time'], rdata['zi'], 'o', label = 'Fixed $r$')
axes[1].set_xscale('log')
#axes[1].set_xlim([1e-4, 5e1])
#axes[1].set_ylim([-0.1, 1.1])
axes[1].set_xlabel('$\zeta = r^2/t$')
axes[1].set_ylabel('Total mass fraction (-)')
axes[1].legend()
plt.tight_layout()
plt.savefig("theis_similarity_waterncg_fig.png")

plt.figure(1)
fig, axes = plt.subplots(1, 2, figsize = (15, 4))
# Gas saturation vs similarity solution
axes[0].plot(tdata['x']**2 / t, tdata['saturation_gas'] * 1e-6, label = 'Fixed $t$')
axes[0].plot(r**2 / rdata['time'], rdata['sgas'] * 1e-6, 'o', label = 'Fixed $r$')
axes[0].set_xscale('log')
#axes[0].set_xlim([1e-4, 5e1])
axes[0].set_xlabel('$\zeta = r^2/t$')
axes[0].set_ylabel('Gas saturation')
axes[0].legend()
# Dissolved gas mass fraction vs similarity solution
axes[1].plot(tdata['x']**2 / t, tdata['x1'], label = 'Fixed $t$')
axes[1].plot(r**2 / rdata['time'], rdata['x1'], 'o', label = 'Fixed $r$')
axes[1].set_xscale('log')
#axes[1].set_xlim([1e-4, 5e1])
#axes[1].set_ylim([-0.1, 1.1])
axes[1].set_xlabel('$\zeta = r^2/t$')
axes[1].set_ylabel('Total mass fraction (-)')
axes[1].legend()
plt.tight_layout()
plt.savefig("theis_similarity_waterncg_fig2.png")

#
# Read MOOSE simulation data for constant time (tdata) and constant
# radial distance (rdata) using the brine-co2 fluid state
tdata = np.genfromtxt('../../../../../../test/tests/fluidstate/gold/theis_brineco2_csvout_line_0028.csv', delimiter = ',', names = True, dtype = float)
rdata = np.genfromtxt('../../../../../../test/tests/fluidstate/gold/theis_brineco2_csvout.csv', delimiter = ',', names = True, dtype = float)

# Distance where data is sampled as a function of time
r = 4
# Time where data is sampled along the spatial dimension
t = 1e5

plt.figure(2)
fig, axes = plt.subplots(3, 1, figsize = (8, 8))
# Gas pressure vs similarity solution
axes[0].plot(tdata['x']**2 / t, tdata['pgas'] * 1e-6, label = 'Fixed $t$')
axes[0].plot(r**2 / rdata['time'], rdata['pgas'] * 1e-6, 'o', label = 'Fixed $r$')
axes[0].set_xscale('log')
axes[0].set_xlim([1e-4, 2e1])
axes[0].set_xlabel('$\zeta = r^2/t$')
axes[0].set_ylabel('Gas pressure (MPa)')
axes[0].legend()
# Total mass fraction vs similarity solution
axes[1].plot(tdata['x']**2 / t, tdata['zi'], label = 'Fixed $t$')
axes[1].plot(r**2 / rdata['time'], rdata['zi'], 'o', label = 'Fixed $r$')
axes[1].set_xscale('log')
axes[1].set_xlim([1e-4, 2e1])
axes[1].set_ylim([-0.1, 1.1])
axes[1].set_xlabel('$\zeta = r^2/t$')
axes[1].set_ylabel('Total CO$_2$ mass fraction (-)')
axes[1].legend()
# NaCl mass fraction vs similarity solution
axes[2].plot(tdata['x']**2 / t, tdata['xnacl'], label = 'Fixed $t$')
axes[2].plot(r**2 / rdata['time'], rdata['xnacl'], 'o', label = 'Fixed $r$')
axes[2].set_xscale('log')
axes[2].set_xlim([1e-4, 2e1])
axes[2].set_ylim([0.09, 0.11])
axes[2].set_xlabel('$\zeta = r^2/t$')
axes[2].set_ylabel('NaCl mass fraction (-)')
axes[2].legend()
plt.tight_layout()
plt.savefig("theis_similarity_brineco2_fig.png")

plt.figure(3)
fig, axes = plt.subplots(2, 1, figsize = (8, 8))
# Gas saturation vs similarity solution
axes[0].plot(tdata['x']**2 / t, tdata['saturation_gas'] * 1e-6, label = 'Fixed $t$')
axes[0].plot(r**2 / rdata['time'], rdata['sgas'] * 1e-6, 'o', label = 'Fixed $r$')
axes[0].set_xscale('log')
axes[0].set_xlim([1e-4, 2e1])
axes[0].set_xlabel('$\zeta = r^2/t$')
axes[0].set_ylabel('Gas pressure (MPa)')
axes[0].legend()
# Dissolved gas mass fraction vs similarity solution
axes[1].plot(tdata['x']**2 / t, tdata['x1'], label = 'Fixed $t$')
axes[1].plot(r**2 / rdata['time'], rdata['x1'], 'o', label = 'Fixed $r$')
axes[1].set_xscale('log')
axes[1].set_xlim([1e-4, 2e1])
axes[1].set_xlabel('$\zeta = r^2/t$')
axes[1].set_ylabel('Total CO$_2$ mass fraction (-)')
axes[1].legend()
plt.tight_layout()
plt.savefig("theis_similarity_brineco2_fig2.png")
