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


a01 = np.genfromtxt('../../../../../../test/tests/heat_advection/gold/heat_advection_1d_csv_T_0010.csv', delimiter = ',', names = True, dtype = float)
b01 = np.genfromtxt('../../../../../../test/tests/heat_advection/gold/heat_advection_1d_fully_saturated_T_0010.csv', delimiter = ',', names = True, dtype = float)
c01 = np.genfromtxt('../../../../../../test/tests/heat_advection/gold/heat_advection_1d_KT_T_0010.csv', delimiter = ',', names = True, dtype = float)

a06 = np.genfromtxt('../../../../../../test/tests/heat_advection/gold/heat_advection_1d_csv_T_0060.csv', delimiter = ',', names = True, dtype = float)
b06 = np.genfromtxt('../../../../../../test/tests/heat_advection/gold/heat_advection_1d_fully_saturated_T_0060.csv', delimiter = ',', names = True, dtype = float)
c06 = np.genfromtxt('../../../../../../test/tests/heat_advection/gold/heat_advection_1d_KT_T_0060.csv', delimiter = ',', names = True, dtype = float)

plt.figure()
fig, axes = plt.subplots(1, 2, figsize = (15, 4))
# Water pressure vs similarity solution
axes[0].plot(b01['x'], b01['temp'], label = 'No upwinding')
axes[0].plot(a01['x'], a01['temp'], label = 'Full upwinding')
axes[0].plot(c01['x'], c01['temp'], label = 'KT stabilization')
axes[0].set_xlabel('x (m)')
axes[0].set_ylabel('Temperature (K)')
axes[0].grid()
axes[0].legend()
axes[0].set_title("Temperature at 0.1s")
# Gas saturation vs similarity solution
axes[1].plot(b06['x'], b06['temp'], label = 'No upwinding')
axes[1].plot(a06['x'], a06['temp'], label = 'Full upwinding')
axes[1].plot(c06['x'], c06['temp'], label = 'KT stabilization')
axes[1].set_xlabel('x (m)')
axes[1].set_ylabel('Temperature (K)')
axes[1].legend()
axes[1].grid()
axes[1].set_title("Temperature at 0.6s")
plt.tight_layout()
plt.savefig("heat_advection.png")


sys.exit(0)
