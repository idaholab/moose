#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from subprocess import call
import numpy as np
import sys


def mms_cases(h_list, string_list):
    for h in h_list:
        n = int(1/h)
        dt = h / 1.4
        for string in string_list:
            # args = ["mpirun", "-np", str(1), "navier_stokes-opt", "-i", "2d_advection_error_testing.i",
            args = ["lldb", "--", "navier_stokes-dbg", "-i", "2d_advection_error_testing.i",
                    "Mesh/nx=%s" % n,
                    "Mesh/ny=%s" % n,
                    # "Outputs/file_base=%s_%sx%s" % (string, n, n),
                    "Outputs/file_base=debug",
                    "Executioner/TimeStepper/dt=%s" % dt,
                    "Executioner/num_steps=1000000",
                    "Executioner/trans_ss_check=true",
                    "Executioner/ss_check_tol=1e-10"]
            call(args)


arg_string = sys.argv[1] if 1 < len(sys.argv) else None
strings = ['_'.join(filter(None, ('stabilized', arg_string)))]  # , '_'.join(filter(None, ('unstabilized', sys.argv[1])))]
h_list = np.array([.5,
                   .25,
                   .125,
                   .0625,
                   .03125])
                   # .015625,
                   # .0078125,
                   # .00390625])
mms_cases(h_list, strings)
