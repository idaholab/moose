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

# Explicit methods

# Explicit Euler has the same timestep restriction as
# Heun/Ralston/ExplicitMidpoint, but is first-order accurate.
explicit_euler = [
  0.00390625, 5.540655e-06,
  0.001953125, 2.779391e-06,
  0.0009765625, 1.391873e-06,
]

# Results of the Explicit Midpoint tests, end_time=0.03125
explicit_midpoint = [
  0.00390625, 1.814894e-07,
  0.001953125, 4.039132e-08,
  0.0009765625, 9.652859e-09,
]

# Results of the Heun tests, end_time=0.03125
heun = [
  0.00390625, 6.212935e-07,
  0.001953125, 1.437959e-07,
  0.0009765625, 3.486753e-08,
]

# Results of the Ralston tests, end_time=0.03125
ralston = [
  0.00390625, 3.029619e-07,
  0.001953125, 6.871244e-08,
  0.0009765625, 1.654343e-08,
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
plot1(explicit_euler, "Explicit Euler", color="darkmagenta", marker="*", linestyle="--")
plot1(explicit_midpoint, "Explicit Midpoint", color="chocolate", marker="4", linestyle="-")
plot1(heun, "Heun", color="lightslategray", marker="d", linestyle="-.")
plot1(ralston, "Ralston", color="steelblue", marker="p", linestyle=":")

# Set up the axis labels.
ax1.set_xlabel('$\log (\Delta t^{-1})$')
ax1.set_ylabel('$\log \|e(T)\|_{L^2}$')


# Add a legend
plt.legend(loc='lower left', prop=fontP)

# Save a PDF
plt.savefig('explicit_plot.pdf', format='pdf')

# Local Variables:
# python-indent: 2
# End:
