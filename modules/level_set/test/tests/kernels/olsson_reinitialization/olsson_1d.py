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
import pandas
import glob

tdata = pandas.read_csv('output/olsson_1d_out_line_time.csv')
ax = plt.subplot(111)


for index, row in tdata.iterrows():
    filename = 'output/olsson_1d_out_line_{:04d}.csv'.format(int(row['timestep']))
    data = pandas.read_csv(filename)
    ax.plot(data['x'], data['phi'])

plt.show()
