#* This file is part of the MOOSE framework
#* https://mooseframework.inl.gov
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from TestHarnessTestCase import TestHarnessTestCase

class TestHarnessTester(TestHarnessTestCase):
    def testDuplicateOutputs(self):
        """
        Test for duplicate output files in the same directory
        """
        out = self.runTests('-i', 'duplicate_outputs', exit_code=1).output
        self.assertIn('Tests: d, c', out)
        self.assertIn('File(s): good_out.e', out)

        # Use a different spec file, which makes use of the AnalyzeJacobian tester
        out = self.runTests('-i', 'duplicate_outputs_analyzejacobian', exit_code=1).output
        self.assertIn('Tests: b, a', out)
        self.assertIn('File(s): good.i', out)

    def testDuplicateOutputsOK(self):
        """
        Test for duplicate output files in the same directory that will _not_ overwrite eachother due to
        proper prereqs set.
        """
        out = self.runTests('-i', 'duplicate_outputs_ok').output
        out += self.runTests('-i', 'duplicate_outputs_ok', '--heavy').output

        # skip case
        self.assertNotRegex(out, 'skipped_out.e')
        # heavy case
        self.assertNotRegex(out, 'heavy_out.e')
        # all
        self.assertNotRegex(out, 'FATAL TEST HARNESS ERROR')
