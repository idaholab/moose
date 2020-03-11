#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

# This file contains the original convergence work performed on the time integration schemes with
# the convergence data hard-coded into this file. The data includes more time steps than are
# executed in the tests so it is best to keep both.

import matplotlib.pyplot as plt
import numpy as np

# Small font size for the legend
from matplotlib.font_manager import FontProperties
fontP = FontProperties()
fontP.set_size('x-small')

# Implicit methods - errors reported are L2-errors at time T=1.
# Note: for reasons of expediency, this does not match the end_time
# used in implicit_convergence.i.

# Implicit Euler: dt and L2-error at final time
implicit_euler = [
  .5, 4.749134e-02,
  .25, 7.198284e-02,
  .125, 3.774241e-02,
  .0625, 1.932944e-02,
  .03125, 9.781895e-03,
  .015625, 4.920568e-03,
  .0078125, 2.467729e-03,
  .00390625, 1.235731e-03,
  .001953125, 6.183327e-04,
  .0009765625, 3.092833e-04,
  .00048828125, 1.546709e-04,
  .000244140625, 7.734274e-05,
  .0001220703125, 3.867320e-05,
]

# The implicit midpoint rule is implemented as a 2-stage RK method in MOOSE.
implicit_midpoint = [
  0.5, 2.334489e-01,
  0.25, 5.793740e-02,
  0.125, 1.445699e-02,
  0.0625, 3.613887e-03,
  0.03125, 9.034762e-04,
  0.015625, 2.258694e-04,
  0.0078125, 5.646737e-05,
  0.00390625, 1.411684e-05,
  0.001953125, 3.529211e-06,
  0.0009765625, 8.823027e-07,
  0.00048828125, 2.205757e-07,
  0.000244140625, 5.514383e-08,
  0.0001220703125, 1.378596e-08,
]

# L-stable DIRK2: dt and L2-error at final time
lstable_dirk2 = [
  .5, 1.715309e-02,
  .25, 6.981537e-03,
  .125, 2.298552e-03,
  .0625, 6.763307e-04,
  .03125, 1.861684e-04,
  .015625, 4.918636e-05,
  .0078125, 1.267732e-05,
  .00390625, 3.221162e-06,
  .001953125, 8.120884e-07,
  .0009765625, 2.038930e-07,
  .00048828125, 5.108318e-08,
  .000244140625, 1.278476e-08,
  .0001220703125, 3.197922e-09,
]

# A-stable DIRK2: dt and L2-error at final time
dirk2 = [
  .5, 1.508335e-01,
  .25, 3.735726e-02,
  .125, 9.357433e-03,
  .0625, 2.347278e-03,
  .03125, 5.879120e-04,
  .015625, 1.471145e-04,
  .0078125, 3.679566e-05,
  .00390625, 9.201044e-06,
  .001953125, 2.300527e-06,
  .0009765625, 5.751650e-07,
  .00048828125, 1.437954e-07,
  .000244140625, 3.594937e-08,
  .0001220703125, 8.987415e-09,
]

# BDF2: dt and L2-error at final time
bdf2 = [
  .5, 6.393734e-02,
  .25, 1.637917e-02,
  .125, 4.050166e-03,
  .0625, 1.009171e-03,
  .03125, 2.521444e-04,
  .015625, 6.302827e-05,
  .0078125, 1.575662e-05,
  .00390625, 3.939129e-06,
  .001953125, 9.847803e-07,
  .0009765625, 2.461958e-07,
  .00048828125, 6.155171e-08,
  .000244140625, 1.538593e-08,
  .0001220703125, 3.844953e-09,
]

# Crank-Nicolson: dt and L2-error at final time
crank_nicolson = [
  .5, 1.556546e-02,
  .25, 4.021001e-03,
  .125, 1.009290e-03,
  .0625, 2.521688e-04,
  .03125, 6.302999e-05,
  .015625, 1.575673e-05,
  .0078125, 3.939134e-06,
  .00390625, 9.847813e-07,
  .001953125, 2.461951e-07,
  .0009765625, 6.154978e-08,
  .00048828125, 1.538985e-08,
  .000244140625, 3.845783e-09,
  .0001220703125, 9.603409e-10,
]

# L-stable DIRK3: dt and L2-error at final time.  Since the exact
# solution is O(dt**3) I kind of thought this method would get the
# exact solution. In order to actually see 3rd-order convergence, I
# had to try fairly small timesteps.  For smaller values of dt, the
# error is actually comparable to Crank-Nicolson, which was the most
# accurate 2nd-order method for this problem.  The last data point is
# not included here, since round-off error (we are doing 16,384
# timesteps for this dt) starts to turn the line back up. The best
# slope we were able to achieve was therefore -2.96, which is fairly
# close to -3.
lstable_dirk3 = [
  .5, 6.367834e-03,
  .25, 3.199227e-03,
  .125, 1.041283e-03,
  .0625, 2.707764e-04,
  .03125, 6.023444e-05,
  .015625, 1.186832e-05,
  .0078125, 2.101242e-06,
  .00390625, 3.368457e-07,
  .001953125, 4.952904e-08,
  .0009765625, 6.820195e-09,
  .00048828125, 8.997519e-10,
  .000244140625, 1.157869e-10,
  .0001220703125, 1.482280e-11,
  # .00006103515625, 2.138562e-12,
]

# Helper function which plots the results for a single method and does a curve fit.
def plot1(data, base_label, **kwargs):
    xdata = np.log10(np.reciprocal(data[0::2]))
    ydata = np.log10(data[1::2])

    # Compute linear fit of last three points.
    start_fit = len(xdata) - 3
    end_fit = len(xdata)
    fit = np.polyfit(xdata[start_fit:end_fit], ydata[start_fit:end_fit], 1)

    # Make the plot -- unpack the user's additional plotting arguments
    # from kwargs by prepending with **.
    ax1.plot(xdata, ydata, label=base_label + ", $" + "{:.2f}".format(fit[0]) + "$", **kwargs)

fig = plt.figure()
ax1 = fig.add_subplot(111)

# Make the plots
plot1(implicit_euler, "Implicit Euler", color="maroon", marker="v", linestyle=":")
plot1(implicit_midpoint, "Implicit Midpoint", color="midnightblue", marker="+", linestyle="--")
plot1(lstable_dirk2, "L-stable DIRK2", color="blue", marker="o", linestyle="-")
plot1(dirk2, "A-stable DIRK2", color="red", marker="s", linestyle="-")
plot1(bdf2, "BDF2", color="green", marker="x", linestyle="-.")
plot1(crank_nicolson, "Crank-Nicolson", color="black", marker="^", linestyle="--")
plot1(lstable_dirk3, "L-stable DIRK3", color="olivedrab", marker="h", linestyle="-.")

# Set up the axis labels.
ax1.set_xlabel('$\log (\Delta t^{-1})$')
ax1.set_ylabel('$\log \|e(T)\|_{L^2}$')


# Add a legend
plt.legend(loc='lower left', prop=fontP)

# Save a PDF
plt.savefig('implicit_plot.pdf', format='pdf')

# Local Variables:
# python-indent: 2
# End:
