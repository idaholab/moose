# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

"""Test moosecontrol.runners.utils.poker.Poker."""

# ruff: noqa: E402

from time import sleep
from unittest.mock import patch

from requests import ConnectionError, HTTPError

from moosecontrol.runners.utils import Poker
from moosecontrol.runners.utils.poker import MAX_CONSECUTIVE_FAILURES

from .common import FakeSession, MooseControlTestCase, mock_response


class TestPoker(MooseControlTestCase):
    """Test moosecontrol.runners.utils.poker.Poker."""

    def test(self):
        """
        Tests the basic usage of the Poker.

        Let it poke a few times and then stop.
        """
        # Spawn a Poker thread that does nothing a few times
        # and then stop it
        poll_time = 0.001
        url = "foo/bar"
        session = FakeSession()

        poke_thread = Poker(poll_time, session, url)
        self.assertEqual(poke_thread.poll_time, poll_time)
        self.assertEqual(poke_thread._session, session)
        self.assertEqual(poke_thread._url, url)
        self.assertEqual(poke_thread.num_poked, 0)

        num_polls = 10
        poke_thread.start()
        sleep(num_polls * poll_time)

        # Should close the session on exit
        with patch.object(session, "close") as close:
            poke_thread.stop()
            poke_thread.join()
        close.assert_called_once()

        # This is time based, so even though we're looking for
        # a fixed number of pokes, we'll look for at least one.
        self.assertGreater(poke_thread.num_poked, 0)

        # Check logging; we should have:
        # - start log
        # - poke logs
        # - stop request log
        # - stopped log
        records = self._caplog.records
        self.assertGreater(len(records), 4)
        self.assert_log_message(0, "Poke thread started", levelname="DEBUG")
        self.assert_in_log("Poking webserver", levelname="DEBUG")
        self.assert_in_log("Poke thread requested to stop", levelname="DEBUG")
        self.assert_log_message(
            len(records) - 1, "Poke thread stopped", levelname="DEBUG"
        )

    def test_raises(self):
        """Test that the poke thread raising repeatedly results in giving up."""
        session = FakeSession()
        poke_thread = Poker(0.001, session, "unused")
        with patch.object(session, "get", side_effect=RuntimeError):
            poke_thread.start()
            poke_thread.join()

        self.assertEqual(poke_thread.num_poked, 0)

        # Start, then (poke, retry) for each of the failed attempts,
        # then giving up, then stopped
        self.assert_log_size(2 + 2 * MAX_CONSECUTIVE_FAILURES + 1)
        self.assert_log_message(0, "Poke thread started", levelname="DEBUG")
        self.assert_log_message(1, "Poking webserver", levelname="DEBUG")
        self.assert_log_message(
            2, "Poke raised RuntimeError; will retry", levelname="DEBUG"
        )
        records = self._caplog.records
        self.assert_log_message(
            len(records) - 2,
            f"Poke failed {MAX_CONSECUTIVE_FAILURES} times in a row; giving up",
            levelname="DEBUG",
        )
        self.assert_log_message(
            len(records) - 1, "Poke thread stopped", levelname="DEBUG"
        )

    def test_non_200_status_gives_up(self):
        """Test a non-200 status code being retried and eventually giving up."""
        session = FakeSession()
        poke_thread = Poker(0.001, session, "unused")
        with patch.object(session, "get", return_value=mock_response(status_code=201)):
            poke_thread.start()
            poke_thread.join()
        self.assert_in_log("Poke has status code 201; will retry", levelname="DEBUG")
        self.assert_in_log(
            f"Poke failed {MAX_CONSECUTIVE_FAILURES} times in a row; giving up",
            levelname="DEBUG",
        )

    def test_status_raise_gives_up(self):
        """Test raise_for_status() in the request being retried and giving up."""
        session = FakeSession()
        poke_thread = Poker(0.001, session, "unused")
        with patch.object(session, "get", side_effect=HTTPError):
            poke_thread.start()
            poke_thread.join()
        self.assert_in_log("Poke raised HTTPError; will retry", levelname="DEBUG")
        self.assert_in_log(
            f"Poke failed {MAX_CONSECUTIVE_FAILURES} times in a row; giving up",
            levelname="DEBUG",
        )

    def test_connection_error_gives_up(self):
        """Test a failed connection being retried and eventually giving up."""
        session = FakeSession()
        poke_thread = Poker(0.001, session, "http://localhost:13579")
        with patch.object(session, "get", side_effect=ConnectionError):
            poke_thread.start()
            poke_thread.join()
        self.assert_in_log("Poke raised ConnectionError; will retry", levelname="DEBUG")
        self.assert_in_log(
            f"Poke failed {MAX_CONSECUTIVE_FAILURES} times in a row; giving up",
            levelname="DEBUG",
        )

    def test_failed_later(self):
        """Test the poke not immediately raising will exit gracefully."""
        poke_count = 0
        allow_successful_pokes = 2

        def mock_get(*_, **__):
            nonlocal poke_count
            if poke_count == allow_successful_pokes:
                raise RuntimeError
            poke_count += 1
            return mock_response()

        session = FakeSession()
        poke_thread = Poker(0.001, session, "unused")
        with patch.object(session, "get", new=mock_get):
            poke_thread.start()
            poke_thread.join()
        self.assertEqual(poke_thread.num_poked, allow_successful_pokes)

    def test_transient_failure_retries(self):
        """Test that a transient poke failure is retried instead of stopping."""
        poll_time = 0.001
        fail_at_attempt = 3
        poke_attempts = 0

        def mock_get(*_, **__):
            nonlocal poke_attempts
            poke_attempts += 1
            if poke_attempts == fail_at_attempt:
                raise ConnectionError
            return mock_response()

        session = FakeSession()
        poke_thread = Poker(poll_time, session, "unused")
        with patch.object(session, "get", new=mock_get):
            poke_thread.start()

            # Enough time for the failure to happen and for the
            # thread to recover and keep polling afterwards
            sleep(20 * poll_time)

            # The thread should still be running despite the failure
            self.assertTrue(poke_thread.is_alive())

            poke_thread.stop()
            poke_thread.join()

        # Every attempt other than the failed one should have succeeded
        self.assertEqual(poke_thread.num_poked, poke_attempts - 1)
        self.assertGreater(poke_thread.num_poked, fail_at_attempt)

        self.assert_in_log("Poke raised ConnectionError; will retry", levelname="DEBUG")

    def test_gives_up_after_max_consecutive_failures(self):
        """Test that the poke thread stops after too many consecutive failures."""
        session = FakeSession()
        poke_thread = Poker(0.001, session, "unused")
        with patch.object(session, "get", side_effect=RuntimeError):
            poke_thread.start()
            poke_thread.join()

        self.assertEqual(poke_thread.num_poked, 0)
        self.assertEqual(poke_thread._consecutive_failures, MAX_CONSECUTIVE_FAILURES)
        self.assert_in_log(
            f"Poke failed {MAX_CONSECUTIVE_FAILURES} times in a row; giving up",
            levelname="DEBUG",
        )

    def test_consecutive_failures_reset_on_success(self):
        """Test that a success in between failures resets the failure count."""
        poke_attempts = 0
        # Fail one fewer time than the max, then succeed, twice in a row;
        # if the failure count didn't reset on success, this would give up
        fail_pattern_length = MAX_CONSECUTIVE_FAILURES

        def mock_get(*_, **__):
            nonlocal poke_attempts
            poke_attempts += 1
            if poke_attempts % fail_pattern_length == 0:
                return mock_response()
            raise RuntimeError

        session = FakeSession()
        poke_thread = Poker(0.001, session, "unused")
        with patch.object(session, "get", new=mock_get):
            poke_thread.start()
            while poke_thread.num_poked < 3:
                sleep(0.001)  # Small sleep
            self.assertTrue(poke_thread.is_alive())
            poke_thread.stop()
            poke_thread.join()

        for record in self._caplog.records:
            self.assertNotIn("giving up", record.message)
