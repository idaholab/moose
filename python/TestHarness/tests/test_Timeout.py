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
    def testTimeout(self):
        """
        Test that timeout tests report TIMEOUT
        """
        with self.assertRaises(subprocess.CalledProcessError) as cm:
            self.runTests('-i', 'timeout')

        e = cm.exception
        self.assertRegexpMatches(e.output, 'test_harness\.timeout.*?TIMEOUT')
