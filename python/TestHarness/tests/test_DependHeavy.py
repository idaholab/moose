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
    def testSkipHeavy(self):
        """
        Test that the heavy test does not run, while non-heavy tests do run.
        """
        output = self.runTests('--no-color', '-i', 'depend_heavy_tests').decode('utf-8')
        self.assertRegex(output, r'test_harness\.heavy.*? \[HEAVY\] SKIP')
        self.assertRegex(output, r'test_harness\.not_heavy.*? OK')
        self.assertRegex(output, r'test_harness\.no_relation.*? OK')

    def testDoPrereqHeavy(self):
        """
        Test that the heavy test allows the non-heavy test to run, and the non-heavy test has
        necessary caveat explaining why it is running.

        Test that a non-dependency non-heavy test does not run (and is silent).
        """
        output = self.runTests('--no-color', '-i', 'depend_heavy_tests', '--heavy').decode('utf-8')
        self.assertRegex(output, r'test_harness\.heavy.*? OK')
        self.assertRegex(output, r'test_harness\.not_heavy.*? \[.*SATISFY DEPENDENCY.*\] OK')
        self.assertNotRegex(output, r'test_harness\.no_relation.*?')
