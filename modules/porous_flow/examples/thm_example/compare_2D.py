#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

# This script generates pictures that demonstrate the agreement between the analytic LaForce solutions and MOOSE

import os
import sys
import numpy as np
import matplotlib.pyplot as plt

def tara_read(fn, scaling = 1):
    try:
        f = open(os.path.join("gold", fn))
        data = f.readlines()
        data = [list(map(float, d.strip().split(","))) for d in data]
        data = ([d[0] for d in data], [d[1] * scaling for d in data])
        f.close()
    except:
        sys.stderr.write("Cannot read " + fn + ", or it contains erroneous data\n")
        sys.exit(1)
    return data

def moose(fn):
    try:
        f = open(os.path.join("gold", fn))
        data = f.readlines()[1:-1]
        data = [list(map(float, d.strip().split(","))) for d in data]
        log10_max_val = np.log10(len(data) - 1)
        num_pts_displayed = 70
        subsample = [np.power(10, log10_max_val * i / float(num_pts_displayed - 1)) for i in range(num_pts_displayed)] # 1 to max_val in logarithmic progression
        subsample = sorted(list(set([0] + [int(np.round(s)) for s in subsample])))  # 0 to len(data)-1 in log progression
        data = [data[i] for i in subsample]
        data = ([d[7] for d in data], [d[2] for d in data], [d[6] for d in data], [d[3] for d in data], [d[0] for d in data], [d[4] for d in data], [d[5] for d in data])
        f.close()
    except:
        sys.stderr.write("Cannot read " + fn + ", or it contains erroneous data\n")
        sys.exit(1)
    return data


result_file_names = ["pp_one_hour.csv", "pp_one_day.csv", "pp_one_month.csv", "pp_five_years.csv"]
tara_pp = [tara_read(fn, scaling = 1E-6) for fn in result_file_names]
result_file_names = ["tt_one_hour.csv", "tt_one_day.csv", "tt_one_month.csv", "tt_five_years.csv"]
tara_tt = [tara_read(fn, scaling = 1) for fn in result_file_names]
result_file_names = ["sg_one_hour.csv", "sg_one_day.csv", "sg_one_month.csv", "sg_five_years.csv"]
tara_sg = [tara_read(fn, scaling = 1) for fn in result_file_names]
result_file_names = ["u_one_hour.csv", "u_one_day.csv", "u_one_month.csv", "u_five_years.csv"]
tara_u = [tara_read(fn, scaling = 1) for fn in result_file_names]
result_file_names = ["seff_rr_one_hour.csv", "seff_rr_one_day.csv", "seff_rr_one_month.csv", "seff_rr_five_years.csv"]
tara_seff_rr = [tara_read(fn, scaling = 1) for fn in result_file_names]
result_file_names = ["seff_tt_one_hour.csv", "seff_tt_one_day.csv", "seff_tt_one_month.csv", "seff_tt_five_years.csv"]
tara_seff_tt = [tara_read(fn, scaling = 1) for fn in result_file_names]


moose_timesteps = ["0062", "0098", "0135", "0180"]
moose_timesteps = ["0078", "0198", "0412", "0704"]
moose_timesteps = ["0062", "0146", "0257", "0401"]
moosePTSUSS = [moose("2D_csv_ptsuss_" + ts + ".csv") for ts in moose_timesteps]

plt.figure()
plt.semilogx(moosePTSUSS[0][0], [(p - 18.3E6)/1E6 for p in moosePTSUSS[0][1]], 'b*', markersize=4, label = 'MOOSE (1 hour)')
plt.semilogx(tara_pp[0][0], tara_pp[0][1], 'b-', label = 'LaForce (1 hour)')
plt.semilogx(moosePTSUSS[1][0], [(p - 18.3E6)/1E6 for p in moosePTSUSS[1][1]], 'rs', markersize=4, label = 'MOOSE (1 day)')
plt.semilogx(tara_pp[1][0], tara_pp[1][1], 'r-', label = 'LaForce (1 day)')
plt.semilogx(moosePTSUSS[2][0], [(p - 18.3E6)/1E6 for p in moosePTSUSS[2][1]], 'g^', markersize=4, label = 'MOOSE (1 month)')
plt.semilogx(tara_pp[2][0], tara_pp[2][1], 'g-', label = 'LaForce (1 month)')
plt.semilogx(moosePTSUSS[3][0], [(p - 18.3E6)/1E6 for p in moosePTSUSS[3][1]], 'ko', markersize=4, label = 'MOOSE (5 years)')
plt.semilogx(tara_pp[3][0], tara_pp[3][1], 'k-', label = 'LaForce (5 years)')
plt.legend(loc = 'best', prop = {'size': 10})
plt.xlim([0.1, 5000])
plt.xlabel("r (m)")
plt.ylabel("Porepressure increase (MPa)")
plt.title("Porepressure")
plt.savefig("../../doc/content/media/porous_flow/2D_thm_compare_porepressure_fig.png")


