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

# Use fonts that match LaTeX
from matplotlib import rcParams
rcParams['font.family'] = 'serif'
rcParams['font.size'] = 17
rcParams['font.serif'] = ['Computer Modern Roman']
rcParams['text.usetex'] = True

# Small font size for the legend.  You must pass prop=fontP to the
# legend command to make it use this. Sizes are xx-small, x-small,
# small, medium, large, x-large, and xx-large.
from matplotlib.font_manager import FontProperties
fontP = FontProperties()
fontP.set_size('small')

fig = plt.figure()
ax1 = fig.add_subplot(111)

# Results for Jeffery-Hamel flow problem, alpha=15deg, Re=30,
# Dirichlet and Natural outflow BCs.  Note that the numerical solution
# *does not* converge to the true solution in the natural BC case, and
# there is no a-priori reason that it should... it has different
# boundary conditions.  This merely shows that although the natural BC
# outlet may look fine, it is not going to magically give you the
# "right" answer based on the other Dirichlet BCs.

# 1/h,   u1-Dirichlet, u2-Dirichlet, p-Dirichlet,  u1-Natural,   u2-Natural
data = [
  6,     4.593313e-01, 1.411345e-01, 5.205426e+01, 4.524177e-01, 2.371472e-01,
  12,    5.400647e-02, 1.517583e-02, 1.179398e+01, 1.651529e-01, 2.011751e-01,
  24,    6.734848e-03, 1.681848e-03, 2.953409e+00, 1.654026e-01, 2.016113e-01,
  48,    8.400521e-04, 2.035659e-04, 7.410616e-01, 1.658418e-01, 2.016884e-01,
  96,    1.107319e-04, 2.562933e-05, 1.840373e-01, 1.658707e-01, 2.016973e-01,
  ]

xdata = np.log10(data[0::6])
vel_x_dirichlet = np.log10(data[1::6])
vel_y_dirichlet = np.log10(data[2::6])
p_dirichlet = np.log10(data[3::6])
vel_x_natural = np.log10(data[4::6])
vel_y_natural = np.log10(data[5::6])

# Compute linear curve fits
fit_x = np.polyfit(xdata, vel_x_dirichlet, 1)
fit_y = np.polyfit(xdata, vel_y_dirichlet, 1)
fit_p = np.polyfit(xdata, p_dirichlet, 1)

ax1.plot(xdata, vel_x_dirichlet, color="red", marker="o", linestyle="-", linewidth=2, label=r'Dirichlet BC, $u_1$')
ax1.plot(xdata, vel_y_dirichlet, color="blue", marker="s", linestyle="-", linewidth=2, label=r'Dirichlet BC, $u_2$')
ax1.plot(xdata, p_dirichlet, color="lightseagreen", marker="8", linestyle="-", linewidth=2, label=r'Dirichlet BC, $p$')

# You *can* plot these, but they do not converge to the exact solution.
# ax1.plot(xdata, vel_x_natural, color="darkgreen", marker="v", linestyle="-", linewidth=2, label=r'Natural BC, $u_1$')
# ax1.plot(xdata, vel_y_natural, color="orange", marker="H", linestyle="-", linewidth=2, label=r'Natural BC, $u_2$')

# Put the slope on the graph.  We don't need to put both since they
# are both approximately the same (although u2 is a little steeper
# than the theoretical value).
ax1.text(1.4, -2.0, 'Slope $\\approx {:.2f}$'.format(fit_x[0]))
ax1.text(1.4,  0.7, 'Slope $\\approx {:.2f}$'.format(fit_p[0]))

# Label the graph
ax1.set_xlabel(r'$\log (1/h)$')
ax1.set_ylabel(r'$\log (L^2 \mathrm{-error})$')
ax1.legend(loc='lower left', prop=fontP)

# Create PDF
plt.savefig('jeffery_hamel.pdf', format='pdf')

# Local Variables:
# python-indent: 2
# truncate-lines: t
# End:
