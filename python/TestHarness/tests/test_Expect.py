# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

from TestHarness.tests.TestHarnessTestCase import TestHarnessTestCase


class TestHarnessTester(TestHarnessTestCase):
    def testExpect(self):
        """
        Test that Expect Err/Out tests report if the message they are supposed to look for is missing
        """
        out = self.runTests("-i", "expect", exit_code=128).output
        self.assertRegex(
            out,
            r"test_harness\.no_expect_err_pattern.*?FAILED \(EXPECTED ERROR MISSING\)",
        )
        self.assertRegex(
            out,
            r"test_harness\.no_expect_out_pattern.*?FAILED \(EXPECTED OUTPUT MISSING\)",
        )
        self.assertRegex(
            out, r"test_harness\.absent_out_pattern.*?FAILED \(OUTPUT NOT ABSENT\)"
        )
        self.assertRegex(
            out,
            r"test_harness\.no_expect_err_literal.*?FAILED \(EXPECTED ERROR MISSING\)",
        )
        self.assertRegex(
            out,
            r"test_harness\.no_expect_out_literal.*?FAILED \(EXPECTED OUTPUT MISSING\)",
        )
        self.assertRegex(
            out, r"test_harness\.absent_out_literal.*?FAILED \(OUTPUT NOT ABSENT\)"
        )
        self.assertRegex(
            out, r"test_harness\.no_expect_exit_code.*?FAILED \(WRONG EXIT CODE\)"
        )

    def testExpectMissing(self):
        """
        Test that Expect Err/Out tests report an error if both expect_err and expect_assert are missing.
        """
        out = self.runTests("-i", "expect_missing_params", exit_code=128).output
        self.assertRegex(
            out, r'Either "expect_err" or "expect_assert" must be supplied'
        )
