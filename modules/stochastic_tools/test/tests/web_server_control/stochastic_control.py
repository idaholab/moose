#!/usr/bin/env python3
# * This file is part of the MOOSE framework
# * https://mooseframework.inl.gov
# *
# * All rights reserved, see COPYRIGHT for full restrictions
# * https://github.com/idaholab/moose/blob/master/COPYRIGHT
# *
# * Licensed under LGPL 2.1, please see LICENSE for details
# * https://www.gnu.org/licenses/lgpl-2.1.html
"""
This should be called by the test harness with sub.i input file.

The purpose is to test StochasticControl for perturbing input parameters and
gathering results on-the-fly using the MOOSE WebServerControl.

It works by randomly generating a series of sampling matrices and compares the
result of running the STM with some optimization benchmark problems. There is no
"gold" file in this sense, it instead throws an assertion if the retrieved value
doesn't match a python version of these functions.
"""

import os
import sys
import shutil
import numpy as np
import argparse
import importlib.util
from scipy.optimize import rosen  # Exact match to Rosenbrock::rosen

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
    _moose_dir = os.environ.get("MOOSE_DIR", None)
    if not _moose_dir:
        _moose_dir = os.path.join(os.path.dirname(__file__), *([".."] * 5))
    _stm_python_path = os.path.abspath(
        os.path.join(_moose_dir, "modules", "stochastic_tools", "python")
    )
    tryImportStochasticControl(_stm_python_path)


def eggholder(x):
    """Exact match to Eggholder::eggholder"""
    return -(x[1] + 47.0) * np.sin(np.sqrt(abs(x[0] / 2.0 + (x[1] + 47.0)))) - x[
        0
    ] * np.sin(np.sqrt(abs(x[0] - (x[1] + 47.0))))


def compare(x, y):
    """
    Compares the result from using StochasticControl to the python version of
    the rosen and eggholder functions.
    """
    for j in range(y.shape[1]):
        if j == 0:
            xj = x if y.shape[1] == 1 else x[:, :-2]
            func = rosen
        else:
            xj = x[:, -2:]
            func = eggholder

        for i in range(y.shape[0]):
            xij = xj[i, :]
            if not np.isclose(y[i, j], func(xij)):
                raise ValueError(f"{func.__name__}({xij}):={func(xij)} != {y[i, j]}")
            assert np.isclose(y[i, j], func(xij))


def test_options():
    """Command-line options to facilitate various testing schemes."""
    parser = argparse.ArgumentParser()
    parser.add_argument("mode", default=0, type=int, help="Execution mode.")
    parser.add_argument(
        "--multi-output", action="store_true", help="Whether to do multiple QoIs."
    )
    parser.set_defaults(multi_output=False)
    return parser.parse_args()


if __name__ == "__main__":

    num_steps = np.random.randint(3, 10)
    num_cols = np.random.randint(3, 10)
    options = {}  # Options to insert into StochasticRunOptions

    # Arguments gathered from RUNAPP_COMMAND
    cmd = os.environ.get("RUNAPP_COMMAND")
    if cmd is None:
        sys.exit("Missing expected command variable RUNAPP_COMMAND")
    cmd = cmd.split()
    # Gather MPI options if running with MPI
    if "-n" in cmd:
        mpi_index = cmd.index("-n") - 1
        options["mpi_command"] = cmd[mpi_index]
        options["num_procs"] = int(cmd[mpi_index + 2])
    # Get executable and input file based on position of '-i' argument
    exec_index = cmd.index("-i") - 1
    executable = cmd[exec_index]
    input_file = cmd[exec_index + 2]
    # Everything after the input is cli_args
    options["cli_args"] = cmd[(exec_index + 3) :]

    # Import StochasticControl based on executable location (i.e. from conda)
    if StochasticControl is None:
        _exec_dir = os.path.dirname(os.path.abspath(shutil.which(executable)))
        _share_dir = os.path.abspath(os.path.join(_exec_dir, "..", "share"))
        _moose_stm_python = os.path.join(_share_dir, "stochastic_tools", "python")
        if not tryImportStochasticControl(_moose_stm_python):
            raise ModuleNotFoundError(
                "Could not find MOOSE stochastic tools module python utilities."
            )

    # Arguments from cli
    cli_args = test_options()
    parameters = [
        "Postprocessors/rosenbrock/x["
        + ",".join([str(i) for i in range(num_cols)])
        + "]"
    ]
    qois = ["rosenbrock/value"]
    if cli_args.multi_output:
        parameters.append(f"Postprocessors/eggholder/x[{num_cols},{num_cols + 1}]")
        qois.append("eggholder/value")
        num_cols += 2
    options["multiapp_mode"] = StochasticRunOptions.MultiAppMode(cli_args.mode)

    # Get sampling matrices to test with
    matrices = [None] * num_steps
    for t in range(num_steps):
        # Test at least one matrix with only one entry
        num_rows = 1 if t == 0 else np.random.randint(1, 10)
        matrices[t] = 2.0 * (t + 1) * (np.random.random((num_rows, num_cols)) - 0.5)

    with StochasticControl(
        executable=executable,
        physics_input=input_file,
        parameters=parameters,
        quantities_of_interest=qois,
        options=StochasticRunOptions(**options),
    ) as runner:
        for matrix in matrices:
            result = runner(matrix)
            # Need to reshape since result could be a float or 1-D array
            result = np.array(result).reshape(
                (matrix.shape[0], 2 if cli_args.multi_output else 1)
            )
            compare(matrix, result)
