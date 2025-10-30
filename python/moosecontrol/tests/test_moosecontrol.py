# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

# ruff: noqa: E402

"""Tests moosecontrol.moosecontrol.MooseControl."""

import logging
import os
from copy import deepcopy
from getpass import getuser
from numbers import Number
from socket import getfqdn
from typing import Any, Optional, Tuple
from unittest.mock import MagicMock, PropertyMock, patch

import pytest

from moosecontrol import MooseControl, SubprocessSocketRunner
from moosecontrol.exceptions import ControlNotWaiting, NotInitialized, UnexpectedFlag
from moosecontrol.moosecontrol import (
    DEBUG_LOG_FORMAT,
    DEFAULT_LOG_FORMAT,
    STREAM_HANDLER,
    MooseControlContextManager,
)

from .common import (
    BASE_INPUT,
    FAKE_URL,
    MooseControlTestCase,
    mock_response,
)
from .test_runners_baserunner import BaseRunnerTest

MOOSECONTROL = "moosecontrol.MooseControl"
BASERUNNER = "moosecontrol.runners.BaseRunner"

FAKE_EXECUTE_ON_FLAG = "foo"
FAKE_NAME = "cool_name"


class TestMooseControl(MooseControlTestCase):
    """Tests moosecontrol.moosecontrol.MooseControl."""

    def test_init(self):
        """Test __init__() with the required and default arguments."""
        runner = BaseRunnerTest()
        with patch("moosecontrol.moosecontrol.logging.basicConfig") as basic_config:
            control = MooseControl(runner)
            basic_config.assert_called_once_with(
                level=logging.INFO,
                handlers=[STREAM_HANDLER],
                format=DEFAULT_LOG_FORMAT,
                datefmt="%H:%M:%S",
            )
        self.assertEqual(control.runner, runner)
        self.assertEqual(control.poll_time, control.runner.poll_time)
        self.assertFalse(control.initialized)

    def test_init_quiet_and_verbose(self):
        """Test __init__() with quiet and verbose set."""
        with self.assertRaisesRegex(
            ValueError, "Cannot set quiet=True and verbose=True"
        ):
            MooseControl(BaseRunnerTest(), quiet=True, verbose=True)

    def test_init_quiet(self):
        """Test __init__() with quiet=True, which does not add a logging handler."""
        with patch("moosecontrol.moosecontrol.logging.basicConfig") as basic_config:
            MooseControl(BaseRunnerTest(), quiet=True)
        basic_config.assert_not_called()

    def test_init_verbose(self):
        """Test __init__() with verbose=True, which adds a debug log handler."""
        with patch("moosecontrol.moosecontrol.logging.basicConfig") as basic_config:
            MooseControl(BaseRunnerTest(), verbose=True)
        basic_config.assert_called_once_with(
            level=logging.DEBUG,
            handlers=[STREAM_HANDLER],
            format=DEBUG_LOG_FORMAT,
            datefmt="%H:%M:%S",
        )

    def test_enter(self):
        """
        Test __enter__().

        Should call initialize()and return a MooseControlContextManager.
        """
        control = MooseControl(BaseRunnerTest())
        control.initialize = MagicMock()

        result = control.__enter__()

        self.assertIsInstance(result, MooseControlContextManager)
        self.assertEqual(result.control, control)
        control.initialize.assert_called_once()

    def test_exit(self):
        """
        Test __exit__().

        Should call finalize() and not cleanup().
        """
        control = MooseControl(BaseRunnerTest())
        control.cleanup = MagicMock()
        control.finalize = MagicMock()

        control.__exit__(None, None, None)

        self.assert_log_size(0)
        control.cleanup.assert_not_called()
        control.finalize.assert_called_once()

    def test_exit_raise(self):
        """
        Test __exit__() when an exception is raised.

        Should call cleanup() instead of finalize.
        """
        self.allow_log_warnings = True

        control = MooseControl(BaseRunnerTest())
        control.cleanup = MagicMock()
        control.finalize = MagicMock()

        control.__exit__(RuntimeError, None, None)

        self.assert_log_size(1)
        self.assert_log_message(
            0, "Encountered RuntimeError on exit; running cleanup", levelname="WARNING"
        )

        control.cleanup.assert_called_once()
        control.finalize.assert_not_called()

    def test_context_manager(self):
        """Test __enter__() and __exit__ together() on success."""
        control = MooseControl(BaseRunnerTest())
        control.initialize = MagicMock()
        control.cleanup = MagicMock()
        control.finalize = MagicMock()

        with control as cm:
            pass

        self.assertIsInstance(cm, MooseControlContextManager)
        self.assert_log_size(0)
        control.initialize.assert_called_once()
        control.finalize.assert_called_once()
        control.cleanup.assert_not_called()

    BASE_GET_INITIALIZE_DATA = {
        "name": "Python MooseControl",
        "host": getfqdn(),
        "user": getuser(),
    }

    def test_get_initialized_data(self):
        """Test get_initialize_data()."""
        control = MooseControl(BaseRunnerTest())
        data = control.get_initialize_data()
        self.assertEqual(data, self.BASE_GET_INITIALIZE_DATA)

    def test_get_initialized_data_override(self):
        """Test get_initialize_data() when a derived class adds data."""
        extra_data = {"foo": "bar"}

        class MooseControlDerived(MooseControl):
            def additional_initialize_data(self):
                return extra_data

        control = MooseControlDerived(BaseRunnerTest())
        data = control.get_initialize_data()
        gold = deepcopy(self.BASE_GET_INITIALIZE_DATA)
        gold["name"] = "Python MooseControlDerived"
        gold.update(extra_data)
        self.assertEqual(data, gold)

    def test_get_initialized_data_bad_override(self):
        """Test get_initialize_data() when a derived class overrides base data."""
        bad_key = "name"

        class MooseControlDerived(MooseControl):
            def additional_initialize_data(self):
                return {bad_key: "foo"}

        with self.assertRaisesRegex(
            KeyError,
            rf"'Cannot override entry \"{bad_key}\" in additional_initialize_data\(\)'",
        ):
            MooseControlDerived(BaseRunnerTest()).get_initialize_data()

    def test_initialize(self):
        """
        Test initialize().

        Should call the underlying runner's initialize() and wait()
        and query the data from initialize.
        """
        control = MooseControl(BaseRunnerTest())

        def set_initialize(data):
            control.runner._initialized = True

        initialized_data = {
            "control_name": "control",
            "control_type": "type",
            "execute_on_flags": ["foo", "bar"],
        }
        control.wait = MagicMock()
        control.runner.initialize = MagicMock(side_effect=set_initialize)
        with patch(
            f"{BASERUNNER}.initialized_data", new_callable=PropertyMock
        ) as mock_initialized_data:
            mock_initialized_data.return_value = initialized_data
            control.initialize()

        control.runner.initialize.assert_called_once()
        control.wait.assert_called_once()
        self.assertEqual(control.initialized_data.data, initialized_data)
        self.assertEqual(control.control_name, initialized_data["control_name"])
        self.assertEqual(control.control_type, initialized_data["control_type"])
        self.assertEqual(control.execute_on_flags, initialized_data["execute_on_flags"])
        first_log = self.assert_in_log(
            f"Initialized connection with {control.control_type}"
            f' "{control.control_name}"'
        )
        self.assert_in_log(
            "Control is listening on flags: foo, bar", after_index=first_log
        )

    def test_finalize(self):
        """
        Test finalize().

        Should call the underlying runner's finalize().
        """
        runner = BaseRunnerTest()
        control = MooseControl(runner)
        runner.finalize = MagicMock()

        control.finalize()

        runner.finalize.assert_called_once()

    def test_cleanup(self):
        """Test that cleanup() calls the underlying runner's cleanup()."""
        runner = BaseRunnerTest()
        control = MooseControl(runner)
        runner.cleanup = MagicMock()

        control.cleanup()

        runner.cleanup.assert_called_once()

    @pytest.mark.moose
    def test_live(self):
        """Test running a MOOSE input live."""
        input_file = os.path.join(self.directory.name, "input.i")
        with open(input_file, "w") as f:
            f.write(BASE_INPUT)

        command = [self.moose_exe, "-i", input_file]
        runner = SubprocessSocketRunner(
            command=command,
            moose_control_name="web_server",
            directory=self.directory.name,
        )
        with MooseControl(runner) as ctx:
            control = ctx.control
            control.set_continue()

        self.assertEqual(runner.get_return_code(), 0)


