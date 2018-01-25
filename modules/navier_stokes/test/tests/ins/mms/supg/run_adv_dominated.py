#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

# Example script for running mms cases

from subprocess import call
import numpy as np
import sys

def mms_cases(h_list, methods, mu):
    for h in h_list:
        n = int(1/h)
        dt = h / 1.8
        for method in methods:
            args = ["mpirun", "-np", str(24), "navier_stokes-opt", "-i", "supg_pspg_adv_dominated_mms.i",
                    "Mesh/nx=%s" % n,
                    "Mesh/ny=%s" % n,
                    "Outputs/file_base=%s_mu%s_%sx%s" % (method, mu, n, n),
                    "Executioner/TimeStepper/dt=%s" % dt,
                    "Executioner/num_steps=1000000",
                    "Executioner/trans_ss_check=true",
                    "Executioner/ss_check_tol=1e-10",
                    "mu=%s" % mu]

            if method.split('_')[0] == "unstabilized":
                args.append("GlobalParams/supg=false")
            call(args)

methods = sys.argv[1:] if 1 < len(sys.argv) else []
methods = map(lambda x: '_'.join(filter(None, ('stabilized', x))), methods)
h_list = np.array([.25,
                   .125,
                   .0625,
                   .03125])
                   # .015625])
                   # .0078125,
                   # .00390625])
mus = ["1.5e1", "1.5e-4"]
for mu in mus:
    mms_cases(h_list, methods, mu)
