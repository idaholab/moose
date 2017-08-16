#!/usr/bin/env python
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
