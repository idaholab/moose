#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

# Plotting the results of flushing.i

import os
import sys
import matplotlib.pyplot as plt

f = open("gold/flushing_out.csv", "r")
data = [list(map(float, line.strip().split(","))) for line in f.readlines()[2:]]
f.close()
tim = [x[0] for x in data]
analcime = [x[1] for x in data]
analcime = [x - analcime[0] for x in analcime]
calcite = [x[2]  for x in data]
calcite = [x - calcite[0] for x in calcite]
dawsonite = [x[3]  for x in data]
dolomite = [x[4]  for x in data]
dolomite = [x - dolomite[0] for x in dolomite]
gibbsite = [x[5]  for x in data] # may as well forget this - it's really small
kaolinite = [x[6]  for x in data]
kaolinite = [x - kaolinite[0] for x in kaolinite]
muscovite = [x[7] for x in data]
muscovite = [x - muscovite[0] for x in muscovite]
paragonite = [x[8]  for x in data]
phlogopite = [x[9]  for x in data]
quartz = [x[10]  for x in data]
quartz = [x - quartz[0] for x in quartz]
ph = [x[11]  for x in data]

gwb_tim = [0, 0.668029, 1.022, 2, 2.4, 3.98743, 4, 4.07116, 4.13541, 4.35716, 4.44707, 4.46983, 6, 8, 10, 12, 14, 16, 18, 19.8911, 20]
gwb_ph = [5.000, 6.478, 6.559, 6.776, 6.858, 7.129, 7.182, 8.585, 8.597, 9.354, 9.366, 9.371, 9.519, 9.606, 9.637, 9.648, 9.652, 9.653, 9.653, 9.654, 9.641]
gwb_analcime = [0,0,0,0,0,0,0,0,0,0,0, 0.1749, 19.32, 45.85, 73.36, 101.2, 129.3, 157.3, 185.4, 211.9, 212.6]
gwb_calcite = [365.0, 367.7, 367.8, 367.9, 368.0, 369.4, 369.5, 370.2, 370.4, 371.9, 372.5, 372.7, 381.7, 392.8, 403.5, 414.1, 424.7, 435.3, 445.8, 455.8, 456.4]
gwb_calcite = [x - gwb_calcite[0] for x in gwb_calcite]
gwb_dawsonite = [0,0,0,0,0,0,0,0,0, 0.5767, 1.553, 1.682, 13.48, 27.24, 40.33, 53.22, 66.04, 78.84, 91.65, 103.8, 104.0]
gwb_dolomite = [235.0, 235.2, 235.2, 235.1, 234.9, 233.7, 233.6, 232.9, 232.7, 230.2, 229.0, 228.7, 213.0, 193.7, 175.0, 156.5, 138.1, 119.8, 101.4, 83.97, 82.97]
gwb_dolomite = [x - gwb_dolomite[0] for x in gwb_dolomite]
gwb_gibbsite = [0, 0, 0, 0, 0.04492, 0.5325, 0.5962, 0.9920, 0.4395, 0.3989, 0, 0,0,0,0,0,0,0,0,0,0]
gwb_kaolinite = [120.0 - 120.0, 117.7 - 120.0, 104.5 - 120.0, 68.39 - 120.0, 54.12 - 120.0, -120, -120, -120, -120, -120, -120, -120, -120, -120, -120, -120, -120, -120, -120, -120, -120]
gwb_muscovite = [180.0, 181.9, 181.9, 181.9, 181.9, 181.9, 181.9, 181.9, 180.0, 178.2, 177.3, 177.1, 165.6, 151.5, 137.9, 124.4, 111.0, 97.61, 84.19, 71.50, 70.77]
gwb_muscovite = [x - gwb_muscovite[0] for x in gwb_muscovite]
gwb_paragonite = [0, 0.3061, 12.03, 44.06, 56.66, 103.9, 103.8, 103.2, 105.3, 106.1, 106.5, 106.4, 96.09, 82.56, 68.80, 54.93, 41.01, 27.09, 13.16, 0, 0]
gwb_phlogopite = [0,0,0,0,0,0,0, 0.005093, 1.954, 3.916, 4.853, 5.043, 17.26, 32.22, 46.70, 61.02, 75.29, 89.55, 103.8, 117.3, 118.1]
gwb_quartz = [0, -0.002127, -0.004058, -0.03795, -0.06614, -0.2105, -0.2119, -0.2295, -0.2829, -0.6745, -0.8675, -0.9168, -4.566, -10.01, -15.80, -21.71, -27.66, -33.61, -39.57, -45.19, -45.51]


