#* This file is part of the MOOSE framework
#* https://mooseframework.inl.gov
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from unittest import TestCase

from common import setup_moose_python_path
setup_moose_python_path()

from moosecontrol.runners.utils import TimedPollHelper

DEFAULT_POLL_TIME = 0.001
DEFAULT_TIMEOUT_TIME = 0.005
DEFAULT_ARGS = {
    'poll_time': DEFAULT_POLL_TIME,
    'timeout_time': DEFAULT_TIMEOUT_TIME
}

class TestTimedPollHelper(TestCase):
    """
    Tests moosecontrol.runners.utils.timedpollhelper.TimedPollHelper.
    """
    def test_init(self):
        """
        Tests initializing.
        """
        helper = TimedPollHelper(**DEFAULT_ARGS)
        self.assertEqual(helper.poll_time, DEFAULT_POLL_TIME)
        self.assertEqual(helper.timeout_time, DEFAULT_TIMEOUT_TIME)

    def test_not_started(self):
        """
        Tests timing when not started.
        """
        helper = TimedPollHelper(**DEFAULT_ARGS)
        self.assertIsNone(helper.start_time)
        self.assertIsNone(helper.end_time)
        self.assertIsNone(helper.total_time)

    def test_start(self):
        """
        Tests timing when started.
        """
        helper = TimedPollHelper(**DEFAULT_ARGS)
        helper.start()
        self.assertIsNotNone(helper.start_time)
        self.assertIsNone(helper.end_time)
        self.assertIsNotNone(helper.total_time)
        self.assertGreater(helper.total_time, 0)

    def test_start_multiple(self):
        """
        Tests calling start() multiple times (allowed).
        """
        helper = TimedPollHelper(**DEFAULT_ARGS)
        helper.start()
        start_time = helper.start_time
        helper.start()
        self.assertEqual(start_time, helper.start_time)

    def test_end(self):
        """
        Tests timing when ended.
        """
        helper = TimedPollHelper(**DEFAULT_ARGS)
        helper.start()
        helper.end()
        self.assertIsNotNone(helper.start_time)
        self.assertIsNotNone(helper.end_time)
        self.assertGreater(helper.end_time, helper.start_time)
        self.assertEqual(helper.total_time, helper.end_time - helper.start_time)

    def test_poll(self):
        """
        Tests poll() during normal use.
        """
        helper = TimedPollHelper(**DEFAULT_ARGS)
        helper.start()
        helper.poll(lambda: True)
        helper.end()
        self.assertGreater(helper.total_time, DEFAULT_POLL_TIME)
        self.assertLess(helper.total_time, DEFAULT_TIMEOUT_TIME)

    def test_poll_timeout(self):
        """
        Tests poll() when a timeout is reached.
        """
        helper = TimedPollHelper(**DEFAULT_ARGS)
        helper.start()
        with self.assertRaises(TimedPollHelper.PollTimeout) as e_cm:
            helper.poll(lambda: False)
        self.assertIsNotNone(helper.end_time)
        self.assertGreater(helper.total_time, DEFAULT_TIMEOUT_TIME)
        self.assertEqual(
            str(e_cm.exception),
            f'Timed out after {helper.total_time:.2f} seconds'
        )

    def test_poll_no_start(self):
        """
        Tests poll() when start is not called.
        """
        helper = TimedPollHelper(**DEFAULT_ARGS)
        with self.assertRaises(TimedPollHelper.StartNotCalled):
            helper.poll(lambda: False)
