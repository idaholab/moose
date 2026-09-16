"""
Test that ParsedPostprocessor 'expression' is a controllable parameter.

The test:
- Starts the MOOSE input with WebServerControl.
- At timestep_begin for step 2, changes the expression from 'c + t' to 'n*c + t' where n is time step.
"""

import os, sys
import importlib.util
if not (importlib.util.find_spec("moose_stochastic_tools") and importlib.util.find_spec("moose_stochastic_tools.StochasticControl")):
    MOOSE_DIR = os.environ.get("MOOSE_DIR")
    if MOOSE_DIR is None:
        HOMEDIR = os.environ.get("HOME")
        if os.path.exists(os.path.join(HOMEDIR,'source','moose')):
            MOOSE_DIR = os.path.join(HOMEDIR,'source','moose')
        elif os.path.exists(os.path.join(HOMEDIR,'projects','moose')):
            MOOSE_DIR = os.path.join(HOMEDIR,'projects','moose')
        else:
            raise ValueError("Cannot find MOOSE. Please set the $MOOSE_DIR environment variable")
        sys.path.append(os.path.join(MOOSE_DIR,'python'))
        sys.path.append(os.path.join(MOOSE_DIR,'modules/stochastic_tools/python'))
        sys.path.append(os.path.join(MOOSE_DIR,'modules/stochastic_tools/python/moose_stochastic_tools'))
    else:
        sys.path.append(os.path.join(MOOSE_DIR,'python'))
        sys.path.append(os.path.join(MOOSE_DIR,'modules/stochastic_tools/python'))
        sys.path.append(os.path.join(MOOSE_DIR,'modules/stochastic_tools/python/moose_stochastic_tools'))
    
    ### TEST CASE SETTINGS
    sys.path.append(os.path.join(MOOSE_DIR,'python'))
    sys.path.append(os.path.join(MOOSE_DIR,'modules','stochastic_tools','python'))
    sys.path.append(os.path.join(MOOSE_DIR,'modules','stochastic_tools','python','moose_stochastic_tools'))

from moosecontrol import MooseControl, SubprocessSocketRunner


def run_test(moose_exe: str, input_file: str, work_dir: str) -> None:
        command = [moose_exe, "-i", input_file]

        runner = SubprocessSocketRunner(
            command=command,
            moose_control_name="web_server",
            directory=work_dir,
        )

        with MooseControl(runner, quiet=False, verbose=True) as ctx:
            control = ctx.control

            # 1. INITIAL: let the simulation start
            control.wait(flag="INITIAL")
            control.set_continue()

            # 2. TIMESTEP_BEGIN change expr and continue
            n = 0
            for _ in range(2):  # remaining timesteps
                control.wait(flag="TIMESTEP_BEGIN")
                n += 2
                new_expr = "c + t < " + str(n)
                path = "Postprocessors/parsed/expression"
                control.set_string(path, new_expr)
                control.set_continue()
    
        # After the context manager exits, MOOSE and WebServerControl have shut down.
        # Now inspect the CSV output to confirm parsed values.
        csv_path = os.path.join(work_dir, "parsed_pp_control_out.csv")
        with open(csv_path, "r") as f:
            lines = f.readlines()

        header = lines[0].strip().split(",")
        t_idx = header.index("time")
        parsed_idx = header.index("parsed")

        times = []
        parsed_values = []
        for line in lines[1:]:
            parts = line.strip().split(",")
            times.append(float(parts[t_idx]))
            parsed_values.append(float(parts[parsed_idx]))

        before_idx = times.index(1.0)
        after_idx = times.index(2.0)

        assert abs(parsed_values[before_idx] - 0.0) < 1e-6, "Parsed value at t=1 is wrong"
        assert abs(parsed_values[after_idx] - 1.0) < 1e-6, "Parsed value at t=2 is wrong"

run_test("/Users/mechnj/projects/myforks/moose/test/moose_test-opt", "parsed_pp_control.i", "/Users/mechnj/projects/myforks/moose/test/tests/postprocessors/parsed_postprocessor")