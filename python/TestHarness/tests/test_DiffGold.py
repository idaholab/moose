#* This file is part of the MOOSE framework
#* https://mooseframework.inl.gov
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import subprocess
from TestHarnessTestCase import TestHarnessTestCase

class TestHarnessTester(TestHarnessTestCase):
    def testDiffs(self):
        """
        Test for Exodiffs, CSVDiffs
        """
        with self.assertRaises(subprocess.CalledProcessError) as cm:
            self.runTests('-i', 'diff_golds')

        e = cm.exception
        self.assertRegex(e.output, r'test_harness\.exodiff.*?FAILED \(EXODIFF\)')
        self.assertRegex(e.output, r'test_harness\.csvdiff.*?FAILED \(CSVDIFF\)')
        self.assertRegex(e.output, r'test_harness\.exodiff.*?Running exodiff')
        self.assertRegex(e.output, r'test_harness\.csvdiff.*?Running csvdiff')
