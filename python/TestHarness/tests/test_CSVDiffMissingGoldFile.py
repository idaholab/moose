# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

import subprocess
from TestHarnessTestCase import TestHarnessTestCase


class TestHarnessTester(TestHarnessTestCase):
    def testMissingGoldFile(self):
        """
        Verify the TestHarness will detect and report a missing gold file error
        """
        result = self.runTests("-i", "csvdiff_missing_gold_file", exit_code=128)

        out = result.output
        self.assertRegex(
            out,
            r"test_harness\.csvdiff_missing_gold_file.*?FAILED \(MISSING GOLD FILE\)",
        )

        self.checkStatus(result.harness, failed=1)
