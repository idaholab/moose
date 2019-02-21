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
    def testTrimOutput(self):
        """
        Verify output exceeded buffer, and is therfore trimmed
        """
        output = self.runTests('--no-color', '-i', 'trimmed_output', '-v')
        self.assertIn('Output trimmed', output)

    def testNoTrimOutput(self):
        """
        Verify trimming did not take place
        """
        output = self.runTests('--no-color', '-i', 'always_ok', '-v')
        self.assertNotIn('Output trimmed', output)

    def testNoTrimmedOutputOnError(self):
        """
        Verify trimming does not take place with a failed test using
        appropriate arguments
        """
        with self.assertRaises(subprocess.CalledProcessError) as cm:
            self.runTests('--no-color', '-i', 'no_trim_on_error', '--no-trimmed-output-on-error', '-v')

        e = cm.exception
        self.assertNotIn('Output trimmed', e.output)
