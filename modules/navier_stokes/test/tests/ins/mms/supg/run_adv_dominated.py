# To compare convergence of INS variables for SUPG stabilized and unstabilized cases
# run this script and then adv_dominated_plot_convergence.py

from subprocess import call
import numpy as np
'''
Function for running MMS simulation cases
'''


def mms_cases(h_list, string_list, mu):
    for h in h_list:
        n = int(1/h)
        dt = h / 1.8
        for string in string_list:
            args = ["mpirun", "-np", str(n / 4 + 8), "navier_stokes-opt", "-i", "supg_adv_dominated_mms.i",
                    "Mesh/nx=%s" % n,
                    "Mesh/ny=%s" % n,
                    "Outputs/file_base=%s_mu%s_%sx%s" % (string, mu, n, n),
                    "Executioner/TimeStepper/dt=%s" % dt,
                    "Executioner/num_steps=1000000",
                    "Executioner/trans_ss_check=true",
                    "Executioner/ss_check_tol=1e-10"]

            if string == "unstabilized":
                args.append("GlobalParams/supg=false")
            call(args)


strings = ["stabilized", "unstabilized"]
h_list = np.array([.25,
                   .125,
                   .0625,
                   .03125,
                   .015625])
mms_cases(h_list, strings, "1.5e-2")
