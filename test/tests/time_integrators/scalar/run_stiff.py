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


def run_moose(y2_exponent, dt, time_integrator, lam):
    '''
    Function which actually runs MOOSE.
    '''
    command_line_args = ['../../../moose_test-opt', '-i', 'stiff.i',
                         'Executioner/dt={}'.format(dt),
                         'Executioner/dtmin={}'.format(dt),
                         'Executioner/TimeIntegrator/type={}'.format(time_integrator),
                         'LAMBDA={}'.format(lam),
                         'Y2_EXPONENT={}'.format(y2_exponent)]
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
    #
    # The columns are in alphabetical order, we want to look at
    # "max_error_y1", which is the "stiff" component of the system of
    # ODEs.
    # The columns are currently given by:
    # time,error_y1,error_y2,max_error_y1,max_error_y2,value_y1,value_y1_abs_max,value_y2,value_y2_abs_max,y1,y2
    output_filename = 'stiff_out.csv'
    last_row = get_last_row(output_filename)
    max_error_y1 = float(last_row[3])
    value_y1_abs_max = float(last_row[6])
    normalized_error = max_error_y1 / value_y1_abs_max
    return normalized_error



#
# Main program
#

# implicit methods only - for the large values of lambda considered
# here, explicit methods would require very small timesteps, and so
# they are not considered here.
time_integrators = ['ImplicitEuler', 'CrankNicolson', 'BDF2', 'ImplicitMidpoint', 'LStableDirk2', 'LStableDirk3', 'LStableDirk4', 'AStableDirk4']

# The sequence of timesteps to try
dts = [.5, .25, .125, .0625, .03125, .015625, .0078125]

# The values of lambda to try
# .) lam=-2 is not allowed for p=2 case.
# .) lam=-1 is not allowed for p=1 case.
lams = [-.1, -10, -100, -1000, -10000]

# The values of the y2 exponent to try. 1=linear, 2=nonlinear.
y2_exponents = [1, 2]

# Plot colors
colors = ['maroon', 'blue', 'green', 'black', 'burlywood', 'olivedrab', 'midnightblue',
          'tomato', 'darkmagenta', 'chocolate', 'lightslategray', 'skyblue']

# Plot line markers
markers = ['v', 'o', 'x', '^', 'H', 'h', '+', 'D', '*', '4', 'd', '8']

# Plot line styles
linestyles = [':', '-', '-', '--', ':', '-', '--', ':', '--', '-', '-', '-']

# Loop over:
# lambdas
#   y2 exponents
#     time_integrators
#       dts
for lam in lams:
    for y2_exponent in y2_exponents:
        fig = plt.figure()
        ax1 = fig.add_subplot(111)

        for i in xrange(len(time_integrators)):
            time_integrator = time_integrators[i]

            # Place to store the results for this TimeIntegrator
            results = []

            # Call MOOSE to compute the results
            for dt in dts:
                results.append(run_moose(y2_exponent, dt, time_integrator, lam))

            # Make plot
            xdata = np.log10(np.reciprocal(dts))
            ydata = np.log10(results)

            # Compute linear fit of last three points.
            start_fit = len(xdata) - 3
            end_fit = len(xdata)
            fit = np.polyfit(xdata[start_fit:end_fit], ydata[start_fit:end_fit], 1)

            # Print results for tabulation etc.
            print('{} (Slope={})'.format(time_integrator, fit[0]))
            print('dt,  max_error_y1')
            for j in xrange(len(dts)):
                print('{}, {}'.format(dts[j], results[j]))
            print('') # blank line

            ax1.plot(xdata, ydata, label=time_integrator + ", $" + "{:.2f}".format(fit[0]) + "$",
                     color=colors[i], marker=markers[i], linestyle=linestyles[i])

        # Set up the axis labels.
        ax1.set_xlabel('$\log (\Delta t^{-1})$')
        ax1.set_ylabel('$\log (\|e\|_{L^{\infty}} / \|y_1\|_{L^{\infty}})$')

        # The input file name up to the file extension
        filebase = 'linear'
        if (y2_exponent != 1):
            filebase = 'nonlinear'

        # Add a title
        ax1.set_title('{}, $\\lambda = {}$'.format(filebase.title(), lam))

        # Add a legend
        plt.legend(loc='lower left', prop=fontP)

        # Save a PDF
        plt.savefig(filebase + '_lambda_{}.pdf'.format(lam), format='pdf')


# Local Variables:
# python-indent: 2
# End:
