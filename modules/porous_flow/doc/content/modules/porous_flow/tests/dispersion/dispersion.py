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
import sys
import numpy as np
import matplotlib.pyplot as plt
from scipy.special import erfc
import pandas as pd

#
# Diffusion-only test
#
# Read MOOSE simulation data
data = pd.read_csv("../../../../../../test/tests/dispersion/gold/diff01_out_xmass_0021.csv")

# The analytical solution is erfc(u) where u is a similarity variable
x = np.linspace(0,10,100)
t = 20
d = 1
tau = 0.1
D = d*tau
u = x/(2*np.sqrt(D*t))

plt.figure(1)
plt.plot(x, erfc(u), label = 'Analytical')
plt.plot(data.x, data.massfrac0, 'o', label = 'MOOSE')
plt.xlabel('x (m)')
plt.ylabel('Mass fraction (-)')
plt.legend()
plt.title('Mass fraction (t = 50 s)')
plt.ylim([-0.05,1])
plt.savefig("diffusion_fig.png")

#
# Dispersion tests
#
def expected(x,t):
    porosity = 0.3
    alphal = 0.2
    v = 1.05e-3 / porosity
    D = alphal * v
    return 0.5 * erfc((x - v * t)/(2 *np.sqrt(D * t))) + np.sqrt(v * v * t/(np.pi * D)) * \
    np.exp(- (x - v * t)**2/(4 * D * t)) - 0.5 * (1 + v * x / D + v * v * t / D) * np.exp(v * x / D) *\
    erfc((x+v*t)/(2*np.sqrt(D*t)))

# Read MOOSE simulation data
data = pd.read_csv("../../../../../../test/tests/dispersion/gold/disp01_out_xmass_0029.csv")
data_heavy = pd.read_csv("../../../../../../test/tests/dispersion/gold/disp01_heavy_out_xmass_0105.csv")

plt.figure(2)
plt.plot(data.x, data.massfrac0, 'b.', label = 'MOOSE')
plt.plot(data_heavy.x, data_heavy.massfrac0, 'g+', label = 'MOOSE (heavy)')
plt.plot(x, expected(x, 1e3), 'k-', label = 'Analytical')
plt.xlabel('x (m)')
plt.ylabel('Mass fraction (-)')
plt.legend()
plt.title('Mass fraction (t = 1000 s)')
plt.ylim([-0.05,1])
plt.savefig("dispersion_fig.png")

sys.exit(0)
