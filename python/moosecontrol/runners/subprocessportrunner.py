# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

"""Defines the SubprocessPortRunner."""

from logging import getLogger
from typing import Optional

from .portrunner import PortRunner
from .subprocessrunnerbase import SubprocessRunnerBase, DEFAULT_DIRECTORY

logger = getLogger("SubprocessPortRunner")


class SubprocessPortRunner(SubprocessRunnerBase, PortRunner):
    """
    Runner to be used with the MooseControl that
    spawns a MOOSE process and connects to the
    webserver over a port.
    """

    def __init__(
        self,
        command: list[str],
        moose_control_name: str,
        port: Optional[int] = None,
        directory: str = DEFAULT_DIRECTORY,
        use_subprocess_reader: bool = True,
        **kwargs,
    ):
        """
        Parameters
        ----------
        command : list[str]
            The command to spawn the subprocess.
        moose_control_name : str
            The name of the WebServerControl in input.

        Optional Parameters
        -------------------
        port : Optional[int]
            The port to connect to. If unset, find
            a random available port.
        directory : str
            Directory to run in. Defaults to the current
            working directory.
        use_subprocess_reader : bool
            Whether or not to spawn a separate reader thread
            to collect subprocess output. Defaults to true.

        See BaseRunner.__init__() for additional parameters.
        """
        assert isinstance(port, (int, type(None)))

        SubprocessRunnerBase.__init__(
            self,
            command=command,
            moose_control_name=moose_control_name,
            directory=directory,
            use_subprocess_reader=use_subprocess_reader,
        )

        # Find an available port if one was not provided
        if port is None:
            port = PortRunner.find_available_port()

        PortRunner.__init__(self, port=port, **kwargs)

    def get_additional_command(self) -> list[str]:
        """
        Gets the full command to run.

        Takes the user's command and also:
            - Sets the port for the control
            - Disables color in output
        """
        control_path = f"Controls/{self.moose_control_name}"
        control_socket = f"{control_path}/port={self.port}"
        return [control_socket, "--color=off"]

    def initialize(self):
        """
        Initializes the runner.
        """
        self.initialize_start()

        if not self.port_is_available(self.port):
            raise ConnectionRefusedError(f"Port {self.port} is already used")

        # Start the subprocess
        SubprocessRunnerBase.initialize(self)
        # And then wait for a connection
        PortRunner.initialize(self)

    def finalize(self):
        """
        Finalizes the runner.
        """
        # Wait for process to finish
        SubprocessRunnerBase.finalize(self)
        # And then close the connection
        PortRunner.finalize(self)

    def cleanup(self):
        """
        Performs cleanup.

        Should ideally do nothing unless something went wrong.
        """
        # Kill process if needed
        SubprocessRunnerBase.cleanup(self)
        # And then cleanup the connection
        PortRunner.cleanup(self)
