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

        output = cm.exception.output
        self.assertIn('Tests: d, c', output)
        self.assertIn('File(s): good_out.e', output)

        # Use a different spec file, which makes use of the AnalyzeJacobian tester
        with self.assertRaises(subprocess.CalledProcessError) as cm:
            self.runTests('-i', 'duplicate_outputs_analyzejacobian')

        output = cm.exception.output
        self.assertIn('Tests: b, a', output)
        self.assertIn('File(s): good.i', output)

    def testDuplicateOutputsOK(self):
        """
        Test for duplicate output files in the same directory that will _not_ overwrite eachother due to
        proper prereqs set.
        """
        output = self.runTests('-i', 'duplicate_outputs_ok')
        output += self.runTests('-i', 'duplicate_outputs_ok', '--heavy')

        # skip case
        self.assertNotRegex(output, 'skipped_out.e')
        # heavy case
        self.assertNotRegex(output, 'heavy_out.e')
        # all
        self.assertNotRegex(output, 'FATAL TEST HARNESS ERROR')
