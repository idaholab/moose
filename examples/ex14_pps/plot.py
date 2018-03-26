#!/usr/bin/env python2
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
import csv

# Python 2.7 does not have str.isnumeric()?
def isInt(string):
    try:
        int(string)
        return True
    except ValueError:
        return False

# Format of the CSV file is:
# time,dofs,integral
# 1,221,2.3592493758695,
# 2,841,0.30939803328432,
# 3,3281,0.088619511656913,
# 4,12961,0.022979021365857,
# 5,51521,0.0057978748995635,
# 6,205441,0.0014528130907967,
reader = csv.reader(file('ex14_out.csv'))

dofs = []
errs = []
for row in reader:
    if row and isInt(row[0]): # Skip rows that don't start with numbers.
        dofs.append(int(row[1]))
        errs.append(float(row[2]))

# Construct data to be plotted
xdata = np.log10(np.sqrt(dofs))
ydata = np.log10(errs)

fig = plt.figure()
ax1 = fig.add_subplot(111)
ax1.plot(xdata, ydata, 'bo-')
ax1.set_xlabel('log (1/h)')
ax1.set_ylabel('log (L2-error)')

# Create linear curve fits of the data, but just the last couple data
# point when we are in the asymptotic regime.
fit = np.polyfit(xdata[2:-1], ydata[2:-1], 1)
fit_msg = 'Slope ~ ' + '%.2f' % fit[0]
ax1.text(2.0, -1.0, fit_msg)

plt.savefig('plot.pdf', format='pdf')

# Local Variables:
# python-indent: 2
# End:
