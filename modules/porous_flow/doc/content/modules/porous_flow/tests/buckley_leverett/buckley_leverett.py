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


sat_data = np.genfromtxt('../../../../../../test/tests/buckley_leverett/gold/bl01_sat_0001.csv', delimiter = ',', names = True, dtype = float)
pp_data = np.genfromtxt('../../../../../../test/tests/buckley_leverett/gold/bl01_pp_0001.csv', delimiter = ',', names = True, dtype = float)

plt.figure(0)
fig, axes = plt.subplots(1, 2, figsize = (15, 4))
# Saturation
axes[0].plot(sat_data['x'], sat_data['sat'])
axes[0].set_xlabel('$x$')
axes[0].set_ylabel('Saturation')
axes[0].legend()
axes[0].grid()
# Total mass fraction vs similarity solution
axes[1].plot(pp_data['x'], pp_data['pp'] / 1E6)
axes[1].set_xlabel('$x$')
axes[1].set_ylabel('Porepressure (MPa)')
axes[1].legend()
axes[1].grid()
plt.tight_layout()
plt.savefig("bl_initial1.png")


sat_data = np.genfromtxt('../../../../../../test/tests/buckley_leverett/gold/bl01_sat_0150.csv', delimiter = ',', names = True, dtype = float)
pp_data = np.genfromtxt('../../../../../../test/tests/buckley_leverett/gold/bl01_pp_0150.csv', delimiter = ',', names = True, dtype = float)

plt.figure(1)
fig, axes = plt.subplots(1, 2, figsize = (15, 4))
# Saturation
axes[0].plot(sat_data['x'], sat_data['sat'])
axes[0].set_xlabel('$x$')
axes[0].set_ylabel('Saturation')
axes[0].legend()
axes[0].grid()
# Total mass fraction vs similarity solution
axes[1].plot(pp_data['x'], pp_data['pp'] / 1E6)
axes[1].set_xlabel('$x$')
axes[1].set_ylabel('Porepressure (MPa)')
axes[1].legend()
axes[1].grid()
plt.tight_layout()
plt.savefig("bl01_sat_and_pp1.png")
