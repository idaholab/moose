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
ax1.set_ylabel("Strain (%)")
ax1.plot(data['time'], data['eff_plastic_strain'], label='Effective Plastic Strain', color='k')
ax1.plot(data['time'], data['tot_strain_yy'], label='Total YY Strain', color='b')
ax1.plot(data['time'], data['pl_strain_yy'], label='Plastic YY Strain', color='r')
ax1.plot(data['time'], data['el_strain_yy'], label='Elastic YY Strain', color='g')
ax1.yaxis.set_major_formatter(mtick.FormatStrFormatter('%.2e'))
leg = ax1.legend(loc='best')
plt.savefig('plot_cycled_strain.pdf')
plt.show(fig)
