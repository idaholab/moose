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
    def testMissingGold(self):
        """
        Test for Missing Gold file
        """
        out = self.runTests('-i', 'missing_gold', exit_code=128).output
        self.assertRegex(out, 'test_harness\.exodiff.*?FAILED \(MISSING GOLD FILE\)')
        self.assertRegex(out, 'test_harness\.csvdiff.*?FAILED \(MISSING GOLD FILE\)')
