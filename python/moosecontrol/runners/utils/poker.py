#* This file is part of the MOOSE framework
#* https://mooseframework.inl.gov
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

# pylint: disable=logging-fstring-interpolation

"""Defines the Poker."""

from logging import getLogger
from numbers import Number
from threading import Event, Thread

from requests import Session

logger = getLogger('Poker')

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

    @property
    def poll_time(self) -> float:
        """
        Return how often to poll in seconds.
        """
        return self._poll_time

    @property
    def num_poked(self) -> int:
        """
        Return the number of times that we've poked.
        """
        return self._num_poked

    def poke(self):
        """
        Performs the poke.
        """
        assert self._session is not None
        with self._session.get(self._url) as request:
            request.raise_for_status()
        return request

    def run(self):
        logger.debug('Poke thread started')

        # Poll until we've been told not to or until
        # a poke fails
        while not self._stop_event.is_set():
            logger.debug('Poking webserver')
            try:
                request = self.poke()
            except Exception as e: # pylint: disable=broad-exception-caught
                logger.debug(f'Poke raised {type(e).__name__}; stopping')
                break
            if request.status_code != 200:
                logger.debug(f'Poke has status code {request.status_code}; stopping')
                break
            self._num_poked += 1
            self._stop_event.wait(self.poll_time)

        # Close the session
        self._session.close()

        logger.debug('Poke thread stopped')

    def stop(self):
        """
        Tell the run thread to stop.
        """
        logger.debug('Poke thread requested to stop')
        self._stop_event.set()
