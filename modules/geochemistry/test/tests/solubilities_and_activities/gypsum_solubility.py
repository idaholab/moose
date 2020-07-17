#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

# Plotting the results of gypsum_solubility and the equivalent GWB simulation

import os
import sys
import matplotlib.pyplot as plt

f = open("gold/gypsum_solubility_out.csv", "r")
data = f.readlines()[2:]
f.close()
cl = [float(line.strip().split(",")[1]) for line in data]
gyp = [float(line.strip().split(",")[2]) for line in data]

gwb_cl_molality = [0.02907, 0.2894, 0.5768, 0.8625, 1.146,  1.428,  1.708, 1.986, 2.261, 2.533, 2.803]
gwb_ca_in_fluid = [0.02386, 0.0417, 0.0559, 0.0682, 0.0796, 0.0904, 0.101, 0.111, 0.121, 0.131, 0.141]

plt.figure()
plt.plot(cl, gyp, 'k-', linewidth = 2.0, label = 'MOOSE')
plt.plot(gwb_cl_molality, gwb_ca_in_fluid, 'rs', markersize = 6.0, label = 'GWB')
plt.legend()
plt.xlabel("Cl molality")
plt.ylabel("Dissolved gypsum (mol)")
plt.title("Gypsum solubility in brine")
plt.savefig("../../../doc/content/media/geochemistry/gypsum_solubility.png")

sys.exit(0)
