# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

"""Test moosecontrol.runners.baserunner.BaseRunner."""

from unittest.mock import MagicMock, patch

from common import (
    FAKE_URL,
    FakeSession,
    MooseControlTestCase,
    fake_response,
    mock_response,
    setup_moose_python_path,
)
from moosecontrol.exceptions import InitializeTimeout
from moosecontrol.runners import BaseRunner
from moosecontrol.runners.baserunner import (
    DEFAULT_INITIALIZE_TIMEOUT,
    DEFAULT_POKE_POLL_TIME,
    DEFAULT_POLL_TIME,
)
from requests import Session
from requests.exceptions import ConnectionError

setup_moose_python_path()

BASERUNNER = "moosecontrol.runners.BaseRunner"


class BaseRunnerTest(BaseRunner):
    """Specialized BaseRunner for testing."""

    @property
    def url(self):
        """Get the faked URL."""
        return FAKE_URL

    @staticmethod
    def build_session():
        """Get the faked Session."""
        return FakeSession()


def patch_baserunner(name: str, **kwargs):
    """Patch a method in the BaseRunner."""
    return patch(f"moosecontrol.runners.BaseRunner.{name}", **kwargs)


def check_baserunner_cleanup_live(
    test: MooseControlTestCase,
    runner: BaseRunner,
    process_output: str,
    process_returncode: int,
):
    """
    Check state when testing cleanup().

    Used in derived runner tests.
    """
    # Should have killed the things
    poke_alive = test.assert_in_log(
        "Poke thread is still alive on cleanup; stopping", levelname="WARNING"
    )
    stop_poke = test.assert_in_log(
        "Stopping poke poll thread", after_index=poke_alive, levelname="DEBUG"
    )
    webserver_alive = test.assert_in_log(
        "MOOSE webserver is still listening on cleanup; killing",
        levelname="WARNING",
        after_index=stop_poke,
    )
    test.assert_in_log("Killing MOOSE webserver", after_index=webserver_alive)

    test.assertIsInstance(process_returncode, int)
    test.assertNotEqual(process_returncode, 0)
    test.assertIn("Client sent kill command; exiting", process_output)


