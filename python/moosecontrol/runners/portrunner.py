from .baserunner import BaseRunner
from requests import Session

import socket

DEFAULT_HOST = 'http://localhost'

class PortRunner(BaseRunner):
    """
    Runner to be used with the MooseControl to interact
    with an already-running MOOSE webserver over a port.
    """
    def __init__(self,
                 port: int,
                 host: str = DEFAULT_HOST,
                 **kwargs):
        """
        Parameters
        ----------
        port : int
            The port.

        Optional Parameters
        -------------------
        host : str
            The host; defaults to http://localhost.

        See BaseRunner.__init__() for additional parameters.
        """
        assert isinstance(port, int)
        assert isinstance(host, str)

        # The port
        self._port: int = port
        # The host
        self._host: str = host

        super().__init__(**kwargs)

    @property
    def url(self) -> str:
        """
        Get the URL for interacting with the server.
        """
        return f'{self.host}:{self.port}'

    def build_session(self) -> Session:
        """
        Get a Session for interacting with the server.
        """
        return Session()

    @property
    def port(self) -> int:
        """
        Get the port that the server is listening on.
        """
        return self._port

    @property
    def host(self) -> str:
        """
        Get the host to connect to.
        """
        return self._host

    @staticmethod
    def find_available_port() -> int:
        """
        Finds a random available local port.
        """
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
            s.bind(('', 0))
            return s.getsockname()[1]

    @staticmethod
    def port_is_available(port: int) -> bool:
        """
        Returns whether or not the given port is available.
        """
        assert isinstance(port, int)

        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
            connect = s.connect_ex(('localhost', port))
        return connect != 0
