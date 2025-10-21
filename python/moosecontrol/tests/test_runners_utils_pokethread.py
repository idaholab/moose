# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

# ruff: noqa: E402

from requests import ConnectionError, HTTPError
from time import sleep
from unittest.mock import patch

from common import (
    MooseControlTestCase,
    FakeSession,
    mock_response,
    setup_moose_python_path,
)

setup_moose_python_path()

from moosecontrol.runners.utils import Poker


class TestPoker(MooseControlTestCase):
    """
    Tests moosecontrol.runners.utils.poker.Poker.
    """

    def test(self):
        """
        Tests the basic usage of the Poker.

        Let it poke a few times and then stop.
        """
        # Spawn a Poker thread that does nothing a few times
        # and then stop it
        poll_time = 0.005
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
        # a fixed number of pokes, we'll take that number and
        # reduce it by a few.
        min_pokes_expected = num_polls / 2
        self.assertGreater(poke_thread.num_poked, min_pokes_expected)

        # Check logging; we should have:
        # - start log
        # - poke logs
        # - stop request log
        # - stopped log
        records = self._caplog.records
        self.assertGreater(len(records), 3 + min_pokes_expected)
        self.assert_log_message(0, "Poke thread started", levelname="DEBUG")
        self.assert_in_log("Poking webserver", levelname="DEBUG")
        self.assert_in_log("Poke thread requested to stop", levelname="DEBUG")
        self.assert_log_message(
            len(records) - 1, "Poke thread stopped", levelname="DEBUG"
        )

    def test_raises(self):
        """
        Tests that if the poke thread raises immediately that it'll
        exit gracefully.
        """
        session = FakeSession()
        poke_thread = Poker(1, session, "unused")
        with patch.object(session, "get", side_effect=RuntimeError):
            poke_thread.start()
            poke_thread.join()

        self.assertEqual(poke_thread.num_poked, 0)

        # Start, poke, poke failed, stopped
        self.assert_log_size(4)
        self.assert_log_message(0, "Poke thread started", levelname="DEBUG")
        self.assert_log_message(1, "Poking webserver", levelname="DEBUG")
        self.assert_log_message(
            2, "Poke raised RuntimeError; stopping", levelname="DEBUG"
        )
        self.assert_log_message(3, "Poke thread stopped", levelname="DEBUG")

    def test_non_200_status_stop(self):
        """
        Tests a non-200 status code stopping the thread.
        """
        session = FakeSession()
        poke_thread = Poker(1, session, "unused")
        with patch.object(session, "get", return_value=mock_response(status_code=201)):
            poke_thread.start()
            poke_thread.join()
        self.assert_in_log("Poke has status code 201; stopping", levelname="DEBUG")

    def test_status_raise_stop(self):
        """
        Tests raise_for_status() in the request stopping the thread.
        """
        session = FakeSession()
        poke_thread = Poker(1, session, "unused")
        with patch.object(session, "get", side_effect=HTTPError):
            poke_thread.start()
            poke_thread.join()
        self.assert_in_log("Poke raised HTTPError; stopping", levelname="DEBUG")

    def test_connection_error_stop(self):
        """
        Tests a failed connection stopping the thread.
        """
        session = FakeSession()
        poke_thread = Poker(1, session, "http://localhost:13579")
        with patch.object(session, "get", side_effect=ConnectionError):
            poke_thread.start()
            poke_thread.join()
        self.assert_in_log("Poke raised ConnectionError; stopping", levelname="DEBUG")

    def test_failed_later(self):
        """
        Tests that if the poke thread throws after a few successful
        pokes that it'll still exit gracefully.
        """
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