class TestBaseRunner(MooseControlTestCase):
    """Test moosecontrol.runners.baserunner.BaseRunner."""

    def test_init(self):
        """Test init with the default arguments."""
        runner = BaseRunnerTest()
        self.assertEqual(runner.url, FAKE_URL)
        self.assertEqual(runner.poll_time, DEFAULT_POLL_TIME)
        self.assertEqual(runner.poke_poll_time, DEFAULT_POKE_POLL_TIME)
        self.assertEqual(runner.initialize_timeout, DEFAULT_INITIALIZE_TIMEOUT)

    def test_init_args(self):
        """Test init with set arguments."""
        value = 100
        for arg in ["poll_time", "poke_poll_time", "initialize_timeout"]:
            runner = BaseRunnerTest(**{arg: value})
            self.assertEqual(getattr(runner, arg), value)

    def test_is_listening_connection_error(self):
        """Test is_listening() with a ConnectionError returning False."""
        runner = BaseRunnerTest()
        runner._session = runner.build_session()
        runner._session.get = MagicMock(side_effect=ConnectionError)

        self.assertFalse(runner.is_listening())

        runner._session.get.assert_called_once_with(f"{runner.url}/check")
        runner._session = None

    def test_is_listening_true(self):
        """Test is_listening() when the server responds 200 to /check."""
        runner = BaseRunnerTest()
        runner._session = runner.build_session()
        runner._session.get = MagicMock(return_value=mock_response())

        self.assertTrue(runner.is_listening())

        runner._session.get.assert_called_once_with(f"{runner.url}/check")

    def test_initialize_start(self):
        """Test that initialize_start() will start the poller."""
        runner = BaseRunnerTest()
        self.assertIsNone(runner._initialize_poller.start_time)
        runner.initialize_start()
        self.assertIsNotNone(runner._initialize_poller.start_time)

    def test_initialize_poll_timeout(self):
        """Test initialize_poll() raising InitializeTimeout."""
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
            f"Initialization timed out after "
            f"{runner._initialize_poller.total_time:.2f} seconds",
        )
        self.assertFalse(runner.initialized)

    def test_initialize_poll_no_start(self):
        """Test initialize_poll() without calling initialize_start() raises."""
        runner = BaseRunnerTest(poll_time=0.001)
        with self.assertRaises(NotImplementedError):
            runner.initialize_poll(lambda: False)

    def test_initialize_timeout(self):
        """Tests initialize() raising InitializeTimeout."""
        poll_time = 0.001
        initialize_timeout = 0.002
        runner = BaseRunnerTest(
            poll_time=poll_time, initialize_timeout=initialize_timeout
        )
        runner.is_listening = MagicMock(return_value=False)

        with self.assertRaises(InitializeTimeout):
            runner.initialize({})
        runner._session = None

        self.assertFalse(runner.initialized)
        self.assertGreater(runner._initialize_poller.total_time, 0)
        self.assert_log_size(1)
        self.assert_log_message(0, "Waiting for MOOSE webserver to be listening...")

        self.assertGreater(len(runner.is_listening.mock_calls), 1)

    def test_initialize_listening_immediate(self):
        """Test initialize() when the server is immediately listening."""
        runner = BaseRunnerTest(poll_time=0.001, poke_poll_time=None)

        runner.is_listening = MagicMock(return_value=True)
        initialize_data = {"name": "foo"}
        initialized_data = {"foo": "bar"}
        with patch("common.FakeSession.post") as mock_post:
            mock_post.return_value = fake_response(data=initialized_data)
            runner.initialize(initialize_data)
        runner._session = None

        runner.is_listening.assert_called_once()
        mock_post.assert_called_once_with(
            f"{FAKE_URL}/initialize", json=initialize_data
        )
        self.assertTrue(runner.initialized)
        self.assertEqual(runner.initialized_data, initialized_data)
        self.assert_log_size(2)
        self.assert_log_message(0, "MOOSE webserver is listening")
        self.assert_log_message(
            1,
            f"Sending initialize to webserver, data={initialize_data}",
            levelname="DEBUG",
        )
        self.assertGreater(runner._initialize_poller.total_time, 0)

    def test_initialize_listening_eventual(self):
        """Test initialize() when the server is not immediately listening."""
        is_listening_calls = 0
        listening_after_num = 2

        def mock_is_listening(*args, **kwargs):
            nonlocal is_listening_calls
            is_listening_calls += 1
            return is_listening_calls == listening_after_num

        runner = BaseRunnerTest(poll_time=0.001, poke_poll_time=None)
        initialize_data = {"name": "foo"}
        initialized_data = {"foo": "bar"}
        with (
            patch_baserunner("is_listening", new=mock_is_listening),
            patch("common.FakeSession.post") as mock_post,
        ):
            mock_post.return_value = fake_response(data=initialized_data)
            runner.initialize(initialize_data)
        runner._session = None

        mock_post.assert_called_once_with(
            f"{FAKE_URL}/initialize", json=initialize_data
        )
        self.assertTrue(runner.initialized)
        self.assertEqual(runner.initialized_data, initialized_data)
        self.assertGreater(runner._initialize_poller.total_time, 0)
        self.assertEqual(is_listening_calls, listening_after_num)
        self.assert_log_size(3)
        self.assert_log_message(0, "Waiting for MOOSE webserver to be listening...")
        self.assert_log_message(1, "MOOSE webserver is listening")
        self.assert_log_message(
            2,
            f"Sending initialize to webserver, data={initialize_data}",
            levelname="DEBUG",
        )

    def test_initialize_start_poker(self):
        """Test initialize() starting the poke thread."""
        runner = BaseRunnerTest()

        with patch(f"{BASERUNNER}.post_initialize", return_value={"foo": "bar"}):
            runner.initialize({})
        runner._session = None

        self.assertTrue(runner._poker.is_alive())
        runner._poker.stop()
        runner._poker.join()

    def test_stop_poker(self):
        """Test stop_poker() stopping the poke thread."""
        runner = BaseRunnerTest()
        runner._poker = runner._build_poker()

        poke_thread = runner._poker
        poke_thread.start()
        self.assertTrue(poke_thread.is_alive())

        runner.stop_poker()

        self.assertIsNone(runner._poker)
        self.assertFalse(poke_thread.is_alive())
        self.assert_in_log("Stopping poke poll thread", levelname="DEBUG")

        # Can be ran again
        self._caplog.clear()
        runner.stop_poker()
        self.assert_log_size(0)

    def test_finalize_close_session(self):
        """Test finalize() closing the session."""
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
        """Test finalize() stopping the poke thread."""
        runner = BaseRunnerTest()
        runner._poker = runner._build_poker()
        poke_thread = runner._poker
        poke_thread.start()

        runner._initialized = True
        runner._session = runner.build_session()
        runner.is_listening = MagicMock(return_value=False)

        runner.finalize()

        runner.is_listening.assert_called_once()
        self.assert_in_log("Stopping poke poll thread", levelname="DEBUG")
        self.assertFalse(poke_thread.is_alive())
        self.assertIsNone(runner._poker)

    def test_finalize_wait_listening(self):
        """Test finalize() waiting for the server to stop listening."""
        runner = BaseRunnerTest(poll_time=0.001)

        num_called = 0
        not_listening_after_calls = 3

        def is_listening(*_, **__):
            nonlocal num_called
            num_called += 1
            return num_called != not_listening_after_calls

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
        """Test cleanup() with nothing to do."""
        runner = BaseRunnerTest()
        runner.cleanup()
        self.assert_log_size(0)

    def test_cleanup_close_session(self):
        """Test cleanup() closing a session."""
        self.allow_log_warnings = True

        runner = BaseRunnerTest(poke_poll_time=None)
        runner._session = runner.build_session()
        runner.is_listening = MagicMock(return_value=False)

        with (
            patch.object(runner._session, "close") as runner_close,
            self._caplog.at_level("WARNING"),
        ):
            runner.cleanup()

        runner_close.assert_called_once()
        runner.is_listening.assert_called_once()
        self.assert_log_size(1)
        self.assert_in_log(
            "Request session is still active on cleanup; closing", levelname="WARNING"
        )

    def test_cleanup_poker_not_alive(self):
        """Test cleanup() with a poke thread set but not alive."""
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
        """Test cleanup() stopping the poke thread."""
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
        self.assert_log_size(1)
        self.assert_log_message(
            0, "Poke thread is still alive on cleanup; stopping", levelname="WARNING"
        )

    def test_cleanup_kill(self):
        """Test cleanup() stopping webserver via kill()."""
        self.allow_log_warnings = True

        runner = BaseRunnerTest(poke_poll_time=None, poll_time=0.001)
        runner._session = runner.build_session()
        runner.kill = MagicMock()
        runner.is_listening = MagicMock(return_value=True)

        runner.cleanup()

        runner.is_listening.assert_called_once()
        runner.kill.assert_called_once()
        self.assert_log_size(2)
        self.assert_in_log(
            "MOOSE webserver is still listening on cleanup; killing",
            levelname="WARNING",
        )

    def test_post(self):
        """Test post()."""
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
        """Test get()."""
        runner = BaseRunnerTest()
        runner._session = runner.build_session()

        url = "foo"
        runner._session.get = MagicMock(return_value=mock_response())

        response = runner.get(url)

        self.assertEqual(runner._session.get.call_args.args, (f"{FAKE_URL}/{url}",))
        runner._session = None
        self.assertEqual(response.response.status_code, 200)

    def test_build_poker(self):
        """Test build_poker()."""
        runner = BaseRunnerTest()
        poke_thread = runner._build_poker()
        self.assertEqual(poke_thread._poll_time, runner.poke_poll_time)
        self.assertIsInstance(poke_thread._session, Session)
        self.assertEqual(poke_thread._url, f"{runner.url}/poke")
        poke_thread._session.close()

    def test_kill_listening(self):
        """Test kill() sending /kill to the webserver because it is listening."""
        runner = BaseRunnerTest(poll_time=0.001)
        runner.get = MagicMock()

        is_listening_calls = 0
        not_listening_after_num = 3

        def mock_is_listening(*args, **kwargs):
            nonlocal is_listening_calls
            is_listening_calls += 1
            return is_listening_calls != not_listening_after_num

        with patch_baserunner("is_listening", new=mock_is_listening):
            runner.kill()

        self.assertEqual(not_listening_after_num, is_listening_calls)
        runner.get.assert_called_once_with("kill")
        self.assert_log_size(2)
        self.assert_log_message(0, "Killing MOOSE webserver")
        self.assert_log_message(1, "Webserver is no longer listening after kill")

    def test_kill_raise(self):
        """Test the get() call raising in kill() being ignored."""
        runner = BaseRunnerTest(poll_time=0.001)
        runner.get = MagicMock(side_effect=Exception)

        is_listening_calls = 0
        not_listening_after_num = 2

        def mock_is_listening(*args, **kwargs):
            nonlocal is_listening_calls
            is_listening_calls += 1
            return is_listening_calls != not_listening_after_num

        with patch_baserunner("is_listening", new=mock_is_listening):
            runner.kill()

        self.assertEqual(not_listening_after_num, is_listening_calls)
        runner.get.assert_called_once()

    def test_kill_not_listening(self):
        """Test kill() when the server is not listening, which will do nothing."""
        runner = BaseRunnerTest()
        runner.is_listening = MagicMock(return_value=False)
        runner.get = MagicMock()

        runner.kill()

        runner.is_listening.assert_called_once()
        runner.get.assert_not_called()
