# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

"""Defines the SubprocessPortRunner."""

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

from .portrunner import PortRunner

logger = getLogger("SubprocessPortRunner")


class SubprocessPortRunner(SubprocessRunnerInterface, PortRunner):
    """Runner that spawns a MOOSE process and connects to it via a socket."""

    def __init__(
        self,
        command: list[str],
        moose_control_name: str,
        port: Optional[int] = None,
        directory: str = DEFAULT_DIRECTORY,
        use_subprocess_reader: bool = True,
        *args,
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
        args : list
            See PortRunner.__init__().

        Optional Parameters
        -------------------
        port : Optional[int]
            The port to connect to. If unset, the
            application binds a free port of the
            operating system's choosing and reports
            it back.
        directory : str
            Directory to run in. Defaults to the current
            working directory.
        use_subprocess_reader : bool
            Whether or not to spawn a separate reader thread
            to collect subprocess output. Defaults to true.
        **kwargs : dict
            See PortRunner.__init__().

        """
        assert isinstance(port, (int, type(None)))

        SubprocessRunnerInterface.__init__(
            self,
            command=command,
            moose_control_name=moose_control_name,
            directory=directory,
            use_subprocess_reader=use_subprocess_reader,
        )

        # With no port provided, ask for port zero, which the application
        # resolves to a free port of the operating system's choosing and then
        # publishes in this file. A free port cannot be reserved from here: a
        # probe has to release the port before the application can bind it.
        self._port_file: Optional[str] = None
        if port is None:
            self._port_file = self.random_port_file_path()
            port = 0

        PortRunner.__init__(self, port=port, *args, **kwargs)

    @property
    def port_file(self) -> Optional[str]:
        """Get the file the application reports its port in, if it chooses one."""
        return self._port_file

    @staticmethod
    def random_port_file_path() -> str:
        """Generate a random port file path in the temporary directory."""
        characters = ascii_lowercase + digits
        name = "".join(choice(characters) for i in range(7))
        return os.path.join(gettempdir(), f"moosecontrol_{name}.port")

    def get_additional_command(self) -> list[str]:
        """
        Get the full command to run.

        Takes the user's command and also:
            - Sets the port for the control
            - Sets where to report the bound port, if the application chooses it
            - Disables color in output
        """
        control_path = f"Controls/{self.moose_control_name}"
        command = [f"{control_path}/port={self.port}"]
        if self.port_file is not None:
            command.append(f'{control_path}/port_file="{self.port_file}"')
        command.append("--color=off")
        return command

    def resolve_port(self):
        """
        Wait for the application to publish the port it bound, and adopt it.

        The application renames the file into place only once its socket is
        listening, so a file that exists holds a port that can be connected to.
        """
        assert self._port_file is not None

        logger.info("Waiting for MOOSE to report the server port...")
        self.initialize_poll(lambda: os.path.exists(self._port_file))

        with open(self._port_file) as f:
            port = int(f.read().strip())
        logger.info(f"MOOSE server is listening on port {port}")

        self._set_port(port)

    def delete_port_file(self):
        """Delete the port file if the application wrote one."""
        if self._port_file is not None and os.path.exists(self._port_file):
            os.remove(self._port_file)

    def initialize(self, data: dict):
        """
        Spawn the process and wait for the server to be listening.

        Parameters
        ----------
        data : dict
            The data to be passed to /initialize.

        """
        self.initialize_start()

        # A port named by the caller can be checked before spawning; one the
        # application chooses needs no check, as it binds what it reports
        if self.port_file is None and not self.port_is_available(self.port):
            raise ConnectionRefusedError(f"Port {self.port} is already used")

        # Start the subprocess
        SubprocessRunnerInterface.initialize(self)
        # Learn where it is listening before trying to reach it
        if self.port_file is not None:
            self.resolve_port()
        # And then wait for a connection
        PortRunner.initialize(self, data)

    def finalize(self):
        """Finalize the process and the connection."""
        # Wait for process to finish
        SubprocessRunnerInterface.finalize(self)
        # And then close the connection
        PortRunner.finalize(self)
        # And drop the port file, which has served its purpose
        self.delete_port_file()

    def cleanup(self):
        """Kill the server and the process."""
        # And then cleanup the connection
        PortRunner.cleanup(self)
        # Kill process if needed
        SubprocessRunnerInterface.cleanup(self)
        # And drop the port file, which may not have been read
        self.delete_port_file()
