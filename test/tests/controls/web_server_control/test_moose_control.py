#* This file is part of the MOOSE framework
#* https://mooseframework.inl.gov
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import math
import os
import shlex
import sys
from numbers import Number

from moosecontrol import SubprocessPortRunner, SubprocessSocketRunner, MooseControlNew
from moosecontrol.runners.subprocessrunnerbase import SubprocessRunnerBase

class TestMooseControl:
    """
    Foo
    """
    def __init__(self, control_name: str, use_port: bool = False):
        """
        Arguments
        ---------
        control_name : str
            The name of the WebServercontrol

        Additional Arguments
        --------------------
        use_port : bool
            Set to True to use a port instead of a socket;
            default is False.
        """
        assert isinstance(control_name, str)
        assert isinstance(use_port, bool)

        # The name of the WebServerControl
        self._control_name: str = control_name

        # Get the command to run (from env)
        command_var = 'RUNAPP_COMMAND'
        command = os.environ.get(command_var)
        if not command:
            raise SystemExit(f'Missing command variable {command}')
        command = shlex.split(command)
        command += [f'Controls/{control_name}/client_timeout=10']

        # Setup the control
        runner_type = SubprocessPortRunner if use_port else SubprocessSocketRunner
        self._runner: SubprocessRunnerBase = runner_type(
            command=command,
            moose_control_name=control_name,
            initialize_timeout=1
        )
        self._control: MooseControlNew = MooseControlNew(
            self._runner,
            verbose=True
        )

    def __enter__(self) -> MooseControlNew:
        self._control.initialize()
        return self._control

    def __exit__(self, exc_type, exc_value, exc_traceback):
        if exc_type is not None:
            self._control.cleanup()
        else:
            self._control.finalize()
            sys.exit(self._runner.get_return_code())

def expect_equal(gold: Number, value: Number):
    """
    Raises ValueError if the two values aren't equal.
    """
    if gold != value:
        raise ValueError(f'"{gold}" != "{value}"')

def expect_close(gold: float, value: float):
    """
    Raises ValueError if the two values aren't close.
    """
    if not math.isclose(gold, value):
        raise ValueError(f'"{gold}" != "{value}"')
