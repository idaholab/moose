#!/usr/bin/env python
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

# Implicit Euler is the canonical first-order time integration method
implicit_midpoint = [
  0.0625, 3.613887e-03,
  0.03125, 9.034762e-04,
  0.015625, 2.258694e-04,
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
plot1(implicit_midpoint, "Implicit Midpoint", color="midnightblue", marker="+", linestyle="--")

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
