#!/usr/bin/env python

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

# Analytical form of van Genuchten's relative permeability
def vg(s, sr, sls, m):

    # Effective saturation
    seff = np.clip((s - sr) / (sls - sr), 0, 1)

    # The liquid relative permeability is
    relperm = np.sqrt(seff) * (1 - np.power(1 - np.power(seff, 1.0 / m), m))**2

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
data = np.genfromtxt('../../tests/relperm/corey1_out_vpp_0001.csv', delimiter = ',', names = True, dtype = None)

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
data = np.genfromtxt('../../tests/relperm/corey2_out_vpp_0001.csv', delimiter = ',', names = True, dtype = None)

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
data = np.genfromtxt('../../tests/relperm/corey3_out_vpp_0001.csv', delimiter = ',', names = True, dtype = None)

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
data = np.genfromtxt('../../tests/relperm/vangenuchten1_out_vpp_0001.csv', delimiter = ',', names = True, dtype = None)

plt.figure(4)
plt.plot(s0, vg(s0, 0, 1, 0.5), label = 'kr0')
plt.plot(data['s0aux'], data['kr0aux'], 'ob', label = 'kr0 (MOOSE)')
plt.plot(s0, corey(1 - s0, 0, 0, 2), label = 'kr1')
plt.plot(data['s0aux'], data['kr1aux'], 'og', label = 'kr1 (MOOSE)')
plt.xlabel('Phase 0 saturation (-)')
plt.ylabel('Relative permeability (-)')
plt.legend(loc = 'best')
plt.title('van Genuchten relative permeability: $S_{0r} = 0, S_{1r} = 0, m = 0.5$')
plt.ylim([-0.01, 1.01])
plt.savefig("vg1_fig.pdf")

# Case 2: residual saturation set to 0.25 for phase 0, 0 for phase 1, m = 0.4, sls = 1
#
# Read MOOSE simulation data
data = np.genfromtxt('../../tests/relperm/vangenuchten2_out_vpp_0001.csv', delimiter = ',', names = True, dtype = None)

plt.figure(5)
plt.plot(s0, vg(s0, 0.25, 1, 0.4), label = 'kr0')
plt.plot(data['s0aux'], data['kr0aux'], 'ob', label = 'kr0 (MOOSE)')
plt.plot(s0, corey(1 - s0, 0, 0.25, 2), label = 'kr1')
plt.plot(data['s0aux'], data['kr1aux'], 'og', label = 'kr1 (MOOSE)')
plt.xlabel('Phase 0 saturation (-)')
plt.ylabel('Relative permeability (-)')
plt.legend(loc = 'best')
plt.title('van Genuchten relative permeability: $S_{0r} = 0.25, S_{1r} = 0, m = 0.4$')
plt.ylim([-0.01, 1.01])
plt.savefig("vg2_fig.pdf")
