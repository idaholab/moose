# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

"""Contains common testing utilities for TestHarness."""

import json
import os
from copy import deepcopy
from typing import Optional

import pytest
from moosepytest.runtestharness import run_test_harness

# Cache for the TestHarness results
CACHED_TESTHARNESS_RESULTS: dict = {}


class TestHarnessResultCache:
    """Helper that runs and caches TestHarness JSON output."""

    def __init__(self, cache_file: str, test_dir: str, test_spec: str, write: bool):
        """
        Initialize state.

        Arguments:
        ---------
        cache_file : str
            The file path to where the cache should be stored.
        test_dir : str
            The directory that contains the tests that should be ran.
        test_spec : str
            The name of the test spec that should be ran.
        write : bool
            Whether or not to overwrite the cache.

        """
        self._result_cache_cache_file: str = cache_file
        self._result_cache_test_dir: str = test_dir
        self._result_cache_test_spec: str = test_spec
        self._result_cache_write: bool = write

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
        from TestHarness.tests.common import CACHED_TESTHARNESS_RESULTS

        cache_key = os.path.relpath(
            self._result_cache_cache_file, os.path.dirname(__file__)
        )
        if cache_key not in CACHED_TESTHARNESS_RESULTS:
            CACHED_TESTHARNESS_RESULTS[cache_key] = {}
        cache = CACHED_TESTHARNESS_RESULTS[cache_key]

        joined_args = ",".join(args)
        kv_kwargs = [f"{k}={v}" for k, v in kwargs.items()]
        joined_kwargs = ",".join(kv_kwargs)
        key = f"args={joined_args},kwargs={joined_kwargs}"

        # Need to generate or load the results file on first call
        if key not in cache:
            # If MOOSE is available, generate the gold file
            if isinstance(self.moose_exe, str):
                result = run_test_harness(
                    self.moose_exe,
                    self._result_cache_test_dir,
                    self._result_cache_test_spec,
                    harness_args=args,
                    **kwargs,
                ).results
                cache[key] = result

                if self._result_cache_write:
                    with open(self._result_cache_cache_file, "w") as f:
                        json.dump(cache, f, indent=2, sort_keys=True)

            # Otherwise, use the cached one
            else:
                with open(self._result_cache_cache_file, "r") as f:
                    cache = json.load(f)

        assert isinstance(cache, dict)
        assert key in cache
        assert isinstance(cache[key], dict)

        return deepcopy(cache[key])
