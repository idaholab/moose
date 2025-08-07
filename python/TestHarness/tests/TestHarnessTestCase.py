#* This file is part of the MOOSE framework
#* https://mooseframework.inl.gov
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import os
import unittest
import tempfile
import json
from io import StringIO
from contextlib import nullcontext, redirect_stdout
from dataclasses import dataclass
from typing import Optional

from TestHarness import TestHarness
import pyhit

MOOSE_DIR = os.getenv('MOOSE_DIR')
TEST_DIR = os.path.join(MOOSE_DIR, 'test')


class TestHarnessTestCase(unittest.TestCase):
    """
    TestCase class for running TestHarness commands.
    """

    @dataclass
    class RunTestsResult:
        # On screen output
        output: str = ''
        # JSON results
        results: Optional[dict] = None
        # TestHarness that was ran
        harness: Optional[TestHarness] = None

    def runTests(self, *args,
                 tmp_output: bool = True,
                 no_capabilities: bool = True,
                 capture_results: bool = True,
                 exit_code: int = 0,
                 tests: Optional[dict[str, dict]] = None) -> RunTestsResult:
        """
        Helper for running tests

        Args:
            Command line arguments to pass to the test harness
        Keyword arguments:
            tmp_output: Set to store output separately using the -o option in a temp directory
            no_capabilities: Set to run without capabilities (--no-capabilities)
            capture_results: Set to capture on-screen results
            exit_code: Exit code to check against
            tests: Test spec (dict of str -> params) to run instead
        """
        argv = ['unused'] + list(args) + ['--term-format', 'njCst']
        if no_capabilities:
            argv += ['--no-capabilities']

        test_root = TEST_DIR

        test_spec = None
        if tests:
            test_spec = self.buildTestSpec(tests)

        result = self.RunTestsResult()

        context = tempfile.TemporaryDirectory if (tmp_output or test_spec) else nullcontext
        with context() as c:
            test_harness_build_kwargs = {}

            if tmp_output:
                argv += ['-o', c]

            # Setup test spec directory if we're building one on the fly
            if test_spec:
                test_root = c

                # Spec goes within the 'test' directory
                test_dir = os.path.join(c, 'test')
                os.mkdir(test_dir)

                test_harness_build_kwargs['skip_testroot'] = True

                # Write the test spec
                test_spec_file = os.path.join(test_dir, 'tests')
                with open(test_spec_file, 'w') as f:
                    f.write(test_spec)

                # Link the moose-exe so that it can be found
                moose_exe = os.path.join(TEST_DIR, 'moose_test-opt')
                os.symlink(moose_exe, os.path.join(test_root, 'moose_test-opt'))

            cwd = os.getcwd()
            os.chdir(test_root)
            stdout = StringIO()
            try:
                with redirect_stdout(stdout):
                    result.harness = TestHarness.build(argv, 'moose_test', MOOSE_DIR, **test_harness_build_kwargs)
                    result.harness.findAndRunTests()
            except SystemExit as e:
                self.assertEqual(e.code, exit_code)
                return result
            finally:
                os.chdir(cwd)
                result.output = stdout.getvalue()
                stdout.close()

            if result.harness.error_code != exit_code:
                print(result.output)
            self.assertEqual(result.harness.error_code, exit_code)

            if capture_results:
                with open(result.harness.options.results_file, 'r') as f:
                    result.results = json.loads(f.read())

        return result

    @staticmethod
    def buildTestSpec(tests: dict[str, list[dict]]) -> str:
        """
        Helper for building a rendered test spec, given a dict of
        test name -> test parameters
        """
        assert isinstance(tests, dict)

        root = pyhit.Node()
        tests_section = root.insert(0, 'Tests')

        i = 0
        for name, params in tests.items():
            assert isinstance(name, str)
            assert isinstance(params, dict)

            test_section = tests_section.insert(i, name, **params)
            i += 1

        return root.render()

    def checkStatus(self, harness: TestHarness,
                    passed: int = 0,
                    skipped: int = 0,
                    failed: int = 0):
        """
        Make sure the TestHarness status line reports the correct counts.
        """
        self.assertEqual(harness.num_passed, passed)
        self.assertEqual(harness.num_skipped, skipped)
        self.assertEqual(harness.num_failed, failed)
