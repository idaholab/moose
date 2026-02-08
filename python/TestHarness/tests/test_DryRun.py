#* This file is part of the MOOSE framework
#* https://mooseframework.inl.gov
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from TestHarnessTestCase import TestHarnessTestCase

class TestHarnessTester(TestHarnessTestCase):
    def testDryRun(self):
        """
        Test that --dry-run returns a passing status
        """
        out = self.runTests('-i', 'diffs', '--dry-run').output

        self.assertRegex(out, 'test_harness\.exodiff.*?DRY RUN')
        self.assertRegex(out, 'test_harness\.csvdiff.*?DRY RUN')

        # Skipped caveat test which returns skipped instead of 'DRY RUN'
        out = self.runTests('--no-color', '-i', 'depend_skip_tests', '--dry-run').output
        self.assertRegex(out, r'tests/test_harness.always_skipped.*? \[ALWAYS SKIPPED\] SKIP')
        self.assertRegex(out, r'tests/test_harness.needs_always_skipped.*? \[SKIPPED DEPENDENCY\] SKIP')
