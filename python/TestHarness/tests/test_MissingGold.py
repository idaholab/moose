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
    def testMissingGold(self):
        """
        Test for Missing Gold file (Exodus Only)
        """
        with self.assertRaises(subprocess.CalledProcessError) as cm:
            self.runTests('-i', 'missing_gold')

        e = cm.exception
        self.assertRegexpMatches(e.output, 'test_harness\.exodiff.*?FAILED \(MISSING GOLD FILE\)')
