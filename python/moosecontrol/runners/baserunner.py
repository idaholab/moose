# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

"""Defines the BaseRunner."""

from abc import ABC, abstractmethod
from contextlib import suppress
from logging import getLogger
from numbers import Number
from time import sleep
from typing import Callable, Optional

from requests import Session
from requests.exceptions import ConnectionError

from moosecontrol.exceptions import InitializeTimeout
from moosecontrol.validation import WebServerControlResponse, process_response

from .utils import Poker, TimedPoller

logger = getLogger("BaseRunner")

# Default value for 'poll_time' in BaseRunner
DEFAULT_POLL_TIME: float = 0.01
# Default value for 'poke_poll_time' in BaseRunner
DEFAULT_POKE_POLL_TIME: float = 1.0
# Default value for 'initialize_timeout' in BaseRunner
DEFAULT_INITIALIZE_TIMEOUT: float = 300.0


class BaseRunner(ABC):
    """
    Abstract base class for a runner to be paired with the MooseControl.

    Is responsible for the initial connection to the running MOOSE
    webserver.
    """

    def __init__(
        self,
        poll_time: float = DEFAULT_POLL_TIME,
        poke_poll_time: Optional[float] = DEFAULT_POKE_POLL_TIME,
        initialize_timeout: float = DEFAULT_INITIALIZE_TIMEOUT,
    ):
        """
        Initialize state.

        Parameters
        ----------
        poll_time : Number
            Time between successive message polls in seconds.
        poke_poll_time : Optional[Number]
            How often to "poke" the WebServerControl while running
            to let it know that this client is listening. If None,
            do not poke.
        initialize_timeout : Number
            Time in seconds to wait before erroring when failing
            to initialize the connection.

        """
        assert isinstance(poll_time, Number)
        assert poll_time > 0
        assert isinstance(poke_poll_time, (Number, type(None)))
        if isinstance(poke_poll_time, Number):
            assert poke_poll_time > 0
        assert isinstance(initialize_timeout, Number)
        assert initialize_timeout > 0

        # Time between polls in seconds
        self._poll_time: float = float(poll_time)
        # How often to "poke" the application; if None, don't poke
        self._poke_poll_time: Optional[float] = (
            None if poke_poll_time is None else float(poke_poll_time)
        )

        # Setup the TimedPoller for use in keeping track
        # of timing and polling during initialize()
        self._initialize_poller: TimedPoller = TimedPoller(
            self.poll_time, initialize_timeout
        )

        # Whether or not initialize() has been called
        self._initialized: bool = False
        # The data received calling the initial /initialize
        self._initialized_data: Optional[dict] = None

        # The Session for the main thread, started in initialize()
        self._session: Optional[Session] = None

        # The separate thread that "pokes" the application
        # on regular intervals to show that this client
        # is still alive
        self._poker: Optional[Poker] = None

    @property
    def poll_time(self) -> float:
        """Get the time between polls in seconds."""
        return self._poll_time

    @property
    def poke_poll_time(self) -> Optional[float]:
        """Get the time between pokes in seconds, if enabled."""
        return self._poke_poll_time

    @property
    def initialize_timeout(self) -> float:
        """Get the time in seconds to wait in initialize() until timing out."""
        return self._initialize_poller.timeout_time

    @property
    @abstractmethod
    def url(self) -> str:
        """
        Get the URL for interacting with the server.

        Must be overridden.
        """
        raise NotImplementedError  # pragma: no cover

    @property
    def initialized(self) -> bool:
        """Whether or not initialize() has been called."""
        return self._initialized

    @property
    def initialized_data(self) -> dict:
        """Get the data received when sending /initialize."""
        assert self._initialized_data is not None
        return self._initialized_data

    @staticmethod
    @abstractmethod
    def build_session() -> Session:
        """
        Build a Session for interacting with the server.

        Must be overridden.
        """
        raise NotImplementedError  # pragma: no cover

    def is_listening(self) -> bool:
        """
        Check whether or not the WebServerControl is listening.

        If a connection cannot be made, the exception is
        caught and False is returned.

        If a connection can be made and the status code
        is 200, True is returned.
        """
        try:
            response = self.get("check")
        except ConnectionError:
            return False
        return response.response.status_code == 200

    def initialize_start(self):
        """
        Start the timer for keeping track of time in initialize().

        Derived classes must call this at the top of
        their override of initialize().
        """
        self._initialize_poller.start()

    def initialize_poll(self, should_exit: Callable[[], bool]):
        """
        Poll for a criteria to be checked every self.poll_time seconds.

        Should be utilized within initialize() to wait for something to
        be available. Will raise InitializeTimeout if execution has
        happened past the initialize timeout.
        """
        try:
            self._initialize_poller.poll(should_exit)
        except TimedPoller.StartNotCalled as e:
            raise NotImplementedError(
                "initialize_start() was not called within initialize()"
            ) from e
        except TimedPoller.PollTimeout as e:
            raise InitializeTimeout(e.waited_time) from e

    def post_initialize(self, data: dict) -> dict:
        """Send /initialize to the server, storing the data."""
        logger.debug(f"Sending initialize to webserver, data={data}")
        initialize_response = self.post("initialize", data, require_status=200)
        return initialize_response.data

    def initialize(self, data: dict):
        """
        Initialize the connection; waits for the web server to be available.

        Derived classes that need to override initialize should call
        this implementation of initialize() last and should call
        initialize_start() first.

        Parameters
        ----------
        data : dict
            The data to be passed to /initialize.

        """
        assert isinstance(data, dict)
        assert not self._initialized
        assert self._initialized_data is None

        # Derived classes should also call this if they override initialize()
        self.initialize_start()

        # Start the shared Session for the main thread
        self._session = self.build_session()

        if not self.is_listening():
            logger.info("Waiting for MOOSE webserver to be listening...")
            self.initialize_poll(self.is_listening)

        logger.info("MOOSE webserver is listening")

        # Call initialize
        self._initialized_data = self.post_initialize(data)
        assert self.initialized_data is not None

        # Start the poke thread now that we are initialized
        if self.poke_poll_time is not None:
            self._poker = self._build_poker()
            self._poker.start()

        # Finalize the initialize time
        self._initialize_poller.end()

        # Mark that we've initialized
        self._initialized = True

    def stop_poker(self):
        """Stop the poke thread if it is running."""
        if self._poker is not None and self._poker.is_alive():
            logger.debug("Stopping poke poll thread")
            self._poker.stop()
            self._poker.join()
            self._poker = None
            logger.debug("Poked poll thread stopped")

    def finalize(self):
        """Finalize method; performs cleanup in a structured manner."""
        assert self._initialized
        assert self._session is not None

        # Stop the poke thread if it's running
        self.stop_poker()

        # Wait for the MOOSE webserver to stop listening
        if self.is_listening():
            logger.info("Waiting for the MOOSE webserver to stop listening...")
            while self.is_listening():
                sleep(self.poll_time)
            logger.info("MOOSE webserver is no longer listening")

        # Close the session
        self._session.close()
        self._session = None

    def cleanup(self):
        """Perform cleanup to be used when exiting non-gracefully."""
        # Stop the poke thread if it's still running (it shouldn't be!)
        if self._poker is not None:
            if self._poker.is_alive():
                logger.warning("Poke thread is still alive on cleanup; stopping")
                self.stop_poker()
            self._poker = None

        # Cleanup the session if it exists
        if self._session is not None:
            # Can only check is_listening() and call kill() if
            # we have a session
            if self.is_listening():
                logger.warning("MOOSE webserver is still listening on cleanup; killing")
                self.kill()

            logger.warning("Request session is still active on cleanup; closing")
            self._session.close()
            self._session = None

    def post(
        self, path: str, data: dict, require_status: Optional[int] = None
    ) -> WebServerControlResponse:
        """
        Send a POST request to the server.

        Parameters
        ----------
        path : str
            The path to POST to.
        data : dict
            The JSON data to send.

        Optional Parameters
        -------------------
        require_status : Optional[int]
            Check that the status code is this if set.

        Returns
        -------
        WebServerControlResponse:
            The combined response, along with the JSON data if any.

        """
        assert self._session is not None
        with self._session.post(f"{self.url}/{path}", json=data) as response:
            return process_response(response, require_status=require_status)

    def get(
        self, path: str, require_status: Optional[int] = None
    ) -> WebServerControlResponse:
        """
        Send a GET request to the server.

        Parameters
        ----------
        path : str
            The path to GET to.

        Optional Parameters
        -------------------
        require_status : Optional[int]
            Check that the status code is this if set.

        Returns
        -------
        WebServerControlResponse:
            The combined response, along with the JSON data if any.

        """
        assert self._session is not None
        with self._session.get(f"{self.url}/{path}") as response:
            return process_response(response, require_status=require_status)

    def _build_poker(self) -> Poker:
        """Build the Poker thread."""
        assert self.poke_poll_time is not None
        return Poker(self.poke_poll_time, self.build_session(), f"{self.url}/poke")

    def kill(self):
        """
        Send the kill command via GET /kill if the webserver is listening.

        Waits for the server to no longer be listening after /kill.
        """
        if self.is_listening():
            logger.info("Killing MOOSE webserver")

            # This will trigger a mooseError in MOOSE, so we can't
            # really guarantee any valid response after this
            with suppress(Exception):
                self.get("kill")

            # Wait for it to stop
            while self.is_listening():
                sleep(self.poll_time)
            logger.info("Webserver is no longer listening after kill")