plt.figure()
plt.semilogx(moosePTSUSS[0][0], moosePTSUSS[0][2], 'b*', markersize=4, label = 'MOOSE (1 hour)')
plt.semilogx(tara_tt[0][0], tara_tt[0][1], 'b-', label = 'LaForce (1 hour)')
plt.semilogx(moosePTSUSS[1][0], moosePTSUSS[1][2], 'rs', markersize=4, label = 'MOOSE (1 day)')
plt.semilogx(tara_tt[1][0], tara_tt[1][1], 'r-', label = 'LaForce (1 day)')
plt.semilogx(moosePTSUSS[2][0], moosePTSUSS[2][2], 'g^', markersize=4, label = 'MOOSE (1 month)')
plt.semilogx(tara_tt[2][0], tara_tt[2][1], 'g-', label = 'LaForce (1 month)')
plt.semilogx(moosePTSUSS[3][0], moosePTSUSS[3][2], 'ko', markersize=4, label = 'MOOSE (5 years)')
plt.semilogx(tara_tt[3][0], tara_tt[3][1], 'k-', label = 'LaForce (5 years)')
plt.legend(loc = 'best', prop = {'size': 10})
plt.xlim([0.1, 5000])
plt.xlabel("r (m)")
plt.ylabel("Temperature (K)")
plt.title("Temperature")
plt.savefig("../../doc/content/media/porous_flow/2D_thm_compare_temperature_fig.png")

plt.figure()
plt.semilogx(moosePTSUSS[0][0], [u * 1000 for u in moosePTSUSS[0][4]], 'b*', markersize=4, label = 'MOOSE (1 hour)')
plt.semilogx(tara_u[0][0], tara_u[0][1], 'b-', label = 'LaForce (1 hour)')
plt.semilogx(moosePTSUSS[1][0], [u * 1000 for u in moosePTSUSS[1][4]], 'rs', markersize=4, label = 'MOOSE (1 day)')
plt.semilogx(tara_u[1][0], tara_u[1][1], 'r-', label = 'LaForce (1 day)')
plt.semilogx(moosePTSUSS[2][0], [u * 1000 for u in moosePTSUSS[2][4]], 'g^', markersize=4, label = 'MOOSE (1 month)')
plt.semilogx(tara_u[2][0], tara_u[2][1], 'g-', label = 'LaForce (1 month)')
plt.semilogx(moosePTSUSS[3][0], [u * 1000 for u in moosePTSUSS[3][4]], 'ko', markersize=4, label = 'MOOSE (5 years)')
plt.semilogx(tara_u[3][0], tara_u[3][1], 'k-', label = 'LaForce (5 years)')
plt.legend(loc = 'best', prop = {'size': 10})
plt.xlim([0.1, 5000])
plt.xlabel("r (m)")
plt.ylabel("Displacement (mm)")
plt.title("Radial displacement")
plt.savefig("../../doc/content/media/porous_flow/2D_thm_compare_displacement_fig.png")

