# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

"""Defines the SubprocessSocketRunner."""

import os
from logging import getLogger
from random import choice
from string import ascii_lowercase, digits
from tempfile import gettempdir
from typing import Optional

from moosecontrol.runners.interfaces.subprocessrunnerinterface import (
    DEFAULT_DIRECTORY,
    SubprocessRunnerInterface,
)

from .socketrunner import SocketRunner

logger = getLogger("SubprocessSocketRunner")


class SubprocessSocketRunner(SubprocessRunnerInterface, SocketRunner):
    """Runner that spawns a MOOSE process and connects to it via a port."""

    def __init__(
        self,
        command: list[str],
        moose_control_name: str,
        socket_path: Optional[str] = None,
        directory: str = DEFAULT_DIRECTORY,
        use_subprocess_reader: bool = True,
        **kwargs,
    ):
        """
        Initialize state.

        Parameters
        ----------
        command : list[str]
            The command to spawn the subprocess.
        moose_control_name : str
            The name of the WebServerControl in input.

        Optional Parameters
        -------------------
        socket_path : Optional[str]
            The socket path to use. If unset, build
            a random onw in the directory.
        directory : str
            Directory to run in. Defaults to the current
            working directory.
        use_subprocess_reader : bool
            Whether or not to spawn a separate reader thread
            to collect subprocess output. Defaults to true.
        **kwargs : dict
            See SocketRunner.__init__().

        """
        SubprocessRunnerInterface.__init__(
            self,
            command=command,
            moose_control_name=moose_control_name,
            directory=directory,
            use_subprocess_reader=use_subprocess_reader,
        )

        # Build a random socket name if one was not provided
        if socket_path is None:
            socket_path = self.random_socket_path()
        else:
            socket_path = os.path.abspath(socket_path)

        SocketRunner.__init__(self, socket_path=socket_path, **kwargs)

    @staticmethod
    def random_socket_path() -> str:
        """Generate a random socket path in the temporary directory."""
        characters = ascii_lowercase + digits
        name = "".join(choice(characters) for i in range(7))
        return os.path.join(gettempdir(), f"moosecontrol_{name}.sock")

    def get_additional_command(self) -> list[str]:
        """
        Get the full command to run.

        Takes the user's command and also:
            - Sets the file socket for the control
            - Disables color in output
        """
        control_path = f"Controls/{self.moose_control_name}"
        control_socket = f'{control_path}/file_socket="{self.socket_path}"'
        return [control_socket, "--color=off"]

    def initialize(self, data: dict):
        """
        Spawn the process and wait for the server to be listening.

        Parameters
        ----------
        data : dict
            The data to be passed to /initialize.

        """
        self.initialize_start()

        if os.path.exists(self.socket_path):
            raise FileExistsError(f"Socket {self.socket_path} already exists")

        # Start the subprocess
        SubprocessRunnerInterface.initialize(self)
        # And then wait for a connection
        SocketRunner.initialize(self, data)

    def finalize(self):
        """Finalize the process and the connection."""
        # Wait for process to finish
        SubprocessRunnerInterface.finalize(self)
        # And then delete the socket
        SocketRunner.finalize(self)

    def cleanup(self):
        """Kill the server and the process."""
        # And then cleanup the socket
        SocketRunner.cleanup(self)
        # Kill process if needed
        SubprocessRunnerInterface.cleanup(self)
