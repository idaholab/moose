#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html
import os
import subprocess
from TestHarnessTestCase import TestHarnessTestCase

class TestHarnessTester(TestHarnessTestCase):
    def testTimeout(self):
        """
        Test that timeout tests report TIMEOUT
        """
        with self.assertRaises(subprocess.CalledProcessError) as cm:
            self.runTests('-i', 'timeout')

        e = cm.exception
        self.assertRegex(e.output, 'test_harness\.timeout.*?TIMEOUT')

        # Verify return code is TIMEOUT related (0x1)
        self.assertIs(0x1, e.returncode)

    def testTimeoutEnv(self):
        """
        Test that timeout tests report TIMEOUT
        """
        with self.assertRaises(subprocess.CalledProcessError) as cm:
            os.environ['MOOSE_TEST_MAX_TIME'] = '2'
            self.runTests('-i', 'timeout')
            os.environ.pop('MOOSE_TEST_MAX_TIME')

        e = cm.exception
        self.assertRegex(e.output, 'test_harness\.timeout.*?TIMEOUT')

        # Verify return code is TIMEOUT related (0x1)
        self.assertIs(0x1, e.returncode)
