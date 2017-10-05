# Example script for running mms cases

from subprocess import call
import numpy as np
import sys

def mms_cases(h_list, string_list, mu):
    for h in h_list:
        n = int(1/h)
        dt = h / 1.8
        for string in string_list:
            args = ["mpirun", "-np", str(24), "navier_stokes-opt", "-i", "supg_pspg_adv_dominated_mms.i",
                    "Mesh/nx=%s" % n,
                    "Mesh/ny=%s" % n,
                    "Outputs/file_base=%s_mu%s_%sx%s" % (string, mu, n, n),
                    "Executioner/TimeStepper/dt=%s" % dt,
                    "Executioner/num_steps=1000000",
                    "Executioner/trans_ss_check=true",
                    "Executioner/ss_check_tol=1e-10",
                    "mu=%s" % mu]

            if string.split('_')[0] == "unstabilized":
                args.append("GlobalParams/supg=false")
            call(args)


strings = ['_'.join(filter(None, ('stabilized', sys.argv[1])))]  # , '_'.join(filter(None, ('unstabilized', sys.argv[1])))]
h_list = np.array([.25,
                   .125,
                   .0625,
                   .03125])
                   # .015625,
                   # .0078125,
                   # .00390625])
# mus = ["1.5e1", "1.5e0", "5e-1", "1.5e-1", "5e-2", "1.5e-2", "5e-3", "1.5e-3", "1.5e-4"]
mus = ["1.5e1"]
for mu in mus:
    mms_cases(h_list, strings, mu)
