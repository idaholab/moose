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
    def testCyclic(self):
        """
        Test cyclic dependency error.
        """
        with self.assertRaises(subprocess.CalledProcessError) as cm:
            self.runTests('--no-color', '-i', 'cyclic_tests')

        e = cm.exception
        self.assertRegex(e.output, r'tests/test_harness.testC.*? FAILED \(Cyclic or Invalid Dependency Detected!\)')
        self.assertRegex(e.output, r'tests/test_harness.test[A|B].*? \[SKIPPED DEPENDENCY\] SKIP')
