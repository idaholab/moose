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
gathering results on-the-fly using the MOOSE WebServerControl for VPP style outputs.

It works by checking predefined matrices and running the STM with some
 benchmark problems. There is no
"gold" file in this sense, it instead checks for syntax correctness.
"""

import os
import sys
import shutil
import numpy as np
import argparse
import importlib.util
import importlib

StochasticControl = None
StochasticRunOptions = None

list_gold = np.empty([4, 2], dtype=object)
list_gold[0, 0] = [[1.5], [0.75, 2.25], [0.5, 1.5, 2.5]]
list_gold[0, 1] = [
    [385.18664530934603],
    [385.35005257628313, 385.16340688051844],
    [385.25617000121963, 385.2147242350503, 385.0900900571101],
]
list_gold[1, 0] = [[0.75, 2.25], [0.75, 2.25], [0.5, 1.5, 2.5]]
list_gold[1, 1] = [
    [500.0295468236967, 500.013788589594],
    [500.0295468236967, 500.013788589594],
    [500.0215952091379, 500.0180934167227, 500.00758762565937],
]
list_gold[2, 0] = [[1.5], [0.75, 2.25], [0.5, 1.5, 2.5]]
list_gold[2, 1] = [
    [385.01222814747734],
    [385.0229278236915, 385.01069967258616],
    [385.01675726921934, 385.014039914151, 385.00588772520666],
]
list_gold[3, 0] = [[1.5], [0.75, 2.25], [0.5, 1.5, 2.5]]
list_gold[3, 1] = [
    [385.18664530934603],
    [385.35005257628313, 385.16340688051844],
    [385.25617000121963, 385.2147242350503, 385.0900900571101],
]
nestedlist_gold = np.array(
    [
        [
            list([[1.5], [0.75, 2.25], [0.5, 1.5, 2.5]]),
            list(
                [
                    [385.18664530934603],
                    [385.35005257628313, 385.16340688051844],
                    [385.25617000121963, 385.2147242350503, 385.0900900571101],
                ]
            ),
            list([[1, 2], [3, 4, 5], [6, 7, 8, 9, 0], [10, 11, 12, 13, 14, 15]]),
        ],
        [
            list([[0.75, 2.25], [0.75, 2.25], [0.5, 1.5, 2.5]]),
            list(
                [
                    [500.0295468236967, 500.013788589594],
                    [500.0295468236967, 500.013788589594],
                    [500.0215952091379, 500.0180934167227, 500.00758762565937],
                ]
            ),
            list([[1, 2], [3, 4, 5], [6, 7, 8, 9, 0], [10, 11, 12, 13, 14, 15]]),
        ],
        [
            list([[1.5], [0.75, 2.25], [0.5, 1.5, 2.5]]),
            list(
                [
                    [385.01222814747734],
                    [385.0229278236915, 385.01069967258616],
                    [385.01675726921934, 385.014039914151, 385.00588772520666],
                ]
            ),
            list([[1, 2], [3, 4, 5], [6, 7, 8, 9, 0], [10, 11, 12, 13, 14, 15]]),
        ],
        [
            list([[1.5], [0.75, 2.25], [0.5, 1.5, 2.5]]),
            list(
                [
                    [385.18664530934603],
                    [385.35005257628313, 385.16340688051844],
                    [385.25617000121963, 385.2147242350503, 385.0900900571101],
                ]
            ),
            list([[1, 2], [3, 4, 5], [6, 7, 8, 9, 0], [10, 11, 12, 13, 14, 15]]),
        ],
    ],
    dtype=object,
)


def compare_gen(nl1, nl2):
    for el_a, el_b in zip(nl1, nl2):
        if type(el_a) != type(el_b):
            yield False
        if isinstance(el_a, list):
            if len(el_a) != len(el_b):
                yield False
            yield from compare_gen(el_a, el_b)
        elif isinstance(el_a, np.ndarray):
            if el_a.shape != el_b.shape:
                yield False
            else:
                yield from compare_gen(el_a, el_b)
        else:
            yield np.isclose(el_a, el_b)


def compare(l1, l2):
    latcher = True
    print("Array 1:", l1, "Array 2:", l2, sep="\n")
    for element in compare_gen(l1, l2):
        if element != True:
            latcher = False
    return latcher


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
    _moose_python_dir = os.path.abspath(os.path.join(_moose_dir, "python"))
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
        "input_name": "stochastic_run.i",
    }
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

    if StochasticControl is None:
        _exec_dir = os.path.dirname(os.path.abspath(shutil.which(executable)))
        _share_dir = os.path.abspath(os.path.join(_exec_dir, "..", "share"))
        _moose_stm_python = os.path.join(_share_dir, "stochastic_tools", "python")
        if not tryImportStochasticControl(_moose_stm_python):
            raise ModuleNotFoundError(
                "Could not find MOOSE stochastic tools module python utilities."
            )

    if StochasticRunOptions.MultiAppMode.BATCH_RESET is not None:
        options["multiapp_mode"] = StochasticRunOptions.MultiAppMode.BATCH_RESET
    cli_args = test_options()
    parameters = [
        "capsule1:Postprocessors/frequency_factor/value",
        "capsule1:Postprocessors/activation_energy/value",
        "capsule1:BCs/heat_DRV_outer/value",
        "capsule1:mesh_specified",
    ]
    qois = ["capsule_01/x", "capsule_01/temp"]
    input_file = "vpp_test_runner.i"
    if cli_args.mode != 3:
        with StochasticControl(
            executable=executable,
            physics_input=input_file,
            parameters=parameters,
            quantities_of_interest=qois,
            options=StochasticRunOptions(**options),
        ) as runner:
            if cli_args.mode == 0:
                result = runner.configCache(tol=[1, 1e-10, 1e-14, 1e-14])
                assert all(
                    [
                        runner._result_cache.tol[i] == [1, 1e-10, 1e-14, 1e-14][i]
                        for i in range(4)
                    ]
                )
            elif cli_args.mode == 1:
                result = runner(
                    [
                        [385478, 29700, 385, 1],
                        [335478, 27700, 500, 2],
                        [485478, 21700, 385, 1],
                        [385478, 29700, 385, 1],
                    ]
                )
                assert compare(result, list_gold)
            else:
                raise ValueError("Unable to parse options")
    else:
        qois.append("capsule_01/nonsense")
        with StochasticControl(
            executable=executable,
            physics_input=input_file,
            parameters=parameters,
            quantities_of_interest=qois,
            options=StochasticRunOptions(**options),
        ) as runner:
            result = runner(
                [
                    [385478, 29700, 385, 1],
                    [335478, 27700, 500, 2],
                    [485478, 21700, 385, 1],
                    [385478, 29700, 385, 1],
                ]
            )
            assert compare(result, nestedlist_gold)
