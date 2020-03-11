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
from matplotlib import pyplot as plt
from moosemooseutils.ConvergencePlot import ConvergencePlot

# Convergence Plot
data = pandas.read_csv('advection_mms_out.csv')
fig = ConvergencePlot(data['h'], data['error'], xlabel='Element Length', ylabel='L2 Error')
fig.save('convergence.png')
fig.show()
