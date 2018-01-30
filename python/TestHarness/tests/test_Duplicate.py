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
    def testDuplicateOutputs(self):
        """
        Test for duplicate output files in the same directory
        """
        with self.assertRaises(subprocess.CalledProcessError) as cm:
            self.runTests('-i', 'duplicate_outputs')

        e = cm.exception

        self.assertRegexpMatches(e.output, r'tests/test_harness.*?FAILED \(OUTFILE RACE CONDITION\)')

    def testDuplicateOutputsOK(self):
        """
        Test for duplicate output files in the same directory
        """
        output = self.runTests('-i', 'duplicate_outputs_ok')
        output += self.runTests('-i', 'duplicate_outputs_ok', '--heavy')

        # skip case
        self.assertNotRegexpMatches(output, 'skipped_out.e')
        # heavy case
        self.assertNotRegexpMatches(output, 'heavy_out.e')
        # all
        self.assertNotRegexpMatches(output, 'FATAL TEST HARNESS ERROR')
