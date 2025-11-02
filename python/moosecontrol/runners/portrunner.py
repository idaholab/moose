# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

"""Defines the PortRunner."""

import socket

from requests import Session

from .baserunner import BaseRunner

DEFAULT_HOST = "http://localhost"


class PortRunner(BaseRunner):
    """Runner that interacts with an already-running webserver over a port."""

    def __init__(self, port: int, host: str = DEFAULT_HOST, *args, **kwargs):
        """
        Initialize state.

        Parameters
        ----------
        port : int
            The port.
        args : list
            See BaseRunner.__init__().

        Optional Parameters
        -------------------
        host : str
            The host; defaults to http://localhost.
        **kwargs : dict
            See BaseRunner.__init__().

        """
        assert isinstance(port, int)
        assert isinstance(host, str)

        # The port
        self._port: int = port
        # The host
        self._host: str = host

        super().__init__(*args, **kwargs)

    @property
    def url(self) -> str:
        """Get the URL for interacting with the server."""
        return f"{self.host}:{self.port}"

    @staticmethod
    def build_session() -> Session:
        """Get a Session for interacting with the server."""
        return Session()

    @property
    def port(self) -> int:
        """Get the port that the server is listening on."""
        return self._port

    @property
    def host(self) -> str:
        """Get the host to connect to."""
        return self._host

    @staticmethod
    def find_available_port() -> int:
        """Find a random available local port."""
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
            s.bind(("", 0))
            return s.getsockname()[1]

    @staticmethod
    def port_is_available(port: int) -> bool:
        """Whether or not the given port is available."""
        assert isinstance(port, int)

        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
            connect = s.connect_ex(("localhost", port))
        return connect != 0
