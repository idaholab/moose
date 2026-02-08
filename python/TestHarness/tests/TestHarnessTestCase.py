# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

# ruff: noqa: E402

"""Test case and utilities for testing the TestHarness."""

import json
import os
import sys
import tempfile
import unittest
from contextlib import nullcontext, redirect_stdout
from copy import deepcopy
from dataclasses import dataclass
from io import StringIO
from typing import Callable, Optional

from mock import patch

MOOSE_DIR: str = os.path.abspath(
    os.path.join(os.path.dirname(__file__), "..", "..", "..")
)

# Add moose python to path if it doesn't exist
MOOSE_PYTHON = os.path.join(MOOSE_DIR, "python")
if MOOSE_PYTHON not in sys.path:
    sys.path.append(MOOSE_PYTHON)

import pyhit

from TestHarness import TestHarness, capability_util
from TestHarness.schedulers.Job import Job

MOOSE_EXE = "moose_test-opt"
TEST_DIR = os.path.join(MOOSE_DIR, "test")


class TestHarnessTestCase(unittest.TestCase):
    """TestCase class for running TestHarness commands."""

    # Cache for util.getAppCapabilities
    CAPABILITIES_CACHE = {}
    # Original method for util.getAppCapabilities to call during the patch
    ORIG_GET_APP_CAPABILITIES = capability_util.getAppCapabilities
    # Cache for runTestsCached()
    RUN_TESTS_CACHED_CACHE: dict[str, "RunTestsResult"] = {}

    @dataclass
    class RunTestsResult:
        """Result for an execution of runTests()."""

        # On screen output
        output: str = ""
        # JSON results
        results: Optional[dict] = None
        # TestHarness that was ran
        harness: Optional[TestHarness] = None
        # Test specification file, if using the "tests" option
        test_spec_file: Optional[str] = None
        # The temp directory the test is being ran in, if any
        tmp_dir: Optional[str] = None

    def setUp(self):
        """
        Test case setup.

        Wraps getAppCapabilities() so that we don't call the
        application with the same arugments more than once.
        """

        def get_app_capabilities_cached(exe: str):
            assert isinstance(exe, str)
            cache = TestHarnessTestCase.CAPABILITIES_CACHE
            if exe not in cache:
                result = TestHarnessTestCase.ORIG_GET_APP_CAPABILITIES(exe)
                cache[exe] = result
            return cache[exe]

        patcher = patch.object(
            capability_util, "getAppCapabilities", wraps=get_app_capabilities_cached
        )
        self.addCleanup(patcher.stop)
        self.mock_run_cmd = patcher.start()

    def runTests(
        self,
        *args,
        tmp_output: bool = True,
        minimal_capabilities: bool = False,
        capture_results: bool = True,
        exit_code: int = 0,
        run: bool = True,
        tests: Optional[dict[str, dict]] = None,
        post_run: Optional[Callable[[RunTestsResult], None]] = None,
    ) -> RunTestsResult:
        """
        Run the TestHarness.

        Positional arguments:
        --------------------
            Command line arguments to pass to the test harness.

        Optional arguments:
        ------------------
        tmp_output : bool (default = True)
            True to output separately using the -o option in a temp directory.
        minimal_capabilities : bool (default = True)
            True to run with minimal capabilities (--minimal-capabilities).
        capture_results : bool (Default = True)
            True to capture on-screen results.
        exit_code : int (default = 0)
            Exit code to check against.
        run : bool (default = True)
            True to run the tests, otherwise just init.
        tests : Optional[dict[str, dict]]
            Test spec (dict of str -> params) to run instead.

        """
        argv = ["unused"] + list(args) + ["--term-format", "njCst"]
        if minimal_capabilities:
            argv += ["--minimal-capabilities"]

        test_root = TEST_DIR

        test_spec: Optional[str] = None
        if tests is not None:
            test_spec = self.buildTestSpec(tests)

        result = self.RunTestsResult()

        context = (
            tempfile.TemporaryDirectory if (tmp_output or test_spec) else nullcontext
        )
        with context() as c:
            test_harness_build_kwargs = {}

            if tmp_output:
                argv += ["-o", c]
                result.tmp_dir = c

            # Setup test spec directory if we're building one on the fly
            if test_spec:
                test_root = str(c)

                # Spec goes within the 'test' directory
                test_dir = os.path.join(test_root, "test")
                os.mkdir(test_dir)

                test_harness_build_kwargs["skip_testroot"] = True

                # Write the test spec
                result.test_spec_file = os.path.join(test_dir, "tests")
                with open(result.test_spec_file, "w") as f:
                    f.write(test_spec)

                # Link the moose-exe so that it can be found
                moose_exe = os.path.join(TEST_DIR, MOOSE_EXE)
                os.symlink(moose_exe, os.path.join(test_root, MOOSE_EXE))

            cwd = os.getcwd()
            os.chdir(test_root)
            stdout = StringIO()
            try:
                with redirect_stdout(stdout):
                    result.harness = TestHarness.build(
                        argv, "moose_test", MOOSE_DIR, **test_harness_build_kwargs
                    )
                    if run:
                        result.harness.findAndRunTests()
            except SystemExit as e:
                if isinstance(e.code, int):
                    self.assertEqual(e.code, exit_code)
                    return result
                raise
            finally:
                os.chdir(cwd)
                result.output = stdout.getvalue()
                stdout.close()

            if result.harness.error_code != exit_code:
                print(result.output)
            self.assertEqual(result.harness.error_code, exit_code)

            if capture_results:
                with open(result.harness.options.results_file, "r") as f:
                    result.results = json.loads(f.read())

            if post_run:
                post_run(result)

        return result

    def runTestsCached(self, *args, **kwargs) -> RunTestsResult:
        """
        runTests(), but caches the result based on the arguments.

        This allows unit tests to not run the same case multple times.

        The TestHarness object isn't deep copied, but the output
        and results are so that they can be manipulated.
        """
        # Combined key for the arguments
        key = (
            "args="
            + ",".join(args)
            + "kwargs="
            + ",".join([f"{k}={v}" for k, v in kwargs.items()])
        )
        # Build the result if it doesn't exist in the cache
        if key not in self.RUN_TESTS_CACHED_CACHE:
            self.RUN_TESTS_CACHED_CACHE[key] = self.runTests(*args, **kwargs)

        # Build a new result with copied data
        result = self.RUN_TESTS_CACHED_CACHE[key]
        result_copy = {"harness": result.harness}
        for key in ["output", "results"]:
            if getattr(result, key):
                result_copy[key] = deepcopy(getattr(result, key))

        return self.RunTestsResult(**result_copy)

    @staticmethod
    def buildTestSpec(tests: dict[str, dict]) -> str:
        """
        Build a rendered test spec.

        Arguments:
        ---------
        tests : dict[str, dict]
            Test name -> test parameters.

        """
        assert isinstance(tests, dict)

        root = pyhit.Node()
        tests_section = root.insert(0, "Tests")

        for i, (name, params) in enumerate(tests.items()):
            assert isinstance(name, str)
            assert isinstance(params, dict)

            tests_section.insert(i, name, **params)

        return root.render()

    @staticmethod
    def getJobWithName(harness: TestHarness, name: str) -> Job:
        """Find the job in TestHarness results given its name."""
        job = None
        for j in harness.finished_jobs:
            if j.getTestNameShort() == name:
                assert job is None
                job = j
        assert job is not None
        return job

    def checkStatus(
        self, harness: TestHarness, passed: int = 0, skipped: int = 0, failed: int = 0
    ):
        """Check that the TestHarness status line reports the correct counts."""
        self.assertEqual(harness.num_passed, passed)
        self.assertEqual(harness.num_skipped, skipped)
        self.assertEqual(harness.num_failed, failed)
