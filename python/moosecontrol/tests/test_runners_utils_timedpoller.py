# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

"""Test moosecontrol.runners.utils.timedpoller.TimedPoller."""

# ruff: noqa: E402

from unittest import TestCase

from moosecontrol.runners.utils import TimedPoller

DEFAULT_POLL_TIME = 0.001
DEFAULT_TIMEOUT_TIME = 0.005
DEFAULT_ARGS = {"poll_time": DEFAULT_POLL_TIME, "timeout_time": DEFAULT_TIMEOUT_TIME}


class TestTimedPoller(TestCase):
    """Test moosecontrol.runners.utils.timedpoller.TimedPoller."""

    def test_init(self):
        """Tests __init__() with default arguments."""
        poller = TimedPoller(**DEFAULT_ARGS)
        self.assertEqual(poller.poll_time, DEFAULT_POLL_TIME)
        self.assertEqual(poller.timeout_time, DEFAULT_TIMEOUT_TIME)

    def test_not_started(self):
        """Test timing when not started."""
        poller = TimedPoller(**DEFAULT_ARGS)
        self.assertIsNone(poller.start_time)
        self.assertIsNone(poller.end_time)
        self.assertIsNone(poller.total_time)

    def test_start(self):
        """Test timing when started."""
        poller = TimedPoller(**DEFAULT_ARGS)
        poller.start()
        self.assertIsNotNone(poller.start_time)
        self.assertIsNone(poller.end_time)
        self.assertIsNotNone(poller.total_time)
        self.assertGreater(poller.total_time, 0)

    def test_start_multiple(self):
        """Test calling start() multiple times (allowed)."""
        poller = TimedPoller(**DEFAULT_ARGS)
        poller.start()
        start_time = poller.start_time
        poller.start()
        self.assertEqual(start_time, poller.start_time)

    def test_end(self):
        """Test timing when ended."""
        poller = TimedPoller(**DEFAULT_ARGS)
        poller.start()
        poller.end()
        self.assertIsNotNone(poller.start_time)
        self.assertIsNotNone(poller.end_time)
        self.assertGreater(poller.end_time, poller.start_time)
        self.assertEqual(poller.total_time, poller.end_time - poller.start_time)

    def test_poll(self):
        """Test poll() during normal use."""
        poller = TimedPoller(**DEFAULT_ARGS)
        poller.start()
        poller.poll(lambda: True)
        poller.end()
        self.assertGreater(poller.total_time, DEFAULT_POLL_TIME)
        self.assertLess(poller.total_time, DEFAULT_TIMEOUT_TIME)

    def test_poll_timeout(self):
        """Test poll() when a timeout is reached."""
        poller = TimedPoller(**DEFAULT_ARGS)
        poller.start()
        with self.assertRaises(TimedPoller.PollTimeout) as e_cm:
            poller.poll(lambda: False)
        self.assertIsNotNone(poller.end_time)
        self.assertGreater(poller.total_time, DEFAULT_TIMEOUT_TIME)
        self.assertEqual(
            str(e_cm.exception), f"Timed out after {poller.total_time:.2f} seconds"
        )

    def test_poll_no_start(self):
        """Test poll() when start is not called."""
        poller = TimedPoller(**DEFAULT_ARGS)
        with self.assertRaises(TimedPoller.StartNotCalled):
            poller.poll(lambda: False)

    def test_context_manager(self):
        """Test the context manager."""
        with TimedPoller(**DEFAULT_ARGS) as poller:
            poller.poll(lambda: True)
        self.assertIsInstance(poller.total_time, float)
