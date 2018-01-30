#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import matplotlib.pyplot as plt
import tools

# Create Figure and Axes
figure = plt.figure(facecolor='white')
axes0 = figure.add_subplot(111)

# Level set alone
data = tools.PostprocessorReader('vortex_out.csv')
axes0.plot(data('time'), data('area'), linewidth=1, color='blue', linestyle='-', label='Level set')

# Level set SUPG
data = tools.PostprocessorReader('vortex_supg_out.csv')
axes0.plot(data('time'), data('area'), linewidth=1, color='red', linestyle='-', label='Level set w/ SUPG')

# Level set reinitializtion
data = tools.PostprocessorReader('vortex_reinit_out.csv')
axes0.plot(data('time'), data('area'), linewidth=1, color='green', linestyle='-', label='Level set w/ Reinitialization')

# Exact
axes0.plot([0, 2], [0.0706858347,0.0706858347], linewidth=1, color='black', linestyle='-', label='Initial')


# x0-axis Settings
axes0.legend(loc='best')
axes0.set_xlabel('Time, t')
#axes0.set_xlim([0.0, 0.5])

# y0-axis Settings
axes0.set_ylabel('Area')
#axes0.set_ylim([0.068, 0.074])

# Show figure and write pdf
plt.show()
figure.savefig("example_vortex_area.png")
