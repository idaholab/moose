# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

import json
import os
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

    def testFailedTestsUpdate(self):
        """
        Tests that previously failing tests that pass with --failed-tests are
        updated in the previous results and not ran by the next --failed-tests,
        and that --failed-tests-no-update leaves the previous results unchanged.
        """
        with tempfile.TemporaryDirectory() as output_dir:
            # Each test fails while its marker file exists
            markers = {name: os.path.join(output_dir, name) for name in ["a", "b"]}
            tests = {
                name: {"type": "RunCommand", "command": f"'test ! -e {marker}'"}
                for name, marker in markers.items()
            }
            for marker in markers.values():
                open(marker, "w").close()

            results_file = os.path.join(output_dir, "results.json")
            args = ["--no-color", "--results-file", results_file]
            kwargs = {"tmp_output": False, "tests": tests}

            # Both tests fail
            stats = self.runTests(*args, exit_code=128, **kwargs).results["stats"]

            # Only a fails; without updating, the previous results are unchanged
            os.remove(markers["b"])
            with open(results_file, "r") as f:
                previous_results = f.read()
            for _ in range(2):
                out = self.runTests(
                    *args, "--failed-tests-no-update", exit_code=128, **kwargs
                ).output
                self.assertRegex(out, r"test\.a.*?FAILED")
                self.assertRegex(out, r"test\.b.*?OK")
                with open(results_file, "r") as f:
                    self.assertEqual(f.read(), previous_results)

            # Only a fails; b is updated to passing
            out = self.runTests(*args, "--failed-tests", exit_code=128, **kwargs).output
            self.assertRegex(out, r"test\.a.*?FAILED")
            self.assertRegex(out, r"test\.b.*?OK")
            with open(results_file, "r") as f:
                results = json.load(f)
            entries = results["tests"]["test"]["tests"]
            self.assertEqual(entries["a"]["status"]["status"], "FAIL")
            self.assertEqual(entries["b"]["status"]["status"], "OK")
            # Stats from the original run are kept
            self.assertEqual(results["stats"], stats)

            # Only a is ran again, and it now passes
            os.remove(markers["a"])
            out = self.runTests(*args, "--failed-tests", **kwargs).output
            self.assertRegex(out, r"test\.a.*?OK")
            self.assertNotRegex(out, r"test\.b")

            # Nothing left to run
            out = self.runTests(*args, "--failed-tests", **kwargs).output
            self.assertNotRegex(out, r"test\.[ab]")
