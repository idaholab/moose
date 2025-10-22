# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

from moosecontrol.exceptions import InitializeTimeout
from moosecontrol.runners import BaseRunner
from moosecontrol.runners.baserunner import (
    DEFAULT_POLL_TIME,
    DEFAULT_POKE_POLL_TIME,
    DEFAULT_INITIALIZE_TIMEOUT,
)

from unittest.mock import MagicMock, patch
from requests import Session
from requests.exceptions import ConnectionError

from common import (
    FAKE_URL,
    MooseControlTestCase,
    FakeSession,
    mock_response,
    setup_moose_python_path,
)

setup_moose_python_path()

BASERUNNER = "moosecontrol.runners.BaseRunner"


class BaseRunnerTest(BaseRunner):
    @property
    def url(self):
        return FAKE_URL

    @staticmethod
    def build_session():
        return FakeSession()


def patch_baserunner(name: str, **kwargs):
    """
    Convenience method for patching the BaseRunner.
    """
    return patch(f"moosecontrol.runners.BaseRunner.{name}", **kwargs)


class TestBaseRunner(MooseControlTestCase):
    """
    Tests moosecontrol.runners.baserunner.BaseRunner.
    """

    def test_init(self):
        """
        Tests init with the default arguments.
        """
        runner = BaseRunnerTest()
        self.assertEqual(runner.url, FAKE_URL)
        self.assertEqual(runner.poll_time, DEFAULT_POLL_TIME)
        self.assertEqual(runner.poke_poll_time, DEFAULT_POKE_POLL_TIME)
        self.assertEqual(runner.initialize_timeout, DEFAULT_INITIALIZE_TIMEOUT)

    def test_init_args(self):
        """
        Tests init with set arguments.
        """
        value = 100
        for arg in ["poll_time", "poke_poll_time", "initialize_timeout"]:
            runner = BaseRunnerTest(**{arg: value})
            self.assertEqual(getattr(runner, arg), value)

    def test_is_listening_connection_error(self):
        """
        Tests is_listening() raising a ConnectionError being
        considered not listening.
        """
        runner = BaseRunnerTest()
        runner._session = runner.build_session()
        runner._session.get = MagicMock(side_effect=ConnectionError)

        self.assertFalse(runner.is_listening())

        runner._session.get.assert_called_once_with(f"{runner.url}/check")
        runner._session = None

    def test_is_listening_true(self):
        """
        Tests is_listening() when the server responds 200 to /check.
        """
        runner = BaseRunnerTest()
        runner._session = runner.build_session()
        runner._session.get = MagicMock(return_value=mock_response())

        self.assertTrue(runner.is_listening())

        runner._session.get.assert_called_once_with(f"{runner.url}/check")

    def test_initialize_start(self):
        """
        Tests that initialize_start() will start the poller.
        """
        runner = BaseRunnerTest()
        self.assertIsNone(runner._initialize_poller.start_time)
        runner.initialize_start()
        self.assertIsNotNone(runner._initialize_poller.start_time)

    def test_initialize_poll_timeout(self):
        """
        Tests that initialize_poll() will eventually raise
        an InitializeTimeout if the timeout has been exceeded.
        """
        poll_time = 0.001
        initialize_timeout = 0.005
        runner = BaseRunnerTest(
            poll_time=poll_time, initialize_timeout=initialize_timeout
        )
        runner.initialize_start()
        with self.assertRaises(InitializeTimeout) as e_cm:
            runner.initialize_poll(lambda: False)
        self.assertEqual(
            str(e_cm.exception),
            f"Initialization timed out after {runner._initialize_poller.total_time:.2f} seconds",
        )
        self.assertFalse(runner.initialized)

    def test_initialize_poll_no_start(self):
        """
        Tests that calling initialize_poll() without calling
        initialize_start() will raise an exception.
        """
        runner = BaseRunnerTest(poll_time=0.001)
        with self.assertRaises(NotImplementedError):
            runner.initialize_poll(lambda: False)

    def test_initialize_timeout(self):
        """
        Tests initialize() throwing after timing out
        at making a connection.
        """
        poll_time = 0.001
        initialize_timeout = 0.002
        runner = BaseRunnerTest(
            poll_time=poll_time, initialize_timeout=initialize_timeout
        )
        runner.is_listening = MagicMock(return_value=False)

        with self.assertRaises(InitializeTimeout):
            runner.initialize()
        runner._session = None

        self.assertFalse(runner.initialized)
        self.assertGreater(runner._initialize_poller.total_time, 0)
        self.assert_log_size(1)
        self.assert_log_message(0, "Waiting for MOOSE webserver to be listening...")

        self.assertGreater(len(runner.is_listening.mock_calls), 1)

    def test_initialize_listening_immediate(self):
        """
        Tests initialize() completing immediately when it is
        immediately listening.
        """
        runner = BaseRunnerTest(poll_time=0.001, poke_poll_time=None)
        runner.is_listening = MagicMock(return_value=True)

        with self._caplog.at_level("INFO"):
            runner.initialize()
        runner._session = None

        runner.is_listening.assert_called_once()
        self.assertTrue(runner.initialized)
        self.assert_log_size(1)
        self.assert_log_message(0, "MOOSE webserver is listening")
        self.assertGreater(runner._initialize_poller.total_time, 0)

    def test_initialize_listening_eventual(self):
        """
        Tests initialize() completing when it takes a little bit
        for the webserver to start listening
        """
        is_listening_calls = 0
        listening_after_num = 2

        def mock_is_listening(*args, **kwargs):
            nonlocal is_listening_calls
            is_listening_calls += 1
            return is_listening_calls == listening_after_num

        runner = BaseRunnerTest(poll_time=0.001, poke_poll_time=None)
        with patch_baserunner("is_listening", new=mock_is_listening):
            runner.initialize()
        runner._session = None

        self.assertTrue(runner.initialized)
        self.assertGreater(runner._initialize_poller.total_time, 0)
        self.assertEqual(is_listening_calls, listening_after_num)
        self.assert_log_size(2)
        self.assert_log_message(0, "Waiting for MOOSE webserver to be listening...")
        self.assert_log_message(1, "MOOSE webserver is listening")

    def test_initialize_start_poker(self):
        """
        Tests initialize() starting the poke thread.
        """
        runner = BaseRunnerTest()

        runner.initialize()
        runner._session = None

        self.assertTrue(runner._poker.is_alive())
        runner._poker.stop()
        runner._poker.join()

    def test_stop_poker(self):
        """
        Tests stop_poker() stopping the poke thread.
        """
        runner = BaseRunnerTest()
        runner._poker = runner._build_poker()

        poke_thread = runner._poker
        poke_thread.start()
        self.assertTrue(poke_thread.is_alive())

        runner.stop_poker()

        self.assertIsNone(runner._poker)
        self.assertFalse(poke_thread.is_alive())
        self.assert_in_log("Stopping poke poll thread")

        # Can be ran again
        self._caplog.clear()
        runner.stop_poker()
        self.assert_log_size(0)

    def test_finalize_close_session(self):
        """
        Tests finalize() closing the session.
        """
        runner = BaseRunnerTest(poke_poll_time=None)
        runner._initialized = True
        runner._session = runner.build_session()
        runner._session.close = MagicMock()
        runner.is_listening = MagicMock(return_value=False)

        with patch.object(runner._session, "close") as session_close:
            runner.finalize()

        session_close.assert_called_once()
        runner.is_listening.assert_called_once()
        self.assertIsNone(runner._session)

    def test_finalize_stop_poker(self):
        """
        Tests finalize() stopping the poke thread.
        """
        runner = BaseRunnerTest()
        runner._poker = runner._build_poker()
        poke_thread = runner._poker
        poke_thread.start()

        runner._initialized = True
        runner._session = runner.build_session()
        runner.is_listening = MagicMock(return_value=False)

        runner.finalize()

        runner.is_listening.assert_called_once()
        self.assert_in_log("Stopping poke poll thread")
        self.assertFalse(poke_thread.is_alive())
        self.assertIsNone(runner._poker)

    def test_finalize_wait_listening(self):
        """
        Tests finalize() to wait for the server to stop listening.
        """
        runner = BaseRunnerTest(poll_time=0.001)

        num_called = 0
        not_listening_after_calls = 3

        def is_listening(*_, **__):
            nonlocal num_called
            num_called += 1
            if num_called == not_listening_after_calls:
                return False
            return True

        runner._initialized = True
        runner._session = runner.build_session()
        with patch_baserunner("is_listening", new=is_listening):
            runner.finalize()

        self.assertEqual(num_called, not_listening_after_calls)
        self.assert_log_size(2)
        self.assert_log_message(
            0, "Waiting for the MOOSE webserver to stop listening..."
        )
        self.assert_log_message(1, "MOOSE webserver is no longer listening")

    def test_cleanup_ok(self):
        """
        Tests cleanup() with nothing to do.
        """
        runner = BaseRunnerTest()
        runner.cleanup()
        self.assert_log_size(0)

    def test_cleanup_close_session(self):
        """
        Tests cleanup() closing a session.
        """
        self.allow_log_warnings = True

        runner = BaseRunnerTest(poke_poll_time=None)
        runner._session = runner.build_session()
        runner.is_listening = MagicMock(return_value=False)

        with patch.object(runner._session, "close") as runner_close:
            with self._caplog.at_level("WARNING"):
                runner.cleanup()

        runner_close.assert_called_once()
        runner.is_listening.assert_called_once()
        self.assert_log_size(1)
        self.assert_in_log(
            "Request session is still active on cleanup; closing", levelname="WARNING"
        )

    def test_cleanup_poker_not_alive(self):
        """
        Tests cleanup() with a poke thread set but not alive
        """
        runner = BaseRunnerTest()

        # Start and finish the poke thread
        runner._poker = runner._build_poker()
        poke_thread = runner._poker
        poke_thread.start()
        poke_thread.stop()
        poke_thread.join()

        self._caplog.clear()
        with self._caplog.at_level("INFO"):
            runner.cleanup()
        self.assert_log_size(0)

    def test_cleanup_stop_poker(self):
        """
        Tests cleanup() with stopping the poke thread.
        """
        self.allow_log_warnings = True

        runner = BaseRunnerTest()

        # Start the poke thread with a fake poke
        runner._poker = runner._build_poker()
        poke_thread = runner._poker
        poke_thread.start()
        self.assertTrue(poke_thread.is_alive())

        with self._caplog.at_level("INFO"):
            runner.cleanup()
        self.assertFalse(poke_thread.is_alive())
        self.assert_log_size(2)
        self.assert_log_message(
            0, "Poke thread is still alive on cleanup; stopping", levelname="WARNING"
        )
        self.assert_log_message(1, "Stopping poke poll thread")

    def test_cleanup_kill(self):
        """
        Tests cleanup() with stopping webserver via kill().
        """
        self.allow_log_warnings = True

        runner = BaseRunnerTest(poke_poll_time=None)
        runner._session = runner.build_session()
        runner.is_listening = MagicMock(return_value=True)
        runner.kill = MagicMock()

        runner.cleanup()

        runner.is_listening.assert_called_once()
        runner.kill.assert_called_once()
        self.assert_log_size(2)
        self.assert_in_log(
            "MOOSE webserver is still listening on cleanup; killing",
            levelname="WARNING",
        )

    def test_post(self):
        """
        Tests post().
        """
        runner = BaseRunnerTest()
        runner._session = runner.build_session()

        url = "foo"
        data = {"foo": "bar"}
        runner._session.post = MagicMock(return_value=mock_response(data=data))

        response = runner.post(url, data)

        self.assertEqual(runner._session.post.call_args.args, (f"{FAKE_URL}/{url}",))
        self.assertEqual(runner._session.post.call_args.kwargs, {"json": data})
        runner._session = None
        self.assertEqual(response.response.status_code, 200)
        self.assertEqual(response.data, data)

    def test_get(self):
        """
        Tests get().
        """
        runner = BaseRunnerTest()
        runner._session = runner.build_session()

        url = "foo"
        runner._session.get = MagicMock(return_value=mock_response())

        response = runner.get(url)

        self.assertEqual(runner._session.get.call_args.args, (f"{FAKE_URL}/{url}",))
        runner._session = None
        self.assertEqual(response.response.status_code, 200)

    def test_build_poker(self):
        """
        Tests build_poker.
        """
        runner = BaseRunnerTest()
        poke_thread = runner._build_poker()
        self.assertEqual(poke_thread._poll_time, runner.poke_poll_time)
        self.assertIsInstance(poke_thread._session, Session)
        self.assertEqual(poke_thread._url, f"{runner.url}/poke")
        poke_thread._session.close()

    def test_kill(self):
        """
        Tests kill() sending /kill to the webserver because it is listening.
        """
        runner = BaseRunnerTest()
        runner.is_listening = MagicMock(return_value=True)
        runner.get = MagicMock()

        runner.kill()

        runner.is_listening.assert_called_once()
        runner.get.assert_called_once_with("kill")
        self.assert_log_size(1)
        self.assert_log_message(0, "Killing MOOSE webserver")

    def test_kill_raise(self):
        """
        Tests the get() call raising in kill() being ignored.
        """
        runner = BaseRunnerTest()
        runner.is_listening = MagicMock(return_value=True)
        runner.get = MagicMock(side_effect=RuntimeError)

        runner.kill()

        runner.is_listening.assert_called_once()
        runner.get.assert_called_once()

    def test_kill_not_listening(self):
        """
        Tests kill() when the server is not listening, which will do nothing.
        """
        runner = BaseRunnerTest()
        runner.is_listening = MagicMock(return_value=False)
        runner.get = MagicMock()

        runner.kill()

        runner.is_listening.assert_called_once()
        runner.get.assert_not_called()
