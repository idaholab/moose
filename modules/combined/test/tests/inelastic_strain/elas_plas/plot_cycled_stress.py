#!/opt/moose/miniconda/bin/python
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import matplotlib as mpl
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.ticker as mtick
import pylab

data = np.genfromtxt('./elas_plas_nl1_cycle_out.csv', delimiter=',', names=True)

fig = plt.figure()
ax1 = fig.add_subplot(111)
mpl.rcParams.update({'font.size': 10})
ax1.set_xlabel("Time")
ax1.set_ylabel("Stress (MPa)")
ax1.plot(data['time'], data['stress_yy'], label='Stress YY', color='k')
ax1.plot(data['time'], data['vonmises'], label='Vonmises', color='b')
ax1.plot(data['time'], data['pressure'], label='Pressure', color='r')
ax1.yaxis.set_major_formatter(mtick.FormatStrFormatter('%.2e'))
leg = ax1.legend(loc='best')
plt.savefig('plot_cycled_stress.pdf')
plt.show(fig)
