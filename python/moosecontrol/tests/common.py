#* This file is part of the MOOSE framework
#* https://mooseframework.inl.gov
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import os
import sys
from contextlib import ExitStack
from importlib.util import find_spec
from requests import Session
from unittest import TestCase
from unittest.mock import MagicMock, patch
from tempfile import TemporaryDirectory
from typing import Callable, Optional

import pytest

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

class CaptureLogTestCase(TestCase):
    """
    Base TestCase that captures logs in _caplog.
    """
    @pytest.fixture(autouse=True)
    def inject_fixtures(self, caplog):
        self._caplog: pytest.LogCaptureFixture = caplog

    def setUp(self):
        self._caplog.clear()

    def tearDown(self):
        self._caplog.clear()

    def assertLogSize(self, num: int):
        """
        Assert that there are exactly num logs.
        """
        records = self._caplog.records
        num_records = len(records)
        if num_records != num:
            raise AssertionError(
                f'Num logs {num_records} != {num}; present logs:\n'
                + '\n'.join([str(v) for v in records])
            )

    def assertLogMessage(self, i: int, message: str, levelname: str = 'INFO', name: Optional[str] = None):
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

    def assertInLog(self,
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

    def assertNoWarningLogs(self):
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

class MooseControlTestCase(CaptureLogTestCase):
    """
    Base TestCase for most runner tests.
    """
    def setUp(self):
        super().setUp()
        self.directory = TemporaryDirectory()

    def tearDown(self):
        super().setUp()
        self.directory.cleanup()

    def assertMethodsCalledInOrder(self,
                                   methods: list[str],
                                   action: Callable[[], None]):
        """
        Asserts that when calling the given action, the provided
        methods are called in the given order.
        """
        order = []
        def mark(name):
            def _inner(*_, **__): order.append(name)
            return _inner

        contexts = (patch(v, new=mark(v)) for v in methods)
        with ExitStack() as stack:
            for i, cm in enumerate(contexts):
                stack.enter_context(cm)

            action()

        self.assertEqual(order, methods)

@staticmethod
def mock_response(status_code: int = 200):
    """
    Creates a mocked Response.

    Optional Parameters
    -------------------
    status_code : int
        The status code; defaults to 200 (success).
    """
    mock_response = MagicMock()
    mock_response.__enter__.return_value = mock_response
    mock_response.status_code = status_code
    return mock_response

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
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]
"""

# Environment variable that represents the MOOSE executable
# to run if it is available.
MOOSE_EXE = os.environ.get('MOOSE_EXE')

# A fake URL to use for testing
FAKE_URL = 'http://127.0.0.1:13579'
