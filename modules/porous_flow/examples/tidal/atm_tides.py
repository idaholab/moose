#!/usr/bin/env python32
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import sys
import matplotlib.pyplot as plt
import numpy as np

data = np.genfromtxt('gold/atm_tides_out.csv', delimiter = ',', names = True, dtype = float)

plt.subplots(1, 2, figsize=(10, 4))

plt.subplot(1, 2, 1)
# remember the first timestep is equilibration, so do not plot that
plt.plot(data['time'][1:] / 3600.0, data['p0'][1:]/1000.0, label='at top')
plt.plot(data['time'][1:] / 3600.0, (data['p100'][1:] - data['p100'][1])/1000.0, label='100m')
plt.xlabel('t (hr)')
plt.ylabel('Porepressure change (kPa)')
plt.grid()
plt.legend()
plt.title('Porepressure response to Atm tide')

plt.subplot(1, 2, 2)
# remember the first timestep is equilibration, so do not plot that
plt.plot(data['time'][1:] / 3600.0, 1000 * data['uz0'][1:])
plt.xlabel('t (hr)')
plt.ylabel('Vertical displacement (mm)')
plt.grid()
plt.legend()
plt.title('Displacement of ground surface in response to Atm tide')

plt.tight_layout()

plt.savefig("../../doc/content/media/porous_flow/atm_tides_p_uz.png")

sys.exit(0)
