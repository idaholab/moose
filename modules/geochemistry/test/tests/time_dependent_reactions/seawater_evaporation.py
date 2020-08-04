#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

# Plotting the results of seawater_evaporation_no_flow_through_out.csv and seawater_evaporation_flow_through_out.csv and the equivalent GWB simulations

import os
import sys
import matplotlib.pyplot as plt

# no flow-through

f = open("gold/seawater_evaporation_no_flow_through_out.csv", "r")
data = [list(map(float, line.strip().split(","))) for line in f.readlines()[2:]]
f.close()
dolomite = [x[1] for x in data]
gypsum = [x[2] for x in data]
halite = [x[3] for x in data]
mirabilite = [x[4] for x in data]
mass_h2o = [x[5] for x in data]

gwb_solvent_mass = [1.0, 1.0, 0.99903, 0.90093, 0.80185, 0.70277, 0.60368, 0.50460, 0.40552, 0.30643, 0.20735, 0.10827, 0.057556, 0.045903, 0.028233, 0.0068083]
gwb_dolomite = [0, 0, 5.022e-6, 0.0004989, 0.0009910, 0.001476, 0.001953, 0.002421, 0.002881, 0.003329, 0.003763, 0.004160, 0.004378, 0.004459, 0.004728, 0.004933]
gwb_mirabilite = [0, 0, 0,0,0,0,0,0,0,0,0,0, 0.2177, 2.346, 3.344, 2.532]
gwb_halite = [0, 0, 0,0,0,0,0,0,0,0,0,0,0, 0.2087, 5.049, 11.15]
gwb_gypsum = [0, 0, 0,0,0,0,0,0,0,0,0,0,0, 0, 0.01633, 0.6214]

plt.figure(0)
plt.loglog([1] + mass_h2o, [0] + dolomite, 'k-', label = "Dolomite (MOOSE)")
plt.loglog(mass_h2o, mirabilite, 'r-', label = "Mirabilite (MOOSE)")
plt.loglog(mass_h2o, halite, 'g-', label = "Halite (MOOSE)")
plt.loglog(mass_h2o, gypsum, 'b-', label = "Gypsum (MOOSE)")
plt.loglog(gwb_solvent_mass, gwb_dolomite, 'ks', label = "Dolomite (GWB)")
plt.loglog(gwb_solvent_mass, gwb_mirabilite, 'rs', label = "Mirabilite (GWB)")
plt.loglog(gwb_solvent_mass, gwb_halite, 'gs', label = "Halite (GWB)")
plt.loglog(gwb_solvent_mass, gwb_gypsum, 'bs', label = "Gypsum (GWB)")
ax = plt.gca()
ax.set_xlim(ax.get_xlim()[::-1])
plt.legend()
plt.xlabel("Mass solvent H$_{2}$O (kg)")
plt.ylabel("Precipitate volume (cm$^{3}$)")
plt.title("Minerals precipitated: evaporating seawater (no flow-through)");
plt.savefig("../../../doc/content/media/geochemistry/seawater_evaporation_1.png")

# with flow-through

f = open("gold/seawater_evaporation_flow_through_out.csv", "r")
data = [list(map(float, line.strip().split(","))) for line in f.readlines()[2:]]
f.close()
dolomite = [x[1] * 64.265 for x in data]
dolomite = [x - dolomite[0] for x in dolomite]
gypsum = [x[2] * 74.69 for x in data]
halite = [x[3] * 27.015 for x in data]
mirabilite = [x[4] * 219.8 for x in data]
mass_h2o = [x[5] for x in data]

gwb_solvent_mass = [1.0, 1.0, 0.99903, 0.90093, 0.80185, 0.70277, 0.60368, 0.50460, 0.40552, 0.30643, 0.20735, 0.10827, 0.057556, 0.045903, 0.028233, 0.027230, 0.0061634]
gwb_dolomite = [0, 0, 5.022e-6, 0.0004989, 0.0009910, 0.001476, 0.001953, 0.002421, 0.002881, 0.003329, 0.003763, 0.004160, 0.004378, 0.004459, 0.004728, 0.004739, 0.004946]
gwb_mirabilite = [0, 0, 0,0,0,0,0,0,0,0,0,0, 0.2177, 2.346, 3.344, 3.344, 3.344]
gwb_halite = [0, 0, 0,0,0,0,0,0,0,0,0,0,0, 0.2087, 5.049, 5.334, 11.17]
gwb_gypsum = [0, 0, 0,0,0,0,0,0,0,0,0,0,0, 0, 0.01633, 0.04210, 0.5795]

plt.figure(1)
plt.loglog([1] + mass_h2o, [0] + dolomite, 'k-', label = "Dolomite (MOOSE)")
plt.loglog(mass_h2o, mirabilite, 'r-', label = "Mirabilite (MOOSE)")
plt.loglog(mass_h2o, halite, 'g-', label = "Halite (MOOSE)")
plt.loglog(mass_h2o, gypsum, 'b-', label = "Gypsum (MOOSE)")
plt.loglog(gwb_solvent_mass, gwb_dolomite, 'ks', label = "Dolomite (GWB)")
plt.loglog(gwb_solvent_mass, gwb_mirabilite, 'rs', label = "Mirabilite (GWB)")
plt.loglog(gwb_solvent_mass, gwb_halite, 'gs', label = "Halite (GWB)")
plt.loglog(gwb_solvent_mass, gwb_gypsum, 'bs', label = "Gypsum (GWB)")
ax = plt.gca()
ax.set_xlim(ax.get_xlim()[::-1])
plt.legend()
plt.xlabel("Mass solvent H$_{2}$O (kg)")
plt.ylabel("Precipitate volume (cm$^{3}$)")
plt.title("Minerals precipitated: evaporating seawater (with flow-through)");
plt.savefig("../../../doc/content/media/geochemistry/seawater_evaporation_2.png")

sys.exit(0)
