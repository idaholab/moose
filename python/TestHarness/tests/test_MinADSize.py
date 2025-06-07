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
    def testMinADSize(self):
        """
        Test AD vector size
        """
        output = self.runTests('-i', 'ad_size', '--no-color', no_capabilities=False).output
        self.assertRegex(output, r'tests/test_harness.enough \.* OK')
        self.assertRegex(output, r'tests/test_harness\.too_few \.* \[NEEDS: AD_SIZE>=1000\] SKIP')