plt.figure()
plt.semilogx(moosePTSUSS[0][0], moosePTSUSS[0][3], 'b*', markersize=4, label = 'MOOSE (1 hour)')
plt.semilogx(tara_sg[0][0], tara_sg[0][1], 'b-', label = 'LaForce (1 hour)')
plt.semilogx(moosePTSUSS[1][0], moosePTSUSS[1][3], 'rs', markersize=4, label = 'MOOSE (1 day)')
plt.semilogx(tara_sg[1][0], tara_sg[1][1], 'r-', label = 'LaForce (1 day)')
plt.semilogx(moosePTSUSS[2][0], moosePTSUSS[2][3], 'g^', markersize=4, label = 'MOOSE (1 month)')
plt.semilogx(tara_sg[2][0], tara_sg[2][1], 'g-', label = 'LaForce (1 month)')
plt.semilogx(moosePTSUSS[3][0], moosePTSUSS[3][3], 'ko', markersize=4, label = 'MOOSE (5 years)')
plt.semilogx(tara_sg[3][0], tara_sg[3][1], 'k-', label = 'LaForce (5 years)')
plt.legend(loc = 'best', prop = {'size': 10})
plt.xlim([0.1, 5000])
plt.xlabel("r (m)")
plt.ylabel("Saturation")
plt.title("CO2 saturation")
plt.savefig("../../doc/content/media/porous_flow/2D_thm_compare_sg_fig.png")

plt.figure()
plt.semilogx(moosePTSUSS[0][0], [s/1E6 for s in moosePTSUSS[0][5]], 'b*', markersize=4, label = 'MOOSE (1 hour)')
plt.semilogx(tara_seff_rr[0][0], tara_seff_rr[0][1], 'b-', label = 'LaForce (1 hour)')
plt.semilogx(moosePTSUSS[1][0], [s/1E6 for s in moosePTSUSS[1][5]], 'rs', markersize=4, label = 'MOOSE (1 day)')
plt.semilogx(tara_seff_rr[1][0], tara_seff_rr[1][1], 'r-', label = 'LaForce (1 day)')
plt.semilogx(moosePTSUSS[2][0], [s/1E6 for s in moosePTSUSS[2][5]], 'g^', markersize=4, label = 'MOOSE (1 month)')
plt.semilogx(tara_seff_rr[2][0], tara_seff_rr[2][1], 'g-', label = 'LaForce (1 month)')
plt.semilogx(moosePTSUSS[3][0], [s/1E6 for s in moosePTSUSS[3][5]], 'ko', markersize=4, label = 'MOOSE (5 years)')
plt.semilogx(tara_seff_rr[3][0], tara_seff_rr[3][1], 'k-', label = 'LaForce (5 years)')
plt.legend(loc = 'best', prop = {'size': 10})
plt.xlim([0.1, 5000])
plt.xlabel("r (m)")
plt.ylabel("Stress (MPa)")
plt.title("Effective radial stress")
plt.savefig("../../doc/content/media/porous_flow/2D_thm_compare_seff_rr_fig.png")

plt.figure()
plt.semilogx(moosePTSUSS[0][0], [s/1E6 for s in moosePTSUSS[0][6]], 'b*', markersize=4, label = 'MOOSE (1 hour)')
plt.semilogx(tara_seff_tt[0][0], tara_seff_tt[0][1], 'b-', label = 'LaForce (1 hour)')
plt.semilogx(moosePTSUSS[1][0], [s/1E6 for s in moosePTSUSS[1][6]], 'rs', markersize=4, label = 'MOOSE (1 day)')
plt.semilogx(tara_seff_tt[1][0], tara_seff_tt[1][1], 'r-', label = 'LaForce (1 day)')
plt.semilogx(moosePTSUSS[2][0], [s/1E6 for s in moosePTSUSS[2][6]], 'g^', markersize=4, label = 'MOOSE (1 month)')
plt.semilogx(tara_seff_tt[2][0], tara_seff_tt[2][1], 'g-', label = 'LaForce (1 month)')
plt.semilogx(moosePTSUSS[3][0], [s/1E6 for s in moosePTSUSS[3][6]], 'ko', markersize=4, label = 'MOOSE (5 years)')
plt.semilogx(tara_seff_tt[3][0], tara_seff_tt[3][1], 'k-', label = 'LaForce (5 years)')
plt.legend(loc = 'best', prop = {'size': 10})
plt.xlim([0.1, 5000])
plt.xlabel("r (m)")
plt.ylabel("Stress (MPa)")
plt.title("Effective hoop stress")
plt.savefig("../../doc/content/media/porous_flow/2D_thm_compare_seff_tt_fig.png")

sys.exit(0)
