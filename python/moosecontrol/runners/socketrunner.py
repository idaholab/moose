# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

"""Defines the SocketRunner."""

import os
import stat
from logging import getLogger

from moosecontrol.requests_unixsocket import Session
from moosecontrol.runners import BaseRunner

logger = getLogger("SocketRunner")


class SocketRunner(BaseRunner):
    """Runner that interacts with an already-running webserver over a socket."""

    def __init__(self, socket_path: str, *args, **kwargs):
        """
        Initialize state.

        Parameters
        ----------
        socket_path : str
            The path to the socket.
        args : list
            See BaseRunner.__init__().

        Optional Parameters
        -------------------
        **kwargs : dict
            See BaseRunner.__init__().

        """
        # The path to the socket
        self._socket_path: str = socket_path

        # Whether or not we've used the socket (used in cleanup())
        self._socket_used: bool = False

        super().__init__(*args, **kwargs)

    @property
    def url(self) -> str:
        """Get the URL for interacting with the server."""
        return f'http+unix://{self.socket_path.replace("/", "%2F")}'

    @staticmethod
    def build_session() -> Session:
        """Get a Session for interacting with the server."""
        return Session()

    @property
    def socket_path(self) -> str:
        """Get the path to the socket."""
        return self._socket_path

    @staticmethod
    def socket_exists(socket_path: str) -> bool:
        """
        Whether or not the given path exists as a socket.

        If the path doesn't exist, returns False. If the path
        exist but isn't a socket, raises FileNotFoundError.
        """
        if not os.path.exists(socket_path):
            return False

        mode = os.stat(socket_path).st_mode
        if not stat.S_ISSOCK(mode):
            raise FileNotFoundError(f"Path {socket_path} is not a socket")
        return True

    def initialize(self, data: dict):
        """
        Wait for the socket to exist and call the parent initialize.

        Parameters
        ----------
        data : dict
            The data to be passed to /initialize.

        """
        self.initialize_start()

        socket_path = self.socket_path

        def socket_exists():
            return self.socket_exists(socket_path)

        if not socket_exists():
            logger.info(f"Waiting for connection socket {socket_path}...")
            self.initialize_poll(socket_exists)
        self._socket_used = True

        logger.info(f"Found connection socket {socket_path}")

        # Let the parent initialize
        super().initialize(data)

    def delete_socket(self):
        """Delete the socket."""
        logger.info(f"Deleting socket {self.socket_path}")
        os.remove(self.socket_path)

    def finalize(self):
        """Finalize the parent and delete the socket."""
        # Let the parent finish up
        super().finalize()

        # Delete the socket
        self.delete_socket()

    def cleanup(self):
        """Cleanup the parent and delete the socket if it exists."""
        super().cleanup()

        # Delete the socket if it exists
        if self._socket_used and os.path.exists(self._socket_path):
            logger.warning("Socket still exists on cleanup; deleting")
            self.delete_socket()
