#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

# Plotting the results of bio_zoning

import os
import sys
import math
import matplotlib.pyplot as plt
import matplotlib.animation as animation
import glob

steps = glob.glob("bio_zoning_flow_out_react0_csv_data_*.csv")
steps = sorted([int(s.split("bio_zoning_flow_out_react0_csv_data_")[1][:-4]) for s in steps])

with open("bio_zoning_flow_out_react0_csv.csv", "r") as f:
    tims = [int(float(line.split(",")[0])) for line in f.readlines()[1:]]

fig, (ax1, ax2) = plt.subplots(2, 1)

def animate(idx):
    fn = "bio_zoning_flow_out_react0_csv_data_" + str(idx * 10).zfill(4) + ".csv"
    if idx == len(tims) - 1:
        fn = "bio_zoning_flow_out_react0_csv_data_" + str(steps[-1]).zfill(4) + ".csv"
    f = open(fn, "r")
    data = [list(map(float, line.strip().split(","))) for line in f.readlines()[1:-1]]
    f.close()
    xvals = [x[6] / 1000 for x in data]
    methanogen = [x[0] * 1000 for x in data]
    sr = [x[1] * 1000 for x in data]
    acetate = [x[3] * 1E6 for x in data]
    methane = [x[4] * 1E6 for x in data]
    so4 = [x[5] * 1E6 for x in data]

    ax1.clear()
    ax1.semilogy(xvals, acetate, label = 'CH$_{3}$COO$^{-}$')
    ax1.semilogy(xvals, methane, label = 'CH$_{4}$(aq)')
    ax1.semilogy(xvals, so4, label = 'SO$_{4}^{2-}$')
    ax1.legend(loc = 'lower right')
    ax1.set_ylabel("Total conc ($\mu$molal)")
    ax1.set_title("Aquifer zoning.  Time = " + str(tims[idx]) + "$\,$yr")
    ax1.set_ylim([1E-1, 1E2])
    ax1.set_xlim([0, 200])
    ax1.grid()

    ax1.annotate('', xy=(24, 0.15), xytext=(10, 0.15), arrowprops=dict(width = 2.5, headwidth = 7.0, headlength = 10.0, facecolor='b'),)
    ax1.text(25, 0.15, "flow", verticalalignment = 'center')

    ax2.clear()
    ax2.plot(xvals, methanogen, label = 'methanogen')
    ax2.plot(xvals, sr, label = 'sulfate reducer')
    ax2.legend(loc = 'upper right')
    ax2.set_xlabel("distance (km)")
    ax2.set_ylabel("Biomass conc ($\mu$g/kg)")
    ax2.set_ylim([0, 2.5])
    ax2.set_xlim([0, 200])
    ax2.grid()

    ax2.arrow(10, 0.15, 9, 0, width = 0.04, head_length = 5.0, head_width = 0.15, facecolor = 'b')
    ax2.text(25, 0.15, "flow", verticalalignment = 'center')


ani = animation.FuncAnimation(fig, animate, frames = range(len(tims)), repeat = False)
plt.show()
ani.save("../../../doc/content/media/geochemistry/bio_zoning.mp4", writer = animation.FFMpegWriter(fps = 5))
plt.close()

with open("bio_zoning_flow_out_react0_csv_data_" + str(steps[-1]).zfill(4) + ".csv", "r") as f:
    data = [list(map(float, line.strip().split(","))) for line in f.readlines()[1:-1]]
xvals = [x[6] / 1000 for x in data]
methanogen = [x[0] * 1000 for x in data]
sr = [x[1] * 1000 for x in data]
acetate = [x[3] * 1E6 for x in data]
methane = [x[4] * 1E6 for x in data]
so4 = [x[5] * 1E6 for x in data]

plt.figure()
plt.plot(xvals, methanogen, label = 'methanogen')
plt.plot(xvals, sr, label = 'sulfate reducer')
plt.xlim([0, 200])
plt.ylim([0, 2])
plt.legend()
plt.xlabel("Distance (km)")
plt.ylabel("Biomass concentration ($\mu$g/kg)")
plt.title("Aquifer zoning.  Time = " + str(tims[-1]) + "$\,$yr")
plt.arrow(10, 0.15, 9, 0, width = 0.02, head_length = 5.0, head_width = 0.075, facecolor = 'b')
plt.text(25, 0.15, "flow", verticalalignment = 'center')
plt.grid()
plt.savefig("../../../doc/content/media/geochemistry/bio_zoning_biomass.png")
plt.show()
plt.close()

plt.figure()
plt.semilogy(xvals, acetate, label = 'CH$_{3}$COO$^{-}$')
plt.semilogy(xvals, methane, label = 'CH$_{4}$(aq)')
plt.semilogy(xvals, so4, label = 'SO$_{4}^{2-}$')
plt.legend()
plt.xlabel("distance (km)")
plt.ylabel("Total concentration ($\mu$molal)")
plt.title("Aquifer zoning.  Time = " + str(tims[-1]) + "$\,$yr")
plt.ylim([1E-1, 1E2])
plt.xlim([0, 200])
plt.annotate('', xy=(24, 0.15), xytext=(10, 0.15), arrowprops=dict(width = 2.5, headwidth = 7.0, headlength = 10.0, facecolor='b'),)
plt.text(25, 0.15, "flow", verticalalignment = 'center')
plt.grid()
plt.savefig("../../../doc/content/media/geochemistry/bio_zoning_conc.png")
plt.show()
plt.close()
sys.exit(0)
