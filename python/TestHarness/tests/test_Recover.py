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
    def testRecover(self):
        """
        Test that --recover returns two passing statuses (part1 and the OK)
        """
        output = self.runTests('-i', 'always_ok', '--recover').decode('utf-8')

        self.assertIn('PART1', output)
        self.assertIn('RECOVER', output)

        # Assert if not exactly two tests ran and passed
        self.assertIn('2 passed', output)
        self.assertIn('0 skipped', output)
        self.assertIn('0 failed', output)

    def testRecoverPart1Fail(self):
        """
        Test that --recover still checks status on Part1 tests
        """
        with self.assertRaises(subprocess.CalledProcessError) as cm:
             self.runTests('-i', 'exception_transient', '--recover').decode('utf-8')

        e = cm.exception
        output = e.output.decode('utf-8')
        self.assertRegex(output, r'test_harness.*?part1.*?FAILED \(CRASH\)')