plt.figure(0)
plt.plot([0] + tim, [5] + ph, 'k-', linewidth = 2.0, label = 'MOOSE')
plt.plot(gwb_tim, gwb_ph, 'ks', linewidth = 2.0, label = 'GWB')
plt.legend()
plt.xlabel("Time (days)")
plt.ylabel("pH")
plt.title("pH during alkali flooding")
plt.savefig("../../../doc/content/media/geochemistry/flushing_1.png")

plt.figure(3)
plt.plot(tim, quartz, 'k-', linewidth = 2.0, label = 'MOOSE')
plt.plot(gwb_tim, gwb_quartz, 'ks', linewidth = 2.0, label = 'GWB')
plt.legend()
plt.xlabel("Time (days)")
plt.ylabel("Quartz volume change (cm$^{3}$)")
plt.title("Kinetic quartz dissolution during alkali flooding")
plt.savefig("../../../doc/content/media/geochemistry/flushing_4.png")

plt.figure(1)
plt.plot(tim, analcime, 'k-', linewidth = 2.0, label = 'Analcime (MOOSE)')
plt.plot(tim, calcite, 'r-', linewidth = 2.0, label = 'Calcite (MOOSE)')
plt.plot(tim, dawsonite, 'g-', linewidth = 2.0, label = 'Dawsonite (MOOSE)')
plt.plot(tim, paragonite, 'b-', linewidth = 2.0, label = 'Paragonite (MOOSE)')
plt.plot(tim, phlogopite, 'y-', linewidth = 2.0, label = 'Phlogopite (MOOSE)')
plt.plot(gwb_tim, gwb_analcime, 'ks', linewidth = 2.0, label = 'Analcime (GWB)')
plt.plot(gwb_tim, gwb_calcite, 'rs', linewidth = 2.0, label = 'Calcite (GWB)')
plt.plot(gwb_tim, gwb_dawsonite, 'gs', linewidth = 2.0, label = 'Dawsonite (GWB)')
plt.plot(gwb_tim, gwb_paragonite, 'bs', linewidth = 2.0, label = 'Paragonite (GWB)')
plt.plot(gwb_tim, gwb_phlogopite, 'ys', linewidth = 2.0, label = 'Phlogopite (GWB)')
plt.legend()
plt.xlabel("Time (days)")
plt.ylabel("Mineral volume change (cm$^{3}$)")
plt.title("Precipitating minerals during alkali flooding")
plt.savefig("../../../doc/content/media/geochemistry/flushing_2.png")

plt.figure(2)
plt.plot(tim, dolomite, 'k-', linewidth = 2.0, label = 'Dolomite (MOOSE)')
plt.plot(tim, kaolinite, 'r-', linewidth = 2.0, label = 'Kaolinite (MOOSE)')
plt.plot(tim, muscovite, 'g-', linewidth = 2.0, label = 'Muscovite (MOOSE)')
plt.plot(gwb_tim, gwb_dolomite, 'ks', linewidth = 2.0, label = 'Dolomite (GWB)')
plt.plot(gwb_tim, gwb_kaolinite, 'rs', linewidth = 2.0, label = 'Kaolinite (GWB)')
plt.plot(gwb_tim, gwb_muscovite, 'gs', linewidth = 2.0, label = 'Muscovite (GWB)')
plt.legend()
plt.xlabel("Time (days)")
plt.ylabel("Mineral volume change (cm$^{3}$)")
plt.title("Dissolving minerals during alkali flooding")
plt.savefig("../../../doc/content/media/geochemistry/flushing_3.png")

sys.exit(0)
