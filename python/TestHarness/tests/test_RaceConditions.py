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
    def testRaceConditions(self):
        """
        Test for Race Conditions in the TestHarness
        """

        # Check for the words 'Diagnostic analysis' which indicate that race conditions exist
        with self.assertRaises(subprocess.CalledProcessError) as cm:
            self.runTests('--pedantic-checks', '-i', 'output_clobber_simple')
        e = cm.exception
        self.assertIn('Diagnostic analysis', e.output.decode('utf-8'))