class TestMooseControlSetUpControl(MooseControlTestCase):
    """Test moosecontrol.moosecontrol.MooseControl with automatic MooseControl setup."""

    def setUp(self):
        """
        Set up test.

        Creates a MooseControl and data structures for tracking
        what calls were made to the session GET and POST and
        calls the parent setUp().
        """
        # The control for this test
        runner = BaseRunnerTest(poll_time=0.001)
        self.control: MooseControl = MooseControl(runner, quiet=True)

        # Paths in order that were called using mocked Session.get
        self.get_paths: list[str] = []
        # Path and JSON data that were called using mocked Session.post
        self.post_paths: list[Tuple[str, dict]] = []

        # Setup the session for the control (would be done in initialize())
        runner._session = runner.build_session()

        super().setUp()

    def tearDown(self):
        """
        Tear down a test.

        Closes the session and calls the parent tearDown().
        """
        # Clean up the session
        self.control.runner._session.close()

        super().tearDown()

    def mock_get(
        self,
        paths: Optional[list[Tuple[str, dict]]] = None,
        waiting: Optional[bool] = None,
    ):
        """
        Mock Session.get for the runner.

        Parameters
        ----------
        paths : Optional[list[Tuple[str, dict]]]
            List of URL -> kwargs to pass to fake_response().
        waiting : Optional[bool]
            If True, sets up a fake GET for the server to be
            waiting on FAKE_EXECUTE_ON_FLAG. If False, sets
            up a fake GET for the server to return that it
            is not waiting.

        """
        self.get_paths.clear()

        if paths is None:
            paths = []
        runner = self.control.runner
        runner._initialized = True

        if waiting is not None:
            if waiting:
                paths.append(
                    (
                        "waiting",
                        {
                            "data": {
                                "waiting": True,
                                "execute_on_flag": FAKE_EXECUTE_ON_FLAG,
                            }
                        },
                    )
                )
            else:
                paths.append(("waiting", {"data": {"waiting": False}}))

        orig = runner._session.get

        def new(path):
            self.get_paths.append(path.replace(f"{FAKE_URL}/", ""))
            for mock_path, mock_kwargs in paths:
                if path.endswith(f"/{mock_path}"):
                    return mock_response(url=path, **mock_kwargs)
            return orig(path)

        return patch.object(runner._session, "get", new=new)

    def assertGetPaths(self, paths: list[str]):
        """Assert that the given paths were passed in the mocked get."""
        self.assertEqual(self.get_paths, paths)

    def mock_post(self, paths: list[Tuple[str, dict]] = []):
        """Mock session.post for the runner."""
        self.post_paths.clear()

        paths = deepcopy(paths)
        runner = self.control.runner
        runner._initialized = True

        orig = runner._session.post

        def new(path, json):
            self.post_paths.append((path.replace(f"{FAKE_URL}/", ""), json))
            for mock_path, mock_kwargs in paths:
                if path.endswith(f"/{mock_path}"):
                    return mock_response(url=path, **mock_kwargs)
            return orig(path)

        return patch.object(runner._session, "post", new=new)

    def assertPostPaths(self, paths: list[Tuple[str, dict]]):
        """Assert that the given paths and data were passed in the mocked get."""
        self.assertEqual(self.post_paths, paths)

    def test_get_waiting_flag(self):
        """Test get_waiting_flag()."""
        with self.mock_get(waiting=False):
            self.assertIsNone(self.control.get_waiting_flag())
        self.assertGetPaths(["waiting"])

        with self.mock_get(waiting=True):
            self.assertEqual(self.control.get_waiting_flag(), FAKE_EXECUTE_ON_FLAG)
        self.assertGetPaths(["waiting"])

    def test_is_waiting(self):
        """Test is_waiting()."""
        with self.mock_get(waiting=False):
            self.assertFalse(self.control.is_waiting())
        self.assertGetPaths(["waiting"])

        with self.mock_get(waiting=True):
            self.assertTrue(self.control.is_waiting())
        self.assertGetPaths(["waiting"])

    def test_require_waiting_not_waiting(self):
        """Test require_waiting() when the control is not waiting."""
        with self.mock_get(waiting=False), self.assertRaises(ControlNotWaiting):
            self.control.require_waiting()
        self.assertGetPaths(["waiting"])

    def test_require_waiting(self):
        """Test require_waiting()."""
        with self.mock_get(waiting=True):
            self.control.require_waiting()
        self.assertGetPaths(["waiting"])

    def test_get_not_initialized(self):
        """Test get() raising when not initialized."""
        with self.assertRaisesRegex(NotInitialized, "MooseControl is not initialized"):
            self.control.get("unused")

    def test_post_not_initialized(self):
        """Test post() raising when not initialized."""
        with self.assertRaisesRegex(NotInitialized, "MooseControl is not initialized"):
            self.control.post("unused", {})

    def test_set_continue(self):
        """Test set_continue()."""
        with self.mock_get(waiting=True, paths=[("continue", {})]):
            self.control.set_continue()
        self.assertGetPaths(["waiting", "continue"])
        self.assert_log_size(1)
        self.assert_log_message(0, "Sending continue to server")

    def test_set_terminate(self):
        """Test set_terminate()."""
        with self.mock_get(waiting=True, paths=[("terminate", {})]):
            self.control.set_terminate()
        self.assertEqual(self.get_paths, ["waiting", "terminate"])
        self.assert_log_size(1)
        self.assert_log_message(0, "Sending terminate to server")

    def test_wait_immediate(self):
        """Test wait() returning immediately."""
        with self.mock_get(waiting=True):
            flag = self.control.wait()
        self.assertEqual(flag, FAKE_EXECUTE_ON_FLAG)
        self.assertGetPaths(["waiting"])
        self.assert_log_size(2)
        self.assert_log_message(0, "Waiting for the server")
        self.assert_log_message(1, f"Server is waiting at flag {FAKE_EXECUTE_ON_FLAG}")

    def test_wait_with_flag(self):
        """Test wait() with a flag."""
        with self.mock_get(waiting=True):
            flag = self.control.wait(FAKE_EXECUTE_ON_FLAG)
        self.assertEqual(flag, FAKE_EXECUTE_ON_FLAG)
        self.assertGetPaths(["waiting"])
        self.assert_log_size(2)
        self.assert_log_message(
            0, f"Waiting for the server to be at flag {FAKE_EXECUTE_ON_FLAG}"
        )
        self.assert_log_message(1, f"Server is waiting at flag {FAKE_EXECUTE_ON_FLAG}")

    def test_wait_wrong_flag(self):
        """Test wait() with a flag and being at the wrong flag."""
        with self.mock_get(waiting=True), self.assertRaises(UnexpectedFlag) as e:
            self.control.wait("abcd")
        self.assertEqual(str(e.exception), "Unexpected execute on flag foo")
        self.assertGetPaths(["waiting"])
        self.assert_log_size(2)
        self.assert_log_message(0, "Waiting for the server to be at flag abcd")
        self.assert_log_message(1, f"Server is waiting at flag {FAKE_EXECUTE_ON_FLAG}")

    def test_wait_with_poll(self):
        """Test wait() not being ready immediately and polling."""
        # Polls for a little bit
        get_count = 0
        waiting_at_num = 2

        def get(path):
            nonlocal get_count
            get_count += 1
            if get_count == waiting_at_num:
                return mock_response(
                    url=path,
                    data={"waiting": True, "execute_on_flag": FAKE_EXECUTE_ON_FLAG},
                )
            return mock_response(url=path, data={"waiting": False})

        with patch.object(self.control.runner._session, "get", new=get):
            flag = self.control.wait()
        self.assertEqual(flag, FAKE_EXECUTE_ON_FLAG)
        self.assertEqual(get_count, 2)
        self.assert_log_size(2)
        self.assert_log_message(0, "Waiting for the server")
        self.assert_log_message(1, f"Server is waiting at flag {FAKE_EXECUTE_ON_FLAG}")

    def test_get_postprocessor(self):
        """Test get_postprocessor()."""
        value = 5.0

        for value_type in [float, int]:
            self._caplog.clear()
            with self.mock_get(waiting=True):
                paths = [("get/postprocessor", {"data": {"value": value_type(value)}})]
                with self.mock_post(paths):
                    get_value = self.control.get_postprocessor(FAKE_NAME)
            self.assertIsInstance(get_value, float)
            self.assertEqual(value, get_value)
            self.assertGetPaths(["waiting"])
            self.assertPostPaths([("get/postprocessor", {"name": FAKE_NAME})])
            self.assert_log_size(1)
            self.assert_log_message(
                0, f'Getting postprocessor value for "{FAKE_NAME}"', levelname="DEBUG"
            )

    def test_get_reporter(self):
        """Test get_reporter()."""
        value = 100
        with self.mock_get(waiting=True):
            paths = [("get/reporter", {"data": {"value": value}})]
            with self.mock_post(paths):
                get_value = self.control.get_reporter(FAKE_NAME)

        self.assertEqual(value, get_value)
        self.assertGetPaths(["waiting"])
        self.assertPostPaths([("get/reporter", {"name": FAKE_NAME})])
        self.assert_log_size(1)
        self.assert_log_message(
            0, f'Getting reporter value for "{FAKE_NAME}"', levelname="DEBUG"
        )

    def test_get_time(self):
        """Test get_time()."""
        value = 5.0

        for value_type in [float, int]:
            self._caplog.clear()
            paths = [("get/time", {"data": {"time": value_type(value)}})]
            with self.mock_get(waiting=True, paths=paths):
                get_value = self.control.get_time()
            self.assertIsInstance(get_value, float)
            self.assertEqual(value, get_value)
            self.assertGetPaths(["waiting", "get/time"])
            self.assert_log_size(1)
            self.assert_log_message(0, "Getting simulation time", levelname="DEBUG")

    def test_get_dt(self):
        """Test get_dt()."""
        value = 5.0

        for value_type in [float, int]:
            self._caplog.clear()
            paths = [("get/dt", {"data": {"dt": value_type(value)}})]
            with self.mock_get(waiting=True, paths=paths):
                get_value = self.control.get_dt()
            self.assertIsInstance(get_value, float)
            self.assertEqual(value, get_value)
            self.assertGetPaths(["waiting", "get/dt"])
            self.assert_log_size(1)
            self.assert_log_message(0, "Getting simulation timestep", levelname="DEBUG")

    def test_set_controllable_scalar_bad_type(self):
        """Test set_controllable_scalar() with a bad type."""
        with self.assertRaisesRegex(
            TypeError, r"Type str is not of allowed type\(s\) Number"
        ):
            self.control.set_controllable_scalar("unused", "unused", (Number,), "foo")

    def test_set_controllable_scalar_convert_type(self):
        """Test set_controllable_scalar() converting input values."""
        value = 1

        # Converts int -> float
        with (
            self.mock_get(waiting=True),
            self.mock_post([("set/controllable", {"status_code": 201})]),
        ):
            self.control.set_controllable_scalar(
                FAKE_NAME, "Real", (Number,), value, float
            )
        post_value = self.post_paths[0][1]["value"]
        self.assertIsInstance(post_value, float)

    def run_test_set_controllable(
        self,
        value: Any,
        cpp_type: str,
        method_type: str,
        expected_value: Optional[Any] = None,
    ):
        """Run a test for the various set_controllable_XXX() routines."""
        if expected_value is None:
            expected_value = value
        with (
            self.mock_get(waiting=True),
            self.mock_post([("set/controllable", {"status_code": 201})]),
        ):
            method = getattr(self.control, f"set_{method_type}")
            method(FAKE_NAME, value)
        self.assertGetPaths(["waiting"])
        self.assertPostPaths(
            [
                (
                    "set/controllable",
                    {"name": FAKE_NAME, "value": expected_value, "type": cpp_type},
                )
            ]
        )
        self.assert_log_size(1)
        self.assert_log_message(
            0, f'Setting controllable value "{FAKE_NAME}"', levelname="DEBUG"
        )

    def test_set_bool(self):
        """Test set_bool()."""
        self.run_test_set_controllable(True, "bool", "bool")

    def test_set_int(self):
        """Test set_int()."""
        self.run_test_set_controllable(int(1), "int", "int")

    def test_set_real(self):
        """Test set_real()."""
        self.run_test_set_controllable(float(1), "Real", "real")

    def test_set_string(self):
        """Test set_string()."""
        self.run_test_set_controllable("foo", "std::string", "string")

    def test_set_controllable_vector_bad_type(self):
        """Test set_controllable_vector() with a bad type."""
        with self.assertRaisesRegex(
            TypeError, r"At index 0: type list is not of allowed type\(s\) Number"
        ):
            self.control.set_controllable_vector(
                "unused",
                "unused",
                (Number,),
                ["foo"],
            )

    def test_set_controllable_vector_convert_type(self):
        """Test set_controllable_vector() converting input values."""
        value = [1, 2, 3]
        self.assertFalse(all(isinstance(v, float) for v in value))

        # Converts list[int] -> list[float]
        with (
            self.mock_get(waiting=True),
            self.mock_post([("set/controllable", {"status_code": 201})]),
        ):
            self.control.set_controllable_vector(
                FAKE_NAME, "Real", (Number,), value, float
            )
        post_value = self.post_paths[0][1]["value"]
        self.assertTrue(all(isinstance(v, float) for v in post_value))

    def test_set_vector_int(self):
        """Test set_vector_int()."""
        self.run_test_set_controllable([1, 2, 3], "std::vector<int>", "vector_int")

    def test_set_vector_real(self):
        """Test set_vector_real()."""
        value = [1.0, 2, 3.0]
        self.assertFalse(all(isinstance(v, float) for v in value))

        self.run_test_set_controllable(value, "std::vector<Real>", "vector_real")

        # Converted all to float
        post_value = self.post_paths[0][1]["value"]
        self.assertTrue(all(isinstance(v, float) for v in post_value))

    def test_set_vector_string(self):
        """Test set_vector_string()."""
        self.run_test_set_controllable(
            ["foo", "bar"], "std::vector<std::string>", "vector_string"
        )

    def test_set_realeigenmatrix(self):
        """Test set_realeigenmatrix()."""
        value = [[1, 2], [3.0, 4.0]]
        self.run_test_set_controllable(value, "RealEigenMatrix", "realeigenmatrix")

    def test_set_realeigenmatrix_1D(self):
        """Test set_realeigenmatrix() with a 1D array."""
        value = [1]
        self.run_test_set_controllable(
            value, "RealEigenMatrix", "realeigenmatrix", [[1]]
        )

    def test_set_realeigenmatrix_non_1D_2D(self):
        """Tests set_realeigenmatrix() when the value is not 1D or 2D."""
        value = [[[1]]]
        with self.assertRaisesRegex(
            ValueError, r"Value not convertible to a 1- or 2-D array"
        ):
            self.run_test_set_controllable(
                value,
                "RealEigenMatrix",
                "realeigenmatrix",
            )
