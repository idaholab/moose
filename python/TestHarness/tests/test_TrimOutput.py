# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

from TestHarness.tests.TestHarnessTestCase import TestHarnessTestCase

class TestHarnessTester(TestHarnessTestCase):
    def testTrimOutput(self):
        """
        Verify output exceeded buffer, and is therfore trimmed
        """
        output = self.runTests('--no-color', '-i', 'trimmed_output', '-v').output
        self.assertIn('Output trimmed', output)

    def testNoTrimOutput(self):
        """
        Verify trimming did not take place
        """
        output = self.runTests('--no-color', '-i', 'always_ok', '-v').output
        self.assertNotIn('Output trimmed', output)

    def testNoTrimmedOutputOnError(self):
        """
        Verify trimming does not take place with a failed test using
        appropriate arguments
        """
        output = self.runTests('--no-color', '-i', 'no_trim_on_error', '--no-trimmed-output-on-error', '-v', exit_code=128).output
        self.assertNotIn('Output trimmed', output)
