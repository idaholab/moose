# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

"""Test the --min-parallel TestHarness option."""

import unittest
from typing import Optional

from TestHarnessTestCase import TestHarnessTestCase


class TestMinParallel(TestHarnessTestCase):
    """Test the --min-parallel TestHarness option."""

    def test(self):
        """Test the --min-parallel TestHarness option."""

        def run_test(min_parallel: int, skip: bool, max_parallel: Optional[int]):
            tests = {
                "test": {
                    "type": "RunApp",
                    "input": "unused",
                    "should_execute": False,
                }
            }
            if max_parallel is not None:
                tests["test"]["max_parallel"] = max_parallel

            result = self.runTests(
                f"--min-parallel={min_parallel}",
                tests=tests,
                minimal_capabilities=True,
            )
            harness = result.harness
            assert harness is not None
            job = self.getJobWithName(harness, "test")
            self.assertEqual(job.getStatus(), job.skip if skip else job.finished)

        # Test has no max_parallel, can be ran with any combo
        run_test(1, False, None)
        run_test(2, False, None)
        # Test has max_parallel=1, run with --min-parallel=2 skips it
        run_test(1, False, 1)
        run_test(2, True, 1)
        # Test has max_parallel=2, can run with --min-parallel=2
        run_test(2, False, 2)


if __name__ == "__main__":
    unittest.main()
