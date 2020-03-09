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
import numpy as np

"""
This script makes log-log plots of the error vs. h for the tests in this directory.
"""

filenames = ['hermite_converge_dirichlet_out.csv',
             'hermite_converge_periodic_out.csv']

for filename in filenames:
    fig = plt.figure()
    ax1 = fig.add_subplot(111)

    # passing names=True option is supposed to treat first row as column
    # header names, and then everything is stored by column name in data.
    data = np.genfromtxt(filename, delimiter=',', names=True)

    log_h1_error = np.log10(data['H1error'])
    log_l2_error = np.log10(data['L2error'])
    logh = np.log10(data['h'])

    h1_fit = np.polyfit(logh, log_h1_error, 1)
    l2_fit = np.polyfit(logh, log_l2_error, 1)

    ax1.plot(logh, log_h1_error, linewidth=2, marker='o', label=r'$H^1$ error')
    ax1.text(-0.4, -2., '{:.2f}'.format(h1_fit[0]))

    ax1.plot(logh, log_l2_error, linewidth=2, marker='o', label=r'$L^2$ error')
    ax1.text(-0.4, -3.5, '{:.2f}'.format(l2_fit[0]))

    ax1.set_xlabel('log(h)')
    ax1.legend(loc='upper left')
    plt.savefig(filename.rsplit( ".", 1)[0] + '.pdf')
