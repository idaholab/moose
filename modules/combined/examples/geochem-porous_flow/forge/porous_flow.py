#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

# Plotting the results of porous_flow.i

import os
import sys
import matplotlib.pyplot as plt

f = open("gold/porous_flow_highres_out.csv", "r")
header = f.readline().strip().split(",")
data = [list(map(float, line.strip().split(","))) for line in f.readlines()[1:]]
f.close()

index = {}
for i in range(len(header)):
    index[header[i]] = i
months = [d[index["time"]] / 3600.0 / 24.0 / 30.0 for d in data]
seconds = [0] + [d[index["time"]] for d in data] # used for determining rates below
temperature = [d[index["approx_production_temperature"]] for d in data]
mass_rate = [d[index["mass_extracted"]] for d in data]
for i in range(len(mass_rate)):
    mass_rate[i] = mass_rate[i] / (seconds[i + 1] - seconds[i])
k_rate = [d[index["mass_extracted_SiO2"]] for d in data]
for i in range(len(k_rate)):
    k_rate[i] = k_rate[i] / (seconds[i + 1] - seconds[i])
k_percentage = [100 * k_rate[i] / mass_rate[i] if mass_rate[i] != 0 else 0.0 for i in range(len(mass_rate))]

# Plot the production temperature
plt.figure(0)
plt.plot(months, temperature)
plt.ylabel("Temperature ($^{\circ}$C)")
plt.xlabel("Months")
plt.title("Production temperature")
plt.tight_layout()
plt.savefig("../../../../geochemistry/doc/content/media/geochemistry/forge_production_temperature.png")

# Plot the production rate
plt.figure(1)
plt.plot(months, mass_rate)
plt.ylabel("Mass rate (kg.s$^{-1}$)")
plt.xlabel("Months")
plt.title("Production rate")
plt.tight_layout()
plt.savefig("../../../../geochemistry/doc/content/media/geochemistry/forge_production_rate.png")

# Plot the K+ rate
plt.figure(2)
plt.plot(months, k_percentage, label = 'produced fluid')
plt.plot(months, [2.69E-2 for m in months], 'k--', label = 'in-situ')
plt.plot(months, [1.64E-3 for m in months], 'k:', label = 'injectate')
plt.legend()
plt.ylabel("Fraction of SiO2(aq) (%)")
plt.xlabel("Months")
plt.title("Mass-fraction of SiO2(aq) in produced fluid")
plt.tight_layout()
plt.savefig("../../../../geochemistry/doc/content/media/geochemistry/forge_production_rate_SiO2.png")
plt.show()

sys.exit(0)
