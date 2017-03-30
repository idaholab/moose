#!/usr/bin/env python

import matplotlib.pyplot as plt
import numpy as np

#
# The two phase radial injection problem has a similarity solution (r^2/t)
#
# Read MOOSE simulation data for constant time (tdata) and constant
# radial distance (rdata)
tdata = np.genfromtxt('../../tests/dirackernels/theis3_line_0016.csv', delimiter = ',', names = True, dtype = float)
rdata = np.genfromtxt('../../tests/dirackernels/theis3.csv', delimiter = ',', names = True, dtype = float)

# Distance where data is sampled as a function of time
r = 4
# Time where data is sampled along the spatial dimension
t = 2e4

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
plt.savefig("theis_similarity_fig.pdf")

#
# The similarity solution (r^2/t) is applicable even when dissolution is included
#
# Read MOOSE simulation data for constant time (tdata) and constant
# radial distance (rdata)
tdata = np.genfromtxt('../../tests/fluidstate/theis_csvout_line_0028.csv', delimiter = ',', names = True, dtype = float)
tdata2 = np.genfromtxt('../../tests/fluidstate/theis_csvout_line2_0028.csv', delimiter = ',', names = True, dtype = float)
rdata = np.genfromtxt('../../tests/fluidstate/theis_csvout.csv', delimiter = ',', names = True, dtype = float)

# Distance where data is sampled as a function of time
r = 4
# Time where data is sampled along the spatial dimension
t = 1e5

fig, axes = plt.subplots(1, 2, figsize = (15, 4))
# Gas pressure vs similarity solution
axes[0].plot(tdata['x']**2 / t, tdata['pgas'] * 1e-6, label = 'Fixed $t$')
axes[0].plot(r**2 / rdata['time'], rdata['pgas'] * 1e-6, 'o', label = 'Fixed $r$')
axes[0].set_xscale('log')
axes[0].set_xlim([1e-4, 5e1])
axes[0].set_xlabel('$\zeta = r^2/t$')
axes[0].set_ylabel('Gas pressure (MPa)')
axes[0].legend()
# Total mass fraction vs similarity solution
axes[1].plot(tdata['x']**2 / t, tdata['zi'], label = 'Fixed $t$')
axes[1].plot(r**2 / rdata['time'], rdata['zi'], 'o', label = 'Fixed $r$')
axes[1].set_xscale('log')
axes[1].set_xlim([1e-4, 5e1])
axes[1].set_ylim([-0.1, 1.1])
axes[1].set_xlabel('$\zeta = r^2/t$')
axes[1].set_ylabel('Total mass fraction (-)')
axes[1].legend()
plt.tight_layout()
plt.savefig("theis_similarity_waterncg_fig.pdf")

fig, axes = plt.subplots(1, 2, figsize = (15, 4))
# Gas saturation vs similarity solution
axes[0].plot(tdata2['x']**2 / t, tdata2['saturation_gas'], label = 'Fixed $t$')
axes[0].plot(r**2 / rdata['time'], rdata['sgas'], 'o', label = 'Fixed $r$')
axes[0].set_xscale('log')
axes[0].set_xlim([1e-4, 5e1])
axes[0].set_ylim([-0.1, 1.1])
axes[0].set_xlabel('$\zeta = r^2/t$')
axes[0].set_ylabel('Gas saturation (-)')
axes[0].legend()
# Dissolved CO2 mass fraction vs similarity solution
axes[1].plot(tdata2['x']**2 / t, tdata2['x1'], label = 'Fixed $t$')
axes[1].plot(r**2 / rdata['time'], rdata['x1'], 'o', label = 'Fixed $r$')
axes[1].set_xscale('log')
axes[1].set_xlim([1e-4, 5e1])
axes[1].set_ylim([-0.1, 0.5])
axes[1].set_xlabel('$\zeta = r^2/t$')
axes[1].set_ylabel('CO$_2$ dissolved mass fraction (-)')
axes[1].legend()
plt.tight_layout()
plt.savefig("theis_similarity_waterncg_fig2.pdf")
