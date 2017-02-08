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

# Explicit methods - errors reported are L2-errors at time T=1.
# Note: for reasons of expediency, this does not match the end_time
# used in explicit_convergence.i.

# Explicit Euler has the same timestep restriction as
# Heun/Ralston/ExplicitMidpoint, but is first-order accurate.
explicit_euler = [
  0.00390625, 1.239474e-03,
  0.001953125, 6.192684e-04,
  0.0009765625, 3.095172e-04,
  0.00048828125, 1.547293e-04,
  0.000244140625, 7.735736e-05,
  0.0001220703125, 3.867685e-05,
  0.00006103515625, 1.933797e-05,
]

# Results of the Explicit Midpoint tests, end_time=1.0
explicit_midpoint = [
  0.00390625, 1.524644e-05,
  0.001953125, 3.551330e-06,
  0.0009765625, 8.643185e-07,
  0.00048828125, 2.134526e-07,
  0.000244140625, 5.305066e-08,
  0.0001220703125, 1.322452e-08,
  0.00006103515625, 3.301415e-09,
]

# Results of the Heun tests, end_time=1.0
heun = [
  0.00390625, 3.168337e-05,
  0.001953125, 7.406706e-06,
  0.0009765625, 1.805047e-06,
  0.00048828125, 4.460446e-07,
  0.000244140625, 1.108902e-07,
  0.0001220703125, 2.764669e-08,
  0.00006103515625, 6.902309e-09,
]

# Results of the Ralston tests, end_time=1.0
ralston = [
  0.00390625, 2.071600e-05,
  0.001953125, 4.834397e-06,
  0.0009765625, 1.177414e-06,
  0.00048828125, 2.908673e-07,
  0.000244140625, 7.230200e-08,
  0.0001220703125, 1.802485e-08,
  0.00006103515625, 4.499967e-09,
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
