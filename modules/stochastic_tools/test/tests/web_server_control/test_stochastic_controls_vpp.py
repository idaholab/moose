#!/usr/bin/env python3
# * This file is part of the MOOSE framework
# * https://mooseframework.inl.gov
# *
# * All rights reserved, see COPYRIGHT for full restrictions
# * https://github.com/idaholab/moose/blob/master/COPYRIGHT
# *
# * Licensed under LGPL 2.1, please see LICENSE for details
# * https://www.gnu.org/licenses/lgpl-2.1.html
import numpy as np
import sys
import os
import importlib
import importlib.util
import argparse
import shutil

StochasticControl = None
StochasticRunOptions = None
StochasticRunner = None

def tryImportStochasticControl(path = None):
    global StochasticControl
    global StochasticRunOptions
    global StochasticRunner

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
        from moose_stochastic_tools.StochasticControl import StochasticRunner
        return True

if not tryImportStochasticControl():
    _moose_dir = os.environ.get("MOOSE_DIR", None)
    if not _moose_dir:
        _moose_dir = os.path.join(os.path.dirname(__file__), *(["..", * 5]))
    _moose_python_dir = os.path.abspath(os.path.join(_moose_dir,'python'))
    sys.path.append(_moose_python_dir)
    _stm_python_path = os.path.abspath(
        os.path.join(_moose_dir, "modules", "stochastic_tools", "python")
    )
    tryImportStochasticControl(_stm_python_path)

def test_options():
    """Command-line options to facilitate various testing schemes."""
    parser = argparse.ArgumentParser()
    parser.add_argument("mode", default=0, type=int, help="Execution mode.")
    return parser.parse_args()


if __name__ == "__main__":
    options = {
            'input_name':'stochastic_run.i',
            'ignore_solve_not_converge':False
        }
    cmd = os.environ.get("RUNAPP_COMMAND")
    if cmd is None:
        sys.exit("Missing expected command variable RUNAPP_COMMAND")
    cmd = cmd.split()

    if "-n" in cmd:
        mpi_index = cmd.index("-n")-1
        options["mpi_command"] = cmd[mpi_index]
        options["num_procs"] = int(cmd[mpi_index+2])
    exec_index = cmd.index("-i")-1
    executable = cmd[exec_index]
    input_file = cmd[exec_index+2]
    options["cli_args"] = cmd[(exec_index + 3):]

    if StochasticControl is None:
        _exec_dir = os.path.dirname(os.path.abspatch(shutil.which(executable)))
        _share_dir = os.path.abspath(os.path.join(_exec_dir, "..", "share"))
        _moose_stm_python = os.path.join(_share_dir, "stochastic_tools","python")
        _moose_python = os.path.join(_moose_stm_python,'..','..','..','python')
        if not (tryImportStochasticControl(_moose_python) and tryImportStochasticControl(_moose_stm_python)):
            raise ModuleNotFoundError("Could not find MOOSE stochastic tools module python utilities.")

    if StochasticRunOptions.MultiAppMode.BATCH_RESET is not None:
        options['multiapp_mode'] = StochasticRunOptions.MultiAppMode.BATCH_RESET,
    cli_args = test_options()
    input_file_name = 'main_runner.i'
    input_file = 'vpp_test_runner.i'
    command_possibilities = []
    with StochasticControl(executable=executable,
                         physics_input=input_file,
                         options=StochasticRunOptions(**options),
                         parameters=['capsule1:Postprocessors/frequency_factor/value',
                            'capsule1:Postprocessors/activation_energy/value',
                            'capsule1:BCs/heat_DRV_outer/value',
                            'capsule1:mesh_specified'],
                         quantities_of_interest=['capsule_01/x',
                                                'capsule_01/temp',
                                        ]) as runner:
        if cmd[exec_index+3]==0:
            runner.configCache(tol=[1,1e-10,1e-14,1e-14])
        elif cmd[exec_index+3]==1:
            runner([335478,27700,500,1])
            runner([[335478,27700,500,1],
                   [335078,23700,500,1]])#Test caching
        elif cmd[exec_index+3]==2:
            runner([ [385478,29700,385,20],
                     [335478,27700,500,1],
                     [485478,21700,385,1],
                     [385478,29700,385,20],
                     ])
