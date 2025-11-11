# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

"""Contains common testing utilities for TestHarness.resultsstore."""

import json
import os
from copy import deepcopy
from io import StringIO
from typing import Optional
from unittest import TestCase

import pytest
from mock import patch
from moosepytest.runtestharness import run_test_harness
from TestHarness.resultsstore.utils import TestName

# Cache for the TestHarness results
TESTHARNESS_RESULTS: dict = {}
# Path to the stored results file for when moose is not available
TESTHARNESS_RESULTS_FILE = os.path.join(
    os.path.dirname(__file__), "content", "testharness_results.json"
)
# Whether or not to overwrite the above file
TESTHARNESS_RESULTS_WRITE = False

# Expected test names in the results for testing
TEST_NAMES = [
    TestName("basic", "ok"),
    TestName("basic", "fail"),
    TestName("basic", "skip"),
    TestName("validation", "test"),
]
# Expected folder names in the results for testing
FOLDER_NAMES = list(set(v.folder for v in TEST_NAMES))
# Expected number of tests in the results for testing
NUM_TESTS = len(TEST_NAMES)
# Expected number of folders in the results for testing
NUM_TEST_FOLDERS = len(FOLDER_NAMES)


class ResultsStoreTestCase(TestCase):
    """Common test case for testing TestHarness.resultsstore."""

    def setUp(self):
        """Add a patcher for mocking stdout."""
        # Mock stdout so we can more easily get print statements
        self._stdout_patcher = patch("sys.stdout", new=StringIO())
        self.stdout_mock: StringIO = self._stdout_patcher.start()

    def tearDown(self):
        """Stop the stdout patcher."""
        self._stdout_patcher.stop()

    @pytest.fixture(autouse=True)
    def inject_fixtures(self, moose_exe):
        """Inject pytest fixtures."""
        # Get the found moose executable during init, if any
        self.moose_exe: Optional[str] = moose_exe

    def get_testharness_result(self, *args: str, **kwargs) -> dict:
        """
        Get a TestHarness JSON result for testing.

        Uses caching to only call the test harness once for each set
        of arguments.

        Parameters
        ----------
        *args : str
            Arguments to pass to the TestHarness execution.
        **kwargs :
            Keyword arguments to pass to run_test_harness().

        Return
        ------
        dict :
            The loaded JSON results from the TestHarness execution.

        """
        from TestHarness.tests.resultsstore.common import TESTHARNESS_RESULTS

        key = f"args={','.join(args)},"
        key += f"kwargs={','.join([f"{k}={v}" for k, v in kwargs.items()])}"

        # Need to generate or load the results file on first call
        if key not in TESTHARNESS_RESULTS:
            # If MOOSE is available, generate the gold file
            if isinstance(self.moose_exe, str):
                test_dir = os.path.join(
                    os.path.dirname(__file__), "content", "testharness_results"
                )
                result = run_test_harness(
                    self.moose_exe,
                    test_dir,
                    "resultsstore",
                    harness_args=args,
                    **kwargs,
                ).results
                TESTHARNESS_RESULTS[key] = result

                if TESTHARNESS_RESULTS_WRITE:
                    with open(TESTHARNESS_RESULTS_FILE, "w") as f:
                        json.dump(TESTHARNESS_RESULTS, f, indent=2)

            # Otherwise, use the cached one
            else:
                with open(TESTHARNESS_RESULTS_FILE, "r") as f:
                    TESTHARNESS_RESULTS = json.load(f)

        assert isinstance(TESTHARNESS_RESULTS, dict)
        assert key in TESTHARNESS_RESULTS
        assert isinstance(TESTHARNESS_RESULTS[key], dict)

        return deepcopy(TESTHARNESS_RESULTS[key])
