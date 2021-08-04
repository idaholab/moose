#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

# Solution to Mandel's problem as presented in
# AHD Cheng and E Detournay "A direct boundary element method for plane strain poroelasticity" International Journal of Numerical and Analytical Methods in Geomechanics 12 (1988) 551-572

import os
import sys
import numpy as np
import matplotlib.pyplot as plt

def expected(x, t):
    # expected solution at time t and position x

    # input parameters
    soil_width = 1.0
    soil_height = 0.1
    soil_lame_lambda = 0.5
    soil_lame_mu = 0.75
    fluid_bulk_modulus = 8.0
    initial_porosity = 0.1
    biot_coeff = 0.6
    fluid_mobility = 1.5
    normal_stress = 1.0

    # derived parameters
    soil_shear_modulus = soil_lame_mu
    soil_drained_bulk = soil_lame_lambda + 2.0 * soil_lame_mu / 3.0
    fluid_bulk_compliance = 1.0 / fluid_bulk_modulus
    biot_modulus = 1.0 / (initial_porosity / fluid_bulk_modulus + (biot_coeff - initial_porosity) * (1.0 - biot_coeff) / soil_drained_bulk)
    undrained_bulk_modulus = soil_drained_bulk + biot_coeff**2 * biot_modulus
    skempton = biot_coeff * biot_modulus / undrained_bulk_modulus
    drained_poisson = (3.0 * soil_drained_bulk - 2.0 * soil_shear_modulus) / (6.0 * soil_drained_bulk + 2.0 * soil_shear_modulus)
    undrained_poisson = (3.0 * undrained_bulk_modulus - 2.0 * soil_shear_modulus) / (6.0 * undrained_bulk_modulus + 2.0 * soil_shear_modulus)
    consolidation_coeff = 2.0 * fluid_mobility * skempton**2 * soil_shear_modulus * (1.0 - drained_poisson) * (1 + undrained_poisson)**2 / 9.0 / (1.0 - undrained_poisson) / (undrained_poisson - drained_poisson)

    roots = [1.419988120304100E+00, 4.666177581823210E+00, 7.826417353528760E+00, 1.097591703059930E+01, 1.412188800507350E+01, 1.726626279765500E+01, 2.040978005325610E+01, 2.355278342938330E+01, 2.669545454962390E+01, 2.983789845132980E+01, 3.298018011077390E+01, 3.612234188229790E+01]
    expr1 = [np.sin(v) / (v - np.sin(v) * np.cos(v)) for v in roots]
    expr2 = [np.sin(v) * np.cos(v) / (v - np.sin(v) * np.cos(v)) for v in roots]


    d_terms = [expr2[i] * np.exp(- roots[i]**2 * consolidation_coeff * t / soil_width**2) for i in range(len(roots))]
    # following is for t = 0
    # vert_disp = - normal_stress * soil_height * (1.0 - undrained_poisson) / 2.0 / soil_shear_modulus / soil_width
    # hor_disp = normal_stress * undrained_poisson / 2.0 / soil_shear_modulus
    vert_disp = - normal_stress * (1.0 - drained_poisson) * soil_height / 2.0 / soil_shear_modulus / soil_width + normal_stress * (1.0 - undrained_poisson) * soil_height / soil_shear_modulus / soil_width * sum(d_terms)
    hor_disp = normal_stress * drained_poisson / 2.0 / soil_shear_modulus + normal_stress * (1.0 - undrained_poisson) / soil_shear_modulus * sum(d_terms)

    p_terms = [(expr1[i] * np.cos(roots[i] * x / soil_width) - expr2[i]) * np.exp(- (roots[i] / soil_width)**2 * consolidation_coeff * t) for i in range(len(roots))]
    porepressure = 2.0 * normal_stress * skempton * (1.0 + undrained_poisson) / 3.0 / soil_width * sum(p_terms)

    return (vert_disp, hor_disp, porepressure)

