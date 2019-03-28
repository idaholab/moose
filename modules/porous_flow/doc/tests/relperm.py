#!/usr/bin/env python2
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

# Script to generate plots used in documentation from test results

import numpy as np
import matplotlib.pyplot as plt

# Analytical form of Corey's relative permeability curve
def corey(s, sr, sum_s_r, n):
    # Effective saturation
    seff = np.clip((s - sr) / (1.0 - sum_s_r), 0, 1)

    # Relative permeability is then
    relperm = np.power(seff, n);

    return relperm

# Analytical form of van Genuchten's wetting relative permeability
def vg(s, sr, sls, m):

    # Effective saturation
    seff = np.clip((s - sr) / (sls - sr), 0, 1)

    # The liquid relative permeability is
    relperm = np.sqrt(seff) * (1 - np.power(1 - np.power(seff, 1.0 / m), m))**2

    # Relative permeability is then
    return relperm

# Analytical form of van Genuchten's nonwetting relative permeability
def vg_nw(s, sr, sls, m):

    # Effective saturation
    seff = np.clip((s - sr) / (sls - sr), 0, 1)

    # The gas relative permeability is
    relperm = np.sqrt(seff) * np.power(1 - np.power(1 - seff, 1.0 / m), 2.0 * m)

    # Relative permeability is then
    return relperm

# Saturation of phase 0 varies linearly from 0 to 1
s0 = np.linspace(0, 1, 200)

################################################################################
#
# Corey relative permeabilities
#
# Case 1: residual saturation set to 0 for both phases, n = 1
#
# Read MOOSE simulation data
data = np.genfromtxt('../../test/tests/relperm/gold/corey1_out_vpp_0001.csv', delimiter = ',', names = True, dtype = None)

plt.figure(1)
plt.plot(s0, corey(s0, 0, 0, 1), label = 'kr0')
plt.plot(data['s0aux'], data['kr0aux'], 'ob', label = 'kr0 (MOOSE)')
plt.plot(s0, corey(1 - s0, 0, 0, 1), label = 'kr1')
plt.plot(data['s0aux'], data['kr1aux'], 'og', label = 'kr1 (MOOSE)')
plt.xlabel('Phase 0 saturation (-)')
plt.ylabel('Relative permeability (-)')
plt.legend(loc = 'best')
plt.title('Corey relative permeability: $S_{0r} = 0, S_{1r} = 0, n = 1$')
plt.savefig("corey1_fig.pdf")

# Case 2: residual saturation set to 0 for both phases, and n = 2
#
# Read MOOSE simulation data
data = np.genfromtxt('../../test/tests/relperm/gold/corey2_out_vpp_0001.csv', delimiter = ',', names = True, dtype = None)

plt.figure(2)
plt.plot(s0, corey(s0, 0, 0, 2), label = 'kr0')
plt.plot(data['s0aux'], data['kr0aux'], 'ob', label = 'kr0 (MOOSE)')
plt.plot(s0, corey(1 - s0, 0, 0, 2), label = 'kr1')
plt.plot(data['s0aux'], data['kr1aux'], 'og', label = 'kr1 (MOOSE)')
plt.xlabel('Phase 0 saturation (-)')
plt.ylabel('Relative permeability (-)')
plt.legend(loc = 'best')
plt.title('Corey relative permeability: $S_{0r} = 0, S_{1r} = 0, n = 2$')
plt.savefig("corey2_fig.pdf")

# Case 3: residual saturation set to 0.2 for phase 0, 0.3 for phase 1 and n = 2
#
# Read MOOSE simulation data
data = np.genfromtxt('../../test/tests/relperm//gold/corey3_out_vpp_0001.csv', delimiter = ',', names = True, dtype = None)

plt.figure(3)
plt.plot(s0, corey(s0, 0.2, 0.5, 2), label = 'kr0')
plt.plot(data['s0aux'], data['kr0aux'], 'ob', label = 'kr0 (MOOSE)')
plt.plot(s0, corey(1 - s0, 0.3, 0.5, 2), label = 'kr1')
plt.plot(data['s0aux'], data['kr1aux'], 'og', label = 'kr1 (MOOSE)')
plt.xlabel('Phase 0 saturation (-)')
plt.ylabel('Relative permeability (-)')
plt.legend(loc = 'best')
plt.title('Corey relative permeability: $S_{0r} = 0, S_{1r} = 0, n = 2$')
plt.ylim([-0.01, 1.01])
plt.savefig("corey3_fig.pdf")

################################################################################
#
# van Genuchten relative permeabilities
#
# Case 1: residual saturation set to 0 for both phases, m = 0.5, sls = 1
#
# Read MOOSE simulation data
data = np.genfromtxt('../../test/tests/relperm/gold/vangenuchten1_out_vpp_0001.csv', delimiter = ',', names = True, dtype = None)

plt.figure(4)
plt.plot(s0, vg(s0, 0, 1, 0.5), label = 'kr_w')
plt.plot(data['s0aux'], data['kr0aux'], 'ob', label = 'kr_w (MOOSE)')
plt.plot(s0, vg_nw(1 - s0, 0, 1, 0.5), label = 'kr_nw')
plt.plot(data['s0aux'], data['kr1aux'], 'og', label = 'kr_nw (MOOSE)')
plt.xlabel('Wetting phase saturation (-)')
plt.ylabel('Relative permeability (-)')
plt.legend(loc = 'best')
plt.title('van Genuchten relative permeability: $S_{w,res} = 0, S_{nw,res} = 0, m = 0.5$')
plt.ylim([-0.01, 1.01])
plt.savefig("vg1_fig.pdf")

# Case 2: residual saturation set to 0.1 for phase 0, 0.2 for phase 1, m = 0.4
#
# Read MOOSE simulation data
data = np.genfromtxt('../../test/tests/relperm/gold/vangenuchten2_out_vpp_0001.csv', delimiter = ',', names = True, dtype = None)

plt.figure(5)
plt.plot(s0, vg(s0, 0.1, 0.8, 0.4), label = 'kr_w')
plt.plot(data['s0aux'], data['kr0aux'], 'ob', label = 'kr_w (MOOSE)')
plt.plot(s0, vg_nw(1 - s0, 0.2, 0.9, 0.4), label = 'kr_nw')
plt.plot(data['s0aux'], data['kr1aux'], 'og', label = 'kr_nw (MOOSE)')
plt.xlabel('Phase 0 saturation (-)')
plt.ylabel('Relative permeability (-)')
plt.legend(loc = 'best')
plt.title('van Genuchten relative permeability: $S_{w,res} = 0.1, S_{nw,res} = 0.2, m = 0.4$')
plt.ylim([-0.01, 1.01])
plt.savefig("vg2_fig.pdf")
plt.savefig("../../doc/content/media/porous_flow/relperm_vg.png")
