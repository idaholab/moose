#* This file is part of the MOOSE framework
#* https://mooseframework.inl.gov
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
        out = self.runTests('-i', 'timeout', exit_code=1).output

        self.assertRegex(out, 'test_harness\.timeout.*?TIMEOUT')

    def testTimeoutEnv(self):
        """
        Test that timeout tests report TIMEOUT
        """
        os.environ['MOOSE_TEST_MAX_TIME'] = '2'
        out = self.runTests('-i', 'timeout', exit_code=1).output
        os.environ.pop('MOOSE_TEST_MAX_TIME')

        self.assertRegex(out, 'test_harness\.timeout.*?TIMEOUT')
