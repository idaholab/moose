# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

from TestHarnessTestCase import TestHarnessTestCase


class TestHarnessTester(TestHarnessTestCase):
    def testInvalidOverride(self):
        """
        Verify the TestHarness will detect and report invalid override arguments
        """
        result = self.runTests("-i", "csvdiff_invalid_override", exit_code=128)

        out = result.output
        self.assertRegex(
            out,
            r"test_harness\.csvdiff_invalid_override.*?FAILED \('override_abs_zero' must have the same number of entries as 'override_columns'\)",
        )

        self.checkStatus(result.harness, failed=1)