def get_moose_results(fi):
    f = open(fi)
    data = [list(map(float, line.strip().split(","))) for line in f.readlines()[1:] if line.strip()]
    f.close()
    t = [d[0] for d in data]
    p0 = [d[1] for d in data]
    p3 = [d[4] for d in data]
    p8 = [d[9] for d in data]
    force = [d[12] for d in data]
    xdisp = [d[13] for d in data]
    ydisp = [d[14] for d in data]
    return (t, p0, p3, p8, force, xdisp, ydisp)

tpoints = np.arange(1E-5, 0.71, 1E-3)

moose_hm = get_moose_results("../../../../../../test/tests/poro_elasticity/gold/mandel.csv")
moose_constM = get_moose_results("../../../../../../test/tests/poro_elasticity/gold/mandel_constM.csv")
moose_fs = get_moose_results("../../../../../../test/tests/poro_elasticity/gold/mandel_fully_saturated.csv")
moose_fsv = get_moose_results("../../../../../../test/tests/poro_elasticity/gold/mandel_fully_saturated_volume.csv")

plt.figure()
plt.plot(tpoints, expected(0.0, tpoints)[2], 'k-', linewidth = 2.0, label = 'expected, x = 0')
plt.plot(tpoints, expected(0.3, tpoints)[2], 'r-', linewidth = 2.0, label = 'expected, x = 0.3')
plt.plot(tpoints, expected(0.8, tpoints)[2], 'b-', linewidth = 2.0, label = 'expected, x = 0.8')
plt.plot(moose_hm[0], moose_hm[1], 'ks', markersize = 6.0, label = 'MOOSE HM, x = 0')
plt.plot(moose_hm[0], moose_hm[2], 'rs', markersize = 6.0, label = 'MOOSE HM, x = 0.3')
plt.plot(moose_hm[0], moose_hm[3], 'bs', markersize = 6.0, label = 'MOOSE HM, x = 0.8')
plt.legend(loc = 'upper right')
plt.xlabel("time")
plt.ylabel("Porepressure")
plt.title("Mandel's problem: Porepressure at points in the sample")
#plt.axis([0, 100, 199, 301])
plt.savefig("mandel_HM.png")

plt.figure()
plt.plot(tpoints, expected(0.0, tpoints)[2], 'k-', linewidth = 2.0, label = 'expected, x = 0')
plt.plot(tpoints, expected(0.3, tpoints)[2], 'r-', linewidth = 2.0, label = 'expected, x = 0.3')
plt.plot(tpoints, expected(0.8, tpoints)[2], 'b-', linewidth = 2.0, label = 'expected, x = 0.8')
plt.plot(moose_constM[0], moose_constM[1], 'ks', markersize = 6.0, label = 'MOOSE ConstM, x = 0')
plt.plot(moose_constM[0], moose_constM[2], 'rs', markersize = 6.0, label = 'MOOSE ConstM, x = 0.3')
plt.plot(moose_constM[0], moose_constM[3], 'bs', markersize = 6.0, label = 'MOOSE ConstM, x = 0.8')
plt.legend(loc = 'upper right')
plt.xlabel("time")
plt.ylabel("Porepressure")
plt.title("Mandel's problem: Porepressure at points in the sample")
#plt.axis([0, 100, 199, 301])
plt.savefig("mandel_ConstM.png")

plt.figure()
plt.plot(tpoints, expected(0.0, tpoints)[2], 'k-', linewidth = 2.0, label = 'expected, x = 0')
plt.plot(tpoints, expected(0.3, tpoints)[2], 'r-', linewidth = 2.0, label = 'expected, x = 0.3')
plt.plot(tpoints, expected(0.8, tpoints)[2], 'b-', linewidth = 2.0, label = 'expected, x = 0.8')
plt.plot(moose_fs[0], moose_fs[1], 'ks', markersize = 6.0, label = 'MOOSE FullySat, x = 0')
plt.plot(moose_fs[0], moose_fs[2], 'rs', markersize = 6.0, label = 'MOOSE FullySat, x = 0.3')
plt.plot(moose_fs[0], moose_fs[3], 'bs', markersize = 6.0, label = 'MOOSE FullySat, x = 0.8')
plt.legend(loc = 'upper right')
plt.xlabel("time")
plt.ylabel("Porepressure")
plt.title("Mandel's problem: Porepressure at points in the sample")
#plt.axis([0, 100, 199, 301])
plt.savefig("mandel_FS.png")


