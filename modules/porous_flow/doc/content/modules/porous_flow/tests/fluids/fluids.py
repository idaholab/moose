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

## creating the analytical function








S=np.linspace(0,1,100) # Saturation of phase 0
S1=np.linspace(1,0,100) # Saturation of phase 1


# Read MOOSE simulation data
brine1 = np.genfromtxt('../../../../../../test/tests/fluids/brine1.csv', delimiter = ',', names = True, dtype = float)
co2 = np.genfromtxt('../../../../../../test/tests/fluids/co2.csv', delimiter = ',', names = True, dtype = float)
h2o = np.genfromtxt('../../../../../../test/tests/fluids/h2o.csv', delimiter = ',', names = True, dtype = float)
ideal_gas = np.genfromtxt('../../../../../../test/tests/fluids/ideal_gas.csv', delimiter = ',', names = True, dtype = float)
methane = np.genfromtxt('../../../../../../test/tests/fluids/methane.csv', delimiter = ',', names = True, dtype = float)


sfdy= np.genfromtxt('../../../../../../test/tests/fluids/simple_fluid_dy_out.csv', delimiter = ',', names = True, dtype = float)
sfhr= np.genfromtxt('../../../../../../test/tests/fluids/simple_fluid_hr_out.csv', delimiter = ',', names = True, dtype = float)
sfMPa= np.genfromtxt('../../../../../../test/tests/fluids/simple_fluid_MPa_out.csv', delimiter = ',', names = True, dtype = float)
sfyrMPa= np.genfromtxt('../../../../../../test/tests/fluids/simple_fluid_yr_MPa_C_out.csv', delimiter = ',', names = True, dtype = float)
sfyr= np.genfromtxt('../../../../../../test/tests/fluids/simple_fluid_yr_out.csv', delimiter = ',', names = True, dtype = float)
sf= np.genfromtxt('../../../../../../test/tests/fluids/simple_fluid.csv', delimiter = ',', names = True, dtype = float)




## Adjust the setting here
x_lim=[-0.05,1.05]
y_lim=[-0.05,1.05]

'''
plt.figure(0)
# Brook Corey 1
brook_corey_a1=brook_corey_func(1) #analytical
plt.plot(S, brook_corey_a1[0] ,'b' , label = 'kr0')
plt.plot(S, brook_corey_a1[1] , 'g' ,label = 'kr1')
plt.plot(brook_corey1['s0aux'], brook_corey1['kr0aux'],'ob' , label = 'kr0 (MOOSE)')
plt.plot(brook_corey1['s0aux'], brook_corey1['kr1aux'],'og' , label = 'kr1 (Moose)')
plt.xlim(x_lim)
plt.ylim(y_lim)
plt.xlabel('Phase 0 saturation (-)')
plt.ylabel('Relative permeability (-)')
plt.legend()
plt.title('Brooks-Corey relative permeability: $\lambda =1$')
plt.tight_layout()
plt.savefig("brook_corey1.png")
'''


