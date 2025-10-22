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
from numbers import Number

from moosecontrol import SubprocessPortRunner, SubprocessSocketRunner, MooseControl
from moosecontrol.runners.interfaces import SubprocessRunnerInterface

class TestMooseControl:
    """
    Helper for testing the MooseControl within TestHarness
    python unit tests with the command_proxy test spec option,
    which sets the proxy command in the RUNAPP_COMMAND
    environment variable.
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

        # Get the command to run (from env)
        command_str = "RUNAPP_COMMAND"
        command = os.environ.get(command_str)
        if not command:
            raise SystemExit(f'Missing command variable {command}')
        command = shlex.split(command)

        # Setup the control
        runner_type = SubprocessPortRunner if use_port else SubprocessSocketRunner
        self._runner: SubprocessRunnerInterface = runner_type(
            command=command, moose_control_name=control_name, initialize_timeout=5
        )
        self._control: MooseControl = MooseControl(
            self._runner,
            verbose=True
        )

    @property
    def control(self) -> MooseControl:
        """
        Gets the underlying MooseControl.

        Can be utilized instead of the context manager if needed.
        """
        return self._control

    def __enter__(self) -> MooseControl:
        """
        Context manager enter; uses the underlying enter
        from the MooseControl.
        """
        return self.control.__enter__().control

    def __exit__(self, *args):
        """
        Context manager exit; uses the underlying exit from
        the MooseControl.
        """
        self.control.__exit__(*args)

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