plt.figure()
plt.plot(tpoints, expected(0.0, tpoints)[2], 'k-', linewidth = 2.0, label = 'expected, x = 0')
plt.plot(tpoints, expected(0.3, tpoints)[2], 'r-', linewidth = 2.0, label = 'expected, x = 0.3')
plt.plot(tpoints, expected(0.8, tpoints)[2], 'b-', linewidth = 2.0, label = 'expected, x = 0.8')
plt.plot(moose_fsv[0], moose_fsv[1], 'ks', markersize = 6.0, label = 'MOOSE FullySatVol, x = 0')
plt.plot(moose_fsv[0], moose_fsv[2], 'rs', markersize = 6.0, label = 'MOOSE FullySatVol, x = 0.3')
plt.plot(moose_fsv[0], moose_fsv[3], 'bs', markersize = 6.0, label = 'MOOSE FullySatVol, x = 0.8')
plt.legend(loc = 'upper right')
plt.xlabel("time")
plt.ylabel("Porepressure")
plt.title("Mandel's problem: Porepressure at points in the sample")
#plt.axis([0, 100, 199, 301])
plt.savefig("mandel_FSV.png")

plt.figure()
plt.plot(tpoints, expected(0.0, tpoints)[1], 'k-', linewidth = 2.0, label = 'expected')
plt.plot(moose_hm[0], moose_hm[5], 'ks', markersize = 6.0, label = 'MOOSE HM')
plt.plot(moose_constM[0], moose_constM[5], 'gx', markersize = 9.0, label = 'MOOSE ConstM')
plt.plot(moose_fs[0], moose_fs[5], 'b^', markersize = 6.0, label = 'MOOSE FullySat')
plt.plot(moose_fsv[0], moose_fsv[5], 'r*', markersize = 4.0, label = 'MOOSE FullySatVol')
plt.legend(loc = 'upper right')
plt.xlabel("time")
plt.ylabel("Displacement")
plt.title("Mandel's problem: Platten horizontal displacement")
plt.savefig("mandel_hor_disp.png")


plt.figure()
plt.plot(tpoints, expected(0.0, tpoints)[0], 'k-', linewidth = 2.0, label = 'expected')
plt.plot(moose_hm[0], moose_hm[6], 'ks', markersize = 6.0, label = 'MOOSE HM')
plt.plot(moose_constM[0], moose_constM[6], 'gx', markersize = 9.0, label = 'MOOSE ConstM')
plt.plot(moose_fs[0], moose_fs[6], 'b^', markersize = 6.0, label = 'MOOSE FullySat')
plt.plot(moose_fsv[0], moose_fsv[6], 'r*', markersize = 4.0, label = 'MOOSE FullySatVol')
plt.legend(loc = 'upper right')
plt.xlabel("time")
plt.ylabel("Displacement")
plt.title("Mandel's problem: Platten vertical displacement")
plt.savefig("mandel_ver_disp.png")

plt.figure()
plt.plot(moose_hm[0], moose_hm[4], 'k-', markersize = 6.0, label = 'MOOSE HM')
plt.plot(moose_constM[0], moose_constM[4], 'g-', markersize = 9.0, label = 'MOOSE ConstM')
plt.plot(moose_fs[0], moose_fs[4], 'b-', markersize = 6.0, label = 'MOOSE FullySat')
plt.plot(moose_fsv[0], moose_fsv[4], 'r-', markersize = 4.0, label = 'MOOSE FullySatVol')
plt.legend(loc = 'upper right')
plt.xlabel("time")
plt.ylabel("Force")
plt.title("Mandel's problem: Total downwards force")
plt.savefig("mandel_force.png")


sys.exit(0)
