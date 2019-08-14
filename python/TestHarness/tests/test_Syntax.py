#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from TestHarnessTestCase import TestHarnessTestCase

class TestHarnessTester(TestHarnessTestCase):
    def testSyntax(self):
        """
        Test for SYNTAX PASS status in the TestHarness
        """

        # Test that the SYNTAX PASS status message properly displays
        output = self.runTests('-i', 'syntax').decode('utf-8')
        self.assertIn('SYNTAX PASS', output)

        # Test that the SYNTAX PASS status message properly displays
        output = self.runTests('--check-input', '-i', 'syntax').decode('utf-8')
        self.assertIn('SYNTAX PASS', output)

        # Check that the _non_ SYNTAX test was not run
        output = self.runTests('--check-input', '-i', 'no_syntax').decode('utf-8')
        self.assertNotIn('SYNTAX PASS', output)

        # Check that _thee_ SYNTAX test is not run
        output = self.runTests('--no-check-input', '-i', 'syntax').decode('utf-8')
        self.assertNotIn('SYNTAX PASS', output)

        # Check that it is skipped when running valgrind
        output = self.runTests('--valgrind', '-i', 'syntax').decode('utf-8')
        self.assertIn('CHECK_INPUT==TRUE', output)
        self.checkStatus(output, skipped=1)
