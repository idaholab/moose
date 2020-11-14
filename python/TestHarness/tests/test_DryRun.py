#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import subprocess
from TestHarnessTestCase import TestHarnessTestCase

class TestHarnessTester(TestHarnessTestCase):
    def testDryRun(self):
        """
        Test that --dry-run returns a passing status
        """
        output = self.runTests('-i', 'diffs', '--dry-run')

        self.assertRegex(output.decode('utf-8'), 'test_harness\.exodiff.*?DRY RUN')
        self.assertRegex(output.decode('utf-8'), 'test_harness\.csvdiff.*?DRY RUN')

        # Skipped caveat test which returns skipped instead of 'DRY RUN'
        output = self.runTests('--no-color', '-i', 'depend_skip_tests', '--dry-run')
        self.assertRegex(output.decode('utf-8'), r'tests/test_harness.always_skipped.*? \[ALWAYS SKIPPED\] SKIP')
        self.assertRegex(output.decode('utf-8'), r'tests/test_harness.needs_always_skipped.*? \[SKIPPED DEPENDENCY\] SKIP')

        # Deleted caveat test which returns a deleted failing tests while
        # performing a dry run
        with self.assertRaises(subprocess.CalledProcessError) as cm:
            self.runTests('--no-color', '-i', 'deleted', '-e', '--dry-run')

        e = cm.exception
        self.assertRegex(e.output.decode('utf-8'), r'test_harness\.deleted.*? \[TEST DELETED TEST\] FAILED \(DELETED\)')
