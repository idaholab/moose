# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

import os
import sys
from contextlib import ExitStack
from importlib.util import find_spec
from json import dumps
from unittest import TestCase
from unittest.mock import MagicMock, patch
from tempfile import TemporaryDirectory
from typing import Callable, Optional

from requests import Response, Session

import pytest

# A fake URL to use for testing
FAKE_URL = 'http://127.0.0.1:13579'

def setup_moose_python_path():
    """
    Helper for adding the moose python path to the
    PYTHONPATH if we fail to import the moosecontrol.

    Used in each unit test to avoid having to set
    PYTHONPATH at test time.
    """
    if find_spec('moosecontrol') is None:
        this_dir = os.path.dirname(__file__)
        moose_python = os.path.join(this_dir, '..', '..')
        sys.path.append(moose_python)
        assert find_spec('moosecontrol')

class MooseControlTestCase(TestCase):
    """
    Base TestCase for MooseControl tests.
    """
    @pytest.fixture(autouse=True)
    def inject_fixtures(self, caplog, moose_exe):
        """
        Inject pytest fixtures.
        """
        # Allow unittest access to the caplog
        self._caplog: pytest.LogCaptureFixture = caplog
        # Allow unittest access to the --moose-exe arg
        self._moose_exe_arg = moose_exe

    def setUp(self):
        super().setUp()

        self._caplog.clear()
        self.directory = TemporaryDirectory()

    def tearDown(self):
        super().setUp()

        self._caplog.clear()
        self.directory.cleanup()

    def get_moose_exe(self) -> str:
        """
        Helper for finding a MOOSE executable to run.

        Will first use the --moose-exe command line option
        if it is set. Will then search for moose_test-<METHOD>
        for all of the valid methods in the relative test folder.

        Will raise an exception if one was not found.
        """
        arg = self._moose_exe_arg
        if arg is not None:
            if not os.path.exists(arg):
                raise FileNotFoundError(f'--moose-exe={arg} does not exist')
            return arg

        this_dir = os.path.dirname(__file__)
        moose_dir = os.path.join(this_dir, '..', '..', '..', 'test')
        for method in ['dbg', 'devel', 'oprof', 'opt']:
            exe = os.path.abspath(os.path.join(moose_dir, f'moose_test-{method}'))
            if os.path.exists(exe):
                return exe

        raise FileNotFoundError(
            'Failed to find a MOOSE executable; either set --moose-exe '
            'to a moose executable or skip moose tests with --no-moose')

    def assert_log_size(self, num: int):
        """
        Assert that there are exactly num logs.
        """
        records = self._caplog.records
        num_records = len(records)
        if num_records != num:
            joined = '\n'.join([str(v) for v in records])
            raise AssertionError(
                f'Num logs {num_records} != {num}; present logs:'
                f'\n{joined}'
            )

    def assert_log_message(self, i: int, message: str,levelname: str = 'INFO',
                           name: Optional[str] = None):
        """
        Assert that a log message exists in the log at the given index.

        Parameters
        ----------
        i : int
            The index in the log records.
        message : str
            The log message.

        Optional Parameters
        -------------------
        levelname : str
            The log level name. Defaults to INFO.
        name : Optional[str]
            The name in the log; if not provided, don't check.
        """
        record = self._caplog.records[i]
        if name is not None:
            self.assertEqual(record.name, name)
        self.assertEqual(record.message, message)
        self.assertEqual(record.levelname, levelname)

    def assert_in_log(self,
                    message: str,
                    name: Optional[str] = None,
                    levelname: str = 'INFO',
                    after_index: Optional[int] = None) -> int:
        """
        Assert that a log message exists in the log.

        Parameters
        ----------
        message : str
            The log message.

        Optional Parameters
        -------------------
        name : Optional[str]
            The name in the log; if not provided, don't check.
        levelname : str
            The log level name. Defaults to INFO.
        after_index : Optional[int]
            If set, assert that this log exists after this index.

        Returns
        -------
        int
            The index at which the message was found in the records.
        """
        records = self._caplog.records
        if after_index is not None:
            self.assertGreater(len(records), after_index + 1)
            records = records[after_index+1:]
        for i, record in enumerate(records):
            if record.message == message:
                if name is not None:
                    self.assertEqual(record.name, name)
                self.assertEqual(record.levelname, levelname)
                return i
        raise AssertionError(f'Assertion not found in {records}')

    def assert_no_warning_logs(self):
        """
        Asserts that no logs of level WARNING or higher
        were found in the log.
        """
        for record in self._caplog.records:
            if record.levelname == 'WARNING':
                raise AssertionError(
                    f'Warning found in logs:\n'
                    f'Name: {record.name}\n'
                    f'Message: "{record.message}"'
                )

    def assert_methods_called_in_order(self,
                                   methods: list[str],
                                   action: Callable[[], None]):
        """
        Asserts that when calling the given action, the provided
        methods are called in the given order.
        """
        order = []
        def mark(name):
            def _inner(*_, **__):
                order.append(name)
            return _inner

        contexts = (patch(v, new=mark(v)) for v in methods)
        with ExitStack() as stack:
            for cm in contexts:
                stack.enter_context(cm)
            action()

        self.assertEqual(order, methods)

def set_fake_response(response: Response | MagicMock,
                      status_code: int = 200,
                      data: Optional[dict] = None,
                      url: str = FAKE_URL):
    """
    Internal helper for setting the state of a Response for testing.

    Parameters
    ----------
    response : Response | MagicMock
        The Response, or a mocked one.

    Additional Parameters
    ---------------------
    status_code : int
        The status code to return; default is 200.
    data : Optional[dict]
        Data to add to the response; defaults to no data.
    url : str
        The URL to associate with the response; defaults to FAKE_URL.
    """
    response.status_code = status_code
    response.url = url
    if data is not None:
        response.headers = {'content-type': 'application/json'}
        if isinstance(response, Response):
            response._content = dumps(data).encode('utf-8')
        else:
            response.json.return_value = data

@staticmethod
def mock_response(**kwargs) -> MagicMock:
    """
    Creates a mocked Response.

    Additional Parameters
    ---------------------
    See set_fake_response().
    """
    response = MagicMock()
    response.__enter__.return_value = response
    set_fake_response(response, **kwargs)
    return response

@staticmethod
def fake_response(**kwargs) -> Response:
    """
    Creates a faked Response.

    Additional Parameters
    ---------------------
    See set_fake_response().
    """
    response = Response()
    set_fake_response(response, **kwargs)
    return response

class FakeSession(Session):
    """
    A fake Session that doesn't init or need to be
    cleaned up.

    Will return code 200 on all GET requests.
    """
    def __init__(self, *_, **__):
        pass

    def close(self):
        pass

    def get(self, *_, **__):
        return mock_response()

# Base input file for testing the web server
BASE_INPUT = """
[Mesh/gmg]
  type = GeneratedMeshGenerator
  dim = 1
[]

[Controls/web_server]
  type = WebServerControl
  execute_on = INITIAL
  client_timeout = 1
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]
"""

# Keyword arguments to pass to the BaseRunner for live tests
LIVE_BASERUNNER_KWARGS = {
    'initialize_timeout': 1,
    'poll_time': 0.01,
    'poke_poll_time': 0.01
}
