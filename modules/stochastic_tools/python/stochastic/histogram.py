#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import os
import argparse
import numpy as np
import matplotlib.mlab as mlab
import matplotlib.pyplot as plt
from scipy import stats
import mooseutils

def command_line_options():
    """
    Command-line options for histogram tool.
    """
    parser = argparse.ArgumentParser(description="Command-line utility for creating histograms from VectorPostprocessor data.")
    parser.add_argument('filename', type=str, help="The VectorPostprocessor data file pattern to open, for sample 'foo_x_*.csv'.")
    parser.add_argument('-t', '--timesteps', default=[-1], nargs='+', type=int, help="List of timesteps to consider, by default only the final timestep is shown.")
    parser.add_argument('-v', '--vectors', default=[], nargs='+', type=str, help="List of vector names to consider, by default all vectors are shown.")
    parser.add_argument('--bins', default=None, type=int, help="Number of bins to consider.")
    parser.add_argument('--alpha', default=0.5, type=float, help="Set the bar chart opacity alpha setting.")
    parser.add_argument('--xlabel', default='Value', type=str, help="The X-axis label.")
    parser.add_argument('--ylabel', default='Probability', type=str, help="The X-axis label.")
    parser.add_argument('--uniform', default=None, type=float, nargs=2, help="Show a uniform distribution between a and b (e.g., --uniform 8 10).")
    parser.add_argument('--weibull', default=None, type=float, nargs=2, help="Show a Weibull distribution with given shape and scale parameters (e.g., --uniform 1 5).")
    return parser.parse_args()

if __name__ == '__main__':

    # Command-line options
    opt = command_line_options()
    opt.filename = os.path.abspath(opt.filename)

    # Read data and set the default vector names
    data = mooseutils.VectorPostprocessorReader(opt.filename)
    if not opt.vectors:
        opt.vectors = data.variables()[1:] # 1: ignores peacock index

    # Plot the results
    times = data.times()
    for t in [times[idx] for idx in opt.timesteps]:
        for vec in opt.vectors:
            plt.hist(data[vec][t], bins=opt.bins, normed=True, alpha=opt.alpha,
                     label="{} (t={})".format(vec, t), ec='black')

    # Add distributions
    if opt.uniform:
        loc = opt.uniform[0]
        scale = opt.uniform[1] - loc
        x = np.linspace(loc, loc + scale, 100)
        plt.plot(x, stats.uniform.pdf(x, loc=loc, scale=scale), label='Uniform Exact')
    if opt.weibull:
        shape = opt.weibull[0]
        scale = opt.weibull[1]
        xlim = plt.gca().get_xlim()
        x = np.linspace(xlim[0], xlim[1], 100)
        plt.plot(x, stats.weibull_min.pdf(x, shape, scale=scale), label='Weibull Exact')

    # Setup the axis and show the plot
    plt.xlabel(opt.xlabel)
    plt.ylabel(opt.ylabel)
    plt.grid(True)
    plt.legend()
    plt.show()
