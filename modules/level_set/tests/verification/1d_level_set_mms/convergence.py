#!/usr/bin/env python
import sys, os
import glob
import pandas
import numpy as np
from mooseutils.ConvergencePlot import ConvergencePlot

# The csv files to read
filenames = glob.glob('level_set_mms*.csv')

# Extract the data
n = len(filenames)
error = np.zeros(n)
length = np.zeros(n)
for i, filename in enumerate(filenames):
    csv = pandas.read_csv(filename)
    error[i] = csv['error'].iloc[-1]
    length[i] = csv['h'].iloc[-1]


fig = ConvergencePlot(length, error, xlabel='Element Length', ylabel='L2 Error')
fig.save('convergence.pdf')
fig.show()
