#!/usr/bin/env python3
# * This file is part of the MOOSE framework
# * https://mooseframework.inl.gov
# *
# * All rights reserved, see COPYRIGHT for full restrictions
# * https://github.com/idaholab/moose/blob/master/COPYRIGHT
# *
# * Licensed under LGPL 2.1, please see LICENSE for details
# * https://www.gnu.org/licenses/lgpl-2.1.html
import os
import numpy as np
from optimize_annulus import optimize_annulus, cli_args

if __name__ == "__main__":

    args = cli_args()

    cmd = os.environ.get("RUNAPP_COMMAND")
    if not cmd is None:
        cmd = cmd.split()
        # Gather MPI options if running with MPI
        if "-n" in cmd:
            mpi_index = cmd.index("-n") - 1
            args.num_procs = int(cmd[mpi_index + 2])
        # Get executable and input file based on position of '-i' argument
        exec_index = cmd.index("-i") - 1
        args.executable = cmd[exec_index]
        args.input_file = cmd[exec_index + 2]
        # Everything after the input is cli_args
        args.cli_args = cmd[(exec_index + 3) :]

    result = optimize_annulus(**vars(args))

    expected_x = np.array([1.198, 6.877])
    assert np.allclose(result.x, expected_x, rtol=0.01, atol=0)
