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

def expected(scheme, angle_degrees):
    angle = angle_degrees * np.pi / 180.0

    cohesion = 10
    friction_degrees = 20
    tip_smoother = 4
    mean = -10
    friction = friction_degrees * np.pi / 180.0

    if (scheme == "native"):
        coh = cohesion
        fric = friction
    elif (scheme == "outer_tip"):
        coh = 2 * np.sqrt(3) * cohesion * np.cos(friction) / (3.0 - np.sin(friction))
        fric = np.arctan(2 * np.sin(friction) / np.sqrt(3) / (3.0 - np.sin(friction)))
    elif (scheme == "inner_tip"):
        coh = 2 * np.sqrt(3) * cohesion * np.cos(friction) / (3.0 + np.sin(friction))
        fric = np.arctan(2 * np.sin(friction) / np.sqrt(3) / (3.0 + np.sin(friction)))
    elif (scheme == "lode_zero"):
        coh = cohesion * np.cos(friction)
        fric = np.arctan(np.sin(friction) / 3.0)
    elif (scheme == "inner_edge"):
        coh = 3 * cohesion * np.cos(friction) / np.sqrt(9.0 + 3.0 * np.power(np.sin(friction), 2))
        fric = np.arctan(np.sin(friction) / np.sqrt(9.0 + 3.0 * np.power(np.sin(friction), 2)))

    bar = np.sqrt(np.power(coh - mean * 3.0 * np.tan(fric), 2) - np.power(tip_smoother, 2))
    x = bar * np.cos(angle)
    y = bar * np.sin(angle)
    return (x, y)

def sigma_mean(stress):
    return (stress[0] + stress[3] + stress[5])/3.0

def sigma_bar(stress):
    mean = sigma_mean(stress)
    return np.sqrt(0.5 * (np.power(stress[0] - mean, 2) + 2*stress[1]*stress[1] + 2*stress[2]*stress[2] + np.power(stress[3] - mean, 2) + 2*stress[4]*stress[4] + np.power(stress[5] - mean, 2)))

def third_inv(stress):
    mean = sigma_mean(stress)
    return (stress[0] - mean)*(stress[3] - mean)*(stress[5] - mean)

def lode_angle(stress):
    bar = sigma_bar(stress)
    third = third_inv(stress)
    return np.arcsin(-1.5 * np.sqrt(3.0) * third / np.power(bar, 3)) / 3.0

def moose_result(fn):
    f = open(fn)
    x = []
    y = []
    for line in f:
        if not line.strip():
            continue
        line = line.strip()
        if line.startswith("time") or line.startswith("0"):
            continue
        line = map(float, line.split(","))
        if line[1] < -1E-10:
            continue # this is an elastic deformation
        bar = sigma_bar(line[4:])
        lode = lode_angle(line[4:])
        x.append(bar * np.cos(lode))
        y.append(bar * np.sin(lode))
    f.close()
    return (x, y)


angles = np.arange(-30, 31, 1)

plt.figure()

plt.plot(expected("native", angles)[0], expected("native", angles)[1], 'k-', label = 'expected (native)')
mr = moose_result("gold/small_deform2_native.csv")
plt.plot(mr[0], mr[1], 'k^', label = 'MOOSE (native)')

plt.plot(expected("outer_tip", angles)[0], expected("outer_tip", angles)[1], 'g-', label = 'expected (outer_tip)')
mr = moose_result("gold/small_deform2_outer_tip.csv")
plt.plot(mr[0], mr[1], 'g^', label = 'MOOSE (outer_tip)')

plt.plot(expected("inner_tip", angles)[0], expected("inner_tip", angles)[1], 'b-', label = 'expected (inner_tip)')
mr = moose_result("gold/small_deform2_inner_tip.csv")
plt.plot(mr[0], mr[1], 'b^', label = 'MOOSE (inner_tip)')

plt.plot(expected("lode_zero", angles)[0], expected("lode_zero", angles)[1], 'c-', label = 'expected (lode_zero)')
mr = moose_result("gold/small_deform2_lode_zero.csv")
plt.plot(mr[0], mr[1], 'c^', label = 'MOOSE (lode_zero)')

plt.plot(expected("inner_edge", angles)[0], expected("inner_edge", angles)[1], 'r-', label = 'expected (inner_edge)')
mr = moose_result("gold/small_deform2_inner_edge.csv")
plt.plot(mr[0], mr[1], 'r^', label = 'MOOSE (inner_edge)')

legend = plt.legend(bbox_to_anchor=(1.16, 0.95))
for label in legend.get_texts():
    label.set_fontsize('small')
plt.xlabel("Stress")
plt.ylabel("Stress")
plt.title("Drucker-Prager yield function on octahedral plane")
plt.axis([5, 25, -12, 12])
plt.savefig("small_deform2.png")

sys.exit(0)
