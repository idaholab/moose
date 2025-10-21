#* This file is part of the MOOSE framework
#* https://mooseframework.inl.gov
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

"""Defines the SubprocessSocketRunner."""

import os
from logging import getLogger
from random import choice
from string import ascii_lowercase, digits
from tempfile import gettempdir
from typing import Optional

from .socketrunner import SocketRunner
from .subprocessrunnerbase import SubprocessRunnerBase, DEFAULT_DIRECTORY

logger = getLogger('SubprocessSocketRunner')

class SubprocessSocketRunner(SubprocessRunnerBase, SocketRunner):
    """
    Runner to be used with the MooseControl that
    spawns a MOOSE process and connects to the
    webserver over a socket.
    """
    # pylint: disable=too-many-arguments,too-many-positional-arguments,R0801
    def __init__(self,
                 command: list[str],
                 moose_control_name: str,
                 socket_path: Optional[str] = None,
                 directory: str = DEFAULT_DIRECTORY,
                 use_subprocess_reader: bool = True,
                 **kwargs):
        """
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

        See BaseRunner.__init__() for additional parameters.
        """
        SubprocessRunnerBase.__init__(
            self,
            command=command,
            moose_control_name=moose_control_name,
            directory=directory,
            use_subprocess_reader=use_subprocess_reader
        )

        # Build a random socket name if one was not provided
        if socket_path is None:
            socket_path = self.random_socket_path()
        else:
            socket_path = os.path.abspath(socket_path)

        SocketRunner.__init__(
            self,
            socket_path=socket_path,
            **kwargs
        )

    @staticmethod
    def random_socket_path() -> str:
        """
        Generates a random socket path in the temporary directory.
        """
        characters = ascii_lowercase + digits
        name = ''.join(choice(characters) for i in range(7))
        return os.path.join(gettempdir(), f'moosecontrol_{name}.sock')

    def get_additional_command(self) -> list[str]:
        """
        Gets the full command to run.

        Takes the user's command and also:
            - Sets the file socket for the control
            - Disables color in output
        """
        control_path = f'Controls/{self.moose_control_name}'
        control_socket = f'{control_path}/file_socket="{self.socket_path}"'
        return [control_socket, '--color=off']

    def initialize(self):
        """
        Initializes the runner.
        """
        self.initialize_start()

        if os.path.exists(self.socket_path):
            raise FileExistsError(f'Socket {self.socket_path} already exists')

        # Start the subprocess
        SubprocessRunnerBase.initialize(self)
        # And then wait for a connection
        SocketRunner.initialize(self)

    def finalize(self):
        """
        Finalizes the runner.
        """
        # Wait for process to finish
        SubprocessRunnerBase.finalize(self)
        # And then delete the socket
        SocketRunner.finalize(self)

    def cleanup(self):
        """
        Performs cleanup.

        Should ideally do nothing unless something went wrong.
        """
        # Kill process if needed
        SubprocessRunnerBase.cleanup(self)
        # And then cleanup the socket
        SocketRunner.cleanup(self)
