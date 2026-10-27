# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

"""Defines the Poker."""

from logging import getLogger
from numbers import Number
from threading import Event, Thread

from requests import Session

logger = getLogger("Poker")

# Number of consecutive failed pokes allowed before giving up
MAX_CONSECUTIVE_FAILURES: int = 5


class Poker(Thread):
    """
    Thread that "pokes" the web server every so often.

    Parameters
    ----------
    poll_time : float
        How often to do the poke in seconds.
    session : Session
        The Session to use to make the poke requests.
    url : str
        The url to call GET to in order to poke.

    """

    def __init__(self, poll_time: float, session: Session, url: str):
        """
        Initialize state.

        Parameters
        ----------
        poll_time : float
            How often to poll in seconds.
        session : Session
            The session to use to connect.
        url : str
            The URL to call GET on to do the poke.

        """
        assert isinstance(poll_time, Number)
        assert poll_time > 0
        assert isinstance(session, Session)
        assert isinstance(url, str)

        super().__init__()

        # How often to poke
        self._poll_time: float = float(poll_time)
        # The Session to poke with
        self._session: Session = session
        # The url to poke to
        self._url: str = url
        # Event so that the owning thread can stop this one
        self._stop_event: Event = Event()
        # The number of times that we've poked
        self._num_poked: int = 0
        # The number of consecutive failed pokes
        self._consecutive_failures: int = 0

    @property
    def poll_time(self) -> float:
        """How often to poll in seconds."""
        return self._poll_time

    @property
    def num_poked(self) -> int:
        """The number of times that we've poked."""
        return self._num_poked

    def poke(self):
        """Poke the server."""
        assert self._session is not None
        with self._session.get(self._url) as request:
            request.raise_for_status()
        return request

    def run(self):
        """Run the poke thread."""
        logger.debug("Poke thread started")

        # Poll until we've been told not to. A failed poke is treated as
        # transient (e.g. a momentary HTTP hiccup on the webserver's control
        # channel) and retried on the next tick rather than stopping the
        # thread outright - MOOSE only resets its client timeout on a
        # successful poke, so silently giving up here would eventually cause
        # an unrelated-looking fatal client_timeout error on the MOOSE side.
        # If pokes keep failing, though, give up after MAX_CONSECUTIVE_FAILURES
        # in a row instead of retrying forever.
        while not self._stop_event.is_set():
            logger.debug("Poking webserver")
            try:
                request = self.poke()
            except Exception as e:
                self._consecutive_failures += 1
                logger.debug(f"Poke raised {type(e).__name__}; will retry")
            else:
                if request.status_code != 200:
                    self._consecutive_failures += 1
                    logger.debug(
                        f"Poke has status code {request.status_code}; will retry"
                    )
                else:
                    self._consecutive_failures = 0
                    self._num_poked += 1

            if self._consecutive_failures >= MAX_CONSECUTIVE_FAILURES:
                logger.debug(
                    f"Poke failed {self._consecutive_failures} times in a "
                    "row; giving up"
                )
                break

            self._stop_event.wait(self.poll_time)

        # Close the session
        self._session.close()

        logger.debug("Poke thread stopped")

    def stop(self):
        """Tell the thread to stop."""
        logger.debug("Poke thread requested to stop")
        self._stop_event.set()
