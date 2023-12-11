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
    def testSkipHeavy(self):
        """
        TestHarness will detect invalid dependencies, and produce an error, and exit with non-zero.
        """
        with self.assertRaises(subprocess.CalledProcessError) as cm:
            self.runTests('--no-color', '-i', 'depend_heavy_tests')
        e = cm.exception
        output = e.output.decode('utf-8')

        self.assertRegex(output, r'test_harness\.heavy .*? \[HEAVY\] SKIP')
        self.assertRegex(output, r'test_harness\.dbg .*? \[METHOD\!\=DBG\] SKIP')
        self.assertRegex(output, r'test_harness\.heavy_on_not_heavy .*? \[HEAVY\] SKIP')
        self.assertRegex(output, r'test_harness\.not_heavy_on_heavy.*? \[SKIPPED DEPENDENCY\] FAILED \(prereq test parameter: heavy\)')
        self.assertRegex(output, r'test_harness\.opt_on_dbg.*? \[SKIPPED DEPENDENCY\] FAILED \(prereq test parameter: method\)')
        self.assertRegex(output, r'test_harness\.not_heavy.*? OK')
        self.assertRegex(output, r'test_harness\.singleton.*? OK')

    def testDoPrereqHeavy(self):
        """
        Test that the heavy test allows the non-heavy test to run, and the non-heavy test has
        necessary caveat explaining why it is running.

        Test that a non-dependency non-heavy test does not run (and is silent).
        """
        output = self.runTests('--no-color', '-i', 'depend_heavy_tests', '--heavy').decode('utf-8')
        self.assertRegex(output, r'test_harness\.heavy.*? OK')
        self.assertRegex(output, r'test_harness\.not_heavy.*? \[IMPLICIT HEAVY\] OK')
        # Silient non-heavy tests should not run
        self.assertNotRegex(output, r'test_harness\.singleton.*?')

    def testDoPrereqHeavy(self):
        """
        Test that the heavy test allows the non-heavy test to run, and the non-heavy test has
        necessary caveat explaining why it is running.

        Test that a non-dependency non-heavy test does not run (and is silent).
        """
        output = self.runTests('--no-color', '-i', 'depend_heavy_tests', '--heavy').decode('utf-8')
        self.assertRegex(output, r'test_harness\.heavy.*? OK')
        self.assertRegex(output, r'test_harness\.not_heavy.*? \[IMPLICIT HEAVY\] OK')
        # Silient non-heavy tests should not run
        self.assertNotRegex(output, r'test_harness\.singleton.*?')
