# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

import tempfile
from TestHarnessTestCase import TestHarnessTestCase


class TestHarnessTester(TestHarnessTestCase):
    def testFailedTests(self):
        """
        In order to test for failed tests, we need to run run_tests twice. Once
        to create a json file containing previous results, and again to only run
        the test which that has failed.
        """
        with tempfile.TemporaryDirectory() as output_dir:
            args = ["--no-color", "--results-file", "failed-unittest", "-o", output_dir]
            kwargs = {"tmp_output": False}

            # Has failing tests; failed test is output and so is failed test summary
            out = self.runTests(
                *args, "-i", "always_bad", exit_code=128, **kwargs
            ).output
            self.assertRegex(out, r"tests/test_harness.always_ok.*?OK")
            self.assertRegex(
                out, r"tests/test_harness.always_bad.*?FAILED \(EXIT CODE 1 != 0\)"
            )
            self.assertIn("Failed Tests:", out)

            # Re-run failed tests
            out = self.runTests(*args, "--failed-tests", exit_code=128, **kwargs).output
            # Verify the passing test is not present
            self.assertNotRegex(out, r"tests/test_harness.always_ok.*?OK")
            # Verify the caveat represents a previous result
            self.assertRegex(
                out,
                r"tests/test_harness.always_bad.*?\[PREVIOUS RESULTS: EXIT CODE 1 != 0\] FAILED \(EXIT CODE 1 != 0\)",
            )

            # Does not having failing tests; failed tests summary not printed
            out = self.runTests(*args, "-i", "always_ok").output
            self.assertNotIn("Failed Tests:", out)
