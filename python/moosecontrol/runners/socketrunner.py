#* This file is part of the MOOSE framework
#* https://mooseframework.inl.gov
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import os
import stat
from logging import getLogger
from typing import Optional

from moosecontrol.requests_unixsocket import Session
from moosecontrol.runners import BaseRunner

logger = getLogger('SocketRunner')

class SocketRunner(BaseRunner):
    """
    Runner to be used with the MooseControl to interact
    with an already-running MOOSE webserver over a socket.
    """
    def __init__(self, socket_path: os.PathLike, **kwargs):
        """
        Parameters
        ----------
        socket_path : os.PathLike
            The path to the socket.

        Optional Parameters
        -------------------
        See BaseRunner.__init__() for additional parameters.
        """
        # The path to the socket
        self._socket_path: str = str(socket_path)

        # Whether or not we've used the socket (used in cleanup())
        self._socket_used: bool = False

        super().__init__(**kwargs)

    @property
    def url(self) -> str:
        """
        Get the URL for interacting with the server.
        """
        return f'http+unix://{self.socket_path.replace("/", "%2F")}'

    def build_session(self) -> Session:
        """
        Get a Session for interacting with the server.
        """
        return Session()

    @property
    def socket_path(self) -> str:
        """
        Get the path to the socket.
        """
        return self._socket_path

    @staticmethod
    def socket_exists(socket_path: os.PathLike) -> bool:
        """
        Helper for checking if the given path exists as a socket.

        If the path doesn't exist, returns False. If the path
        exist but isn't a socket, raises FileNotFoundError.
        """
        if not os.path.exists(socket_path):
            return False

        mode = os.stat(socket_path).st_mode
        if not stat.S_ISSOCK(mode):
            raise FileNotFoundError(f'Path {socket_path} is not a socket')
        return True

    def initialize(self):
        """
        Initializes the runner.

        Waits for the socket to exist and then calls
        BaseRunner.initialize().
        """
        self.initialize_start()

        socket_path = self.socket_path
        socket_exists = lambda: self.socket_exists(socket_path)

        if not socket_exists():
            logger.info(f'Waiting for connection socket {socket_path}...')
            self.initialize_poll(socket_exists)
        self._socket_used = True

        logger.info(f'Found connection socket {socket_path}')

        # Let the parent initialize
        super().initialize()

    def delete_socket(self):
        """
        Delete the socket.
        """
        logger.info(f'Deleting socket {self.socket_path}')
        os.remove(self.socket_path)

    def finalize(self):
        """
        Finalizes the runner.

        Calls finalize() in the BaseRunner and cleans up
        the socket.
        """
        # Let the parent finish up
        super().finalize()

        # Delete the socket
        self.delete_socket()

    def cleanup(self):
        """
        Performs cleanup.

        Should ideally do nothing unless something went wrong.
        """
        super().cleanup()

        # Delete the socket if it exists
        if self._socket_used and os.path.exists(self._socket_path):
            logger.warning('Socket still exists on cleanup; deleting')
            self.delete_socket()
