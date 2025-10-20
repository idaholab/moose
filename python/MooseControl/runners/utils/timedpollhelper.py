#* This file is part of the MOOSE framework
#* https://mooseframework.inl.gov
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from numbers import Number
from typing import Callable, Optional
from time import monotonic, sleep

class TimedPollHelper:
    """
    Helper for performing an action in a polled fashion,
    while also raising an exception when a timeout is reached.

    Example
    -------
    >>>> helper = TimedPollHelper(poll_time=0.1, timeout_time=10)
    >>>> helper.start()
    >>>>
    >>>> # Develop some criteria on when to exit
    >>>> should_exit = lambda: True # fake criteria
    >>>>
    >>>> # Poll until should_exit is true, which can be done
    >>>> # multiple times. If the timeout time is reached,
    >>>> # these calls to poll() will raise TimedPollTimeout
    >>>> helper.poll(should_exit)
    >>>> helper.poll(should_exit)
    >>>>
    >>>> # Finalize timing if it succeeds
    >>>> helper.end()
    """
    def __init__(self, poll_time: Number, timeout_time: Number):
        """
        Parameters
        ----------
        poll_time : Number
            How often to poll in seconds.
        timeout_time : Number
            How many seconds may elapse before raising TimedPollTimeout
            when calling poll().
        """
        assert isinstance(poll_time, Number)
        assert poll_time > 0
        assert isinstance(timeout_time, Number)
        assert timeout_time > 0

        # How often to poll in seconds
        self._poll_time: float = float(poll_time)
        # Timeout time during polling in seconds
        self._timeout_time: float = float(timeout_time)

        # Start time
        self._start_time: Optional[float] = None
        # End time
        self._end_time: Optional[float] = None

    @property
    def poll_time(self) -> float:
        """
        How often to poll in seconds.
        """
        return self._poll_time

    @property
    def timeout_time(self) -> float:
        """
        Timeout time during polling in seconds.
        """
        return self._timeout_time

    @property
    def start_time(self) -> Optional[float]:
        """
        The time when polling started (if started).

        The actual time is only used in conjunction
        with the current time or self.end_time in order
        to determine self.total_time.
        """
        return self._start_time

    @property
    def end_time(self) -> Optional[float]:
        """
        The time when ended (if started and ended).

        The actual time is only used in conjunction
        with the self.start_time in order
        to determine self.total_time.
        """
        return self._end_time

    @property
    def total_time(self) -> Optional[Number]:
        """
        The total time spent polling in seconds.

        If not started, returns None.

        If started and not ended, returns the
        difference between the start time and now.

        If started and ended, returns the
        difference between the start end end times.
        """
        if self.start_time is not None:
            if self.end_time is not None:
                return self.end_time - self.start_time
            return monotonic() - self.start_time
        return None

    def start(self):
        """
        Starts the timing.

        Must be called before using poll().
        """
        if self.start_time is None:
            self._start_time = monotonic()

    def end(self):
        """
        Finishes the timing.
        """
        assert self.start_time is not None
        self._end_time = monotonic()

    class PollTimeout(Exception):
        """
        Exception raised when the timeout is hit during poll()
        """
        def __init__(self, waited_time: float):
            self.waited_time: float = waited_time
            message = f'Timed out after {waited_time:.2f} seconds'
            super().__init__(message)

    class StartNotCalled(Exception):
        """
        Exception for calling poll() without start() in TimedPollHelper.
        """
        def __init__(self):
            super().__init__('poll() was called without calling start()')

    def poll(self, should_exit: Callable[[], bool]):
        """
        Poll to do something.

        If should_exit returns True, the poll loop will break.

        If the total time elapsed exceeds the timeout time,
        this will raise a TimedPollTimeout.
        """
        # Must call start() first
        if self.start_time is None:
            raise self.StartNotCalled

        # Keep checking the criteria until criteria is
        # met or we have gone past the timeout
        while self.total_time < self.timeout_time:
            sleep(self.poll_time)
            if should_exit():
                return

        # We have timed out if we reach this
        self.end()
        raise self.PollTimeout(self.total_time)
