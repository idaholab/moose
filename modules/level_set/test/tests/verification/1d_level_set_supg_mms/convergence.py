#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import sys, os
import glob
import pandas
import numpy as np
from mooseutils.ConvergencePlot import ConvergencePlot

# The csv files to read
filenames = glob.glob('1d_level_set_supg_mms*.csv')

# Extract the data
n = len(filenames)
error = np.zeros(n)
length = np.zeros(n)
for i, filename in enumerate(filenames):
    csv = pandas.read_csv(filename)
    error[i] = csv['error'].iloc[-1]
    length[i] = csv['h'].iloc[-1]


fig = ConvergencePlot(length, error, xlabel='Element Length', ylabel='L2 Error')
#fig.save('convergence.pdf')
fig.show()
