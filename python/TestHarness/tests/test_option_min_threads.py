# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

"""Test the --min-threads TestHarness option."""

import unittest
from typing import Optional

from TestHarnessTestCase import TestHarnessTestCase


class TestMinThreads(TestHarnessTestCase):
    """Test the --min-threads TestHarness option."""

    def test(self):
        """Test the --min-threads TestHarness option."""

        def run_test(min_threads: int, skip: bool, max_threads: Optional[int]):
            tests = {
                "test": {
                    "type": "RunApp",
                    "input": "unused",
                    "should_execute": False,
                }
            }
            if max_threads is not None:
                tests["test"]["max_threads"] = max_threads

            result = self.runTests(
                f"--min-threads={min_threads}",
                tests=tests,
                minimal_capabilities=True,
            )
            harness = result.harness
            assert harness is not None
            job = self.getJobWithName(harness, "test")
            self.assertEqual(job.getStatus(), job.skip if skip else job.finished)

        # Test has no max_threads, can be ran with any combo
        run_test(1, False, None)
        run_test(2, False, None)
        # Test has max_threads=1, run with --min-threads=2 skips it
        run_test(1, False, 1)
        run_test(2, True, 1)
        # Test has max_threads=2, can run with --min-threads=2
        run_test(2, False, 2)


if __name__ == "__main__":
    unittest.main()
