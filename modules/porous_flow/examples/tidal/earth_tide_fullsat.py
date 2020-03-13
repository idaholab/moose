#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

# This script generates pictures that show the porepressure response to some "realistic" strain fields

import os
import sys
import numpy as np
import matplotlib.pyplot as plt

def strain_xx(t):
    return 5*np.cos(t*2*np.pi) + 2*np.cos((t-0.5)*2*np.pi) + 1*np.cos((t+0.3)*0.5*np.pi)

def strain_yy(t):
    return 7*np.cos(t*2*np.pi) + 4*np.cos((t-0.3)*2*np.pi) + 7*np.cos((t+0.6)*0.5*np.pi)

def strain_zz(t):
    return 7*np.cos((t-0.5)*2*np.pi) + 4*np.cos((t-0.8)*2*np.pi) + 7*np.cos((t+0.1)*4*np.pi)

moose = np.genfromtxt("gold/earth_tide_fullsat_out.csv", delimiter = ',', names = True, dtype = float)


plt.subplots(1, 2, figsize=(10, 4))

plt.subplot(1, 2, 1)
t = np.arange(0, 2.01, 0.01)
plt.plot(t, strain_xx(t), label=r'$\epsilon_{xx}$')
plt.plot(t, strain_yy(t), label=r'$\epsilon_{yy}$')
plt.plot(t, strain_zz(t), label=r'$\epsilon_{zz}$')
plt.legend()
plt.xlabel("days")
plt.ylabel("Strain (nstrain)")
plt.title("Applied earth-tide strains")

plt.subplot(1, 2, 2)
plt.plot(moose['time'][1:], moose['pp'][1:])
plt.xlabel("days")
plt.ylabel("Porepressure (Pa)")
plt.title("Resulting porepressure changes")

plt.tight_layout()

plt.savefig("../../doc/content/media/porous_flow/earth_tide_fullsat.png")

sys.exit(0)
