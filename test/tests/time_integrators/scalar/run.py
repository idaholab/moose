#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import subprocess
import sys
import csv
import matplotlib.pyplot as plt
import numpy as np

# Use fonts that match LaTeX
from matplotlib import rcParams
rcParams['font.family'] = 'serif'
rcParams['font.size'] = 17
rcParams['font.serif'] = ['Computer Modern Roman']
rcParams['text.usetex'] = True

# Small font size for the legend
from matplotlib.font_manager import FontProperties
fontP = FontProperties()
fontP.set_size('x-small')


def get_last_row(csv_filename):
    '''
    Function which returns just the last row of a CSV file.  We have to
    read every line of the file, there was no stackoverflow example of
    reading just the last line.
    http://stackoverflow.com/questions/20296955/reading-last-row-from-csv-file-python-error
    '''
    with open(csv_filename, 'r') as f:
        lastrow = None
        for row in csv.reader(f):
            if (row != []): # skip blank lines at end of file.
                lastrow = row
        return lastrow


def run_moose(dt, time_integrator):
    '''
    Function which actually runs MOOSE.
    '''
    implicit_flag = 'true'
    explicit_methods = ['ExplicitEuler', 'ExplicitMidpoint', 'Heun', 'Ralston']

    # Set implicit_flag based on TimeIntegrator name
    if (time_integrator in explicit_methods):
        implicit_flag = 'false'

    command_line_args = ['../../../moose_test-opt', '-i', 'scalar.i',
                         'Executioner/dt={}'.format(dt),
                         'Executioner/TimeIntegrator/type={}'.format(time_integrator),
                         'GlobalParams/implicit={}'.format(implicit_flag)]
    try:
        child = subprocess.Popen(command_line_args, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
        # communicate() waits for the process to terminate, so there's no
        # need to wait() for it.  It also sets the returncode attribute on
        # child.
        (stdoutdata, stderrdata) = child.communicate()
        if (child.returncode != 0):
            print('Running MOOSE failed: program output is below:')
            print(stdoutdata)
            raise
    except:
        print('Error executing moose_test')
        sys.exit(1)

    # Parse the last line of the output file to get the error at the final time.
    last_row = get_last_row('scalar_out.csv')
    return float(last_row[1])



#
# Main program
#
fig = plt.figure()
ax1 = fig.add_subplot(111)

# Lists of timesteps and TimeIntegrators to plot.
time_integrators = ['ImplicitEuler', 'ImplicitMidpoint', 'LStableDirk2', 'BDF2', 'CrankNicolson',
                    'LStableDirk3', 'LStableDirk4', 'AStableDirk4',
                    'ExplicitEuler', 'ExplicitMidpoint', 'Heun', 'Ralston']
dts = [.125, .0625, .03125, .015625]

# Plot colors
colors = ['maroon', 'blue', 'green', 'black', 'burlywood', 'olivedrab', 'midnightblue',
          'tomato', 'darkmagenta', 'chocolate', 'lightslategray', 'skyblue']

# Plot line markers
markers = ['v', 'o', 'x', '^', 'H', 'h', '+', 'D', '*', '4', 'd', '8']

# Plot line styles
linestyles = [':', '-', '-.', '--', ':', '-.', '--', ':', '--', '-', '-.', '-']

for i in xrange(len(time_integrators)):
    time_integrator = time_integrators[i]

    # Place to store the results for this TimeIntegrator
    results = []

    # Call MOOSE to compute the results
    for dt in dts:
        results.append(run_moose(dt, time_integrator))

    # Make plot
    xdata = np.log10(np.reciprocal(dts))
    ydata = np.log10(results)

    # Compute linear fit of last three points.
    start_fit = len(xdata) - 3
    end_fit = len(xdata)
    fit = np.polyfit(xdata[start_fit:end_fit], ydata[start_fit:end_fit], 1)

    # Make the plot -- unpack the user's additional plotting arguments
    # from kwargs by prepending with **.
    ax1.plot(xdata, ydata, label=time_integrator + ", $" + "{:.2f}".format(fit[0]) + "$",
             color=colors[i], marker=markers[i], linestyle=linestyles[i])


# Set up the axis labels.
ax1.set_xlabel('$\log (\Delta t^{-1})$')
ax1.set_ylabel('$\log \|e(T)\|_{L^2}$')

# Add a legend
plt.legend(loc='lower left', prop=fontP)

# Save a PDF
plt.savefig('plot.pdf', format='pdf')


# Local Variables:
# python-indent: 2
# End:
