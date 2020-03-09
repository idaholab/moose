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

data = np.genfromtxt('gold/atm_tides_open_hole_out.csv', delimiter = ',', names = True, dtype = float)

avp100_100 = np.mean(data['p100_100'][-24:])
avp0_100 = np.mean(data['p0_100'][-24:])
avuz = np.mean(data['uz0'][-24:])
avuz100 = np.mean(data['uz100'][-24:])

plt.subplots(1, 2, figsize=(10, 4))

plt.subplot(1, 2, 1)
# remember the first timestep is equilibration, so do not plot that
plt.plot(data['time'][1:] / 3600.0, (data['p0_0'][1:] - data['p0_0'][1]) / 1000.0, label='atmospheric variation')
plt.plot(data['time'][1:] / 3600.0, (data['p100_100'][1:] - data['p100_100'][1]) / 1000.0, label='100m deep, 100m from borehole bottom')
plt.xlabel('t (hr)')
plt.ylabel('Porepressure change (kPa)')
plt.grid()
plt.legend()
plt.title('Porepressure (model with borehole)')

plt.subplot(1, 2, 2)
# remember the first timestep is equilibration, so do not plot that
plt.plot(data['time'][1:] / 3600.0, 1000 * (data['uz0'][1:]), label='above borehole')
plt.plot(data['time'][1:] / 3600.0, 1000 * (data['uz100'][1:]), label='100m from borehole')
plt.xlabel('t (hr)')
plt.ylabel('Vertical displacement (mm)')
plt.legend()
plt.title('Displacement of top (model with borehole)')

plt.tight_layout()

plt.savefig("../../doc/content/media/porous_flow/atm_tides_open_hole_p_uz.png")

sys.exit(0)
