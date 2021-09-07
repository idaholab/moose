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
    def testJSONDiff(self):
        """
        Test for JSONDiff
        """
        with self.assertRaises(subprocess.CalledProcessError) as cm:
            self.runTests('-i', 'jsondiff')

        e = cm.exception
        self.assertRegex(e.output.decode('utf-8'), r'test_harness\.jsondiff.*?FAILED \(JSONDIFF\)')
        self.assertRegex(e.output.decode('utf-8'), r'test_harness\.jsondiff.*?Running jsondiff')

        # Verify return code is DIFF related (0x81)
        self.assertIs(0x81, e.returncode)
