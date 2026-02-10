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
import sys
import shutil
import importlib.util
import numpy as np

StochasticControl = None
StochasticRunOptions = None


def tryImportStochasticControl(path=None):
    global StochasticControl
    global StochasticRunOptions

    if StochasticControl is not None:
        return True

    append_path = not (path is None or path in sys.path)
    if append_path:
        if not os.path.isdir(path):
            return False
        sys.path.append(path)

    if importlib.util.find_spec("moose_stochastic_tools") is None:
        if append_path:
            sys.path.pop(path)
        return False
    else:
        from moose_stochastic_tools import StochasticControl, StochasticRunOptions

        return True


if not tryImportStochasticControl():
    _moose_dir = os.environ.get(
        "MOOSE_DIR", os.path.join(os.path.dirname(__file__), *([".."] * 5))
    )
    _stm_python_path = os.path.abspath(
        os.path.join(_moose_dir, "modules", "stochastic_tools", "python")
    )
    tryImportStochasticControl(_stm_python_path)

if __name__ == "__main__":

    cmd = os.environ.get("RUNAPP_COMMAND")
    cmd_args = {}
    if not cmd is None:
        cmd = cmd.split()
        # Gather MPI options if running with MPI
        if "-n" in cmd:
            mpi_index = cmd.index("-n") - 1
            cmd_args["num_procs"] = int(cmd[mpi_index + 2])
        # Get executable and input file based on position of '-i' argument
        exec_index = cmd.index("-i") - 1
        cmd_args["executable"] = cmd[exec_index]
        cmd_args["input_file"] = cmd[exec_index + 2]
        # Everything after the input is cli_args
        cmd_args["cli_args"] = cmd[(exec_index + 3) :]

        # Import StochasticControl based on executable location (i.e. from conda)
        if StochasticControl is None:
            _exec_dir = os.path.dirname(
                os.path.abspath(shutil.which(cmd_args["executable"]))
            )
            _share_dir = os.path.abspath(os.path.join(_exec_dir, "..", "share"))
            _moose_stm_python = os.path.join(_share_dir, "stochastic_tools", "python")
            if not tryImportStochasticControl(_moose_stm_python):
                raise ModuleNotFoundError(
                    "Could not find MOOSE stochastic tools module python utilities."
                )

    from optimize_annulus import optimize_annulus, cli_args

    args = cli_args()
    for a, val in cmd_args.items():
        setattr(args, a, val)

    result = optimize_annulus(**vars(args))

    expected_x = np.array([1.198, 6.877])
    assert np.allclose(result.x, expected_x, rtol=0.01, atol=0)
