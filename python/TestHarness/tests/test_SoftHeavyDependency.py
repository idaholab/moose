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
    def testNotHeavy(self):
        """
        Heavy test is skipped while non-heavy tests are not
        """
        output = self.runTests('--no-color', '-i', 'heavy_on_not_heavy')
        # The following should be skipped
        self.assertRegex(output, 'test_harness\.heavy_a .*? \[HEAVY\] SKIP')
        self.assertRegex(output, 'test_harness\.heavy_b .*? \[HEAVY\] SKIP')
        self.assertRegex(output, 'test_harness\.heavy_on_not_heavy .*? \[HEAVY\] SKIP')
        self.assertRegex(output, 'test_harness\.heavy_on_not_heavy_a_and_not_heavy_b .*? \[HEAVY\] SKIP')

        # The following should not be skipped, they should finish with an OK status.
        self.assertRegex(output, 'test_harness\.singleton_a .*? OK')
        self.assertRegex(output, 'test_harness\.singleton_b .*? OK')
        self.assertRegex(output, 'test_harness\.not_heavy .*? OK')
        self.assertRegex(output, 'test_harness\.not_heavy_a .*? OK')
        self.assertRegex(output, 'test_harness\.not_heavy_b .*? OK')
        self.assertRegex(output, 'test_harness\.not_heavy_on_singleton_a_and_singleton_b .*? OK')

        # The following should run, and should not list [implict heavy] caveat.
        # (a little redundant, but I don't see a way to check for this and the OK test above, in one go)
        self.assertNotRegex(output, 'test_harness\.singleton_a .*? \[IMPLICT HEAVY\] OK')
        self.assertNotRegex(output, 'test_harness\.singleton_b .*? \[IMPLICT HEAVY\] OK')
        self.assertNotRegex(output, 'test_harness\.not_heavy .*? \[IMPLICT HEAVY\] OK')
        self.assertNotRegex(output, 'test_harness\.not_heavy_a .*? \[IMPLICT HEAVY\] OK')
        self.assertNotRegex(output, 'test_harness\.not_heavy_b .*? \[IMPLICT HEAVY\] OK')
        self.assertNotRegex(output, 'test_harness\.not_heavy_on_singleton_a_and_singleton_b .*? \[IMPLICT HEAVY\] OK')

        # Special: caveat placements are random. Only check that it is skipped.
        # [skipped dependency,HEAVY] SKIP  versus  [HEAVY,skipped dependency] SKIP
        self.assertRegex(output, 'test_harness\.heavy_on_heavy_a_and_heavy_b .*?SKIP')

    def testSoftHeavy(self):
        """
        Heavy tests run along with their non-heavy prereq tests. The non-heavy tests which do run
        in this manner should have an 'implict heavy' caveat.

        Non-heavy tests with non-heavy prereqs do not run/are not displayed.
        """
        output = self.runTests('--no-color', '-i', 'heavy_on_not_heavy', '--heavy')
        # The following should run, and mention the additional [implicit heavy] caveat.
        self.assertRegex(output, 'test_harness\.not_heavy .*? \[IMPLICIT HEAVY\] OK')
        self.assertRegex(output, 'test_harness\.not_heavy_a .*? \[IMPLICIT HEAVY\] OK')
        self.assertRegex(output, 'test_harness\.not_heavy_b .*? \[IMPLICIT HEAVY\] OK')

        # The following should not be skipped, they should finish with an OK status.
        self.assertRegex(output, 'test_harness\.heavy_a .*? OK')
        self.assertRegex(output, 'test_harness\.heavy_b .*? OK')
        self.assertRegex(output, 'test_harness\.heavy_on_not_heavy .*? OK')
        self.assertRegex(output, 'test_harness\.heavy_on_heavy_a_and_heavy_b .*? OK')
        self.assertRegex(output, 'test_harness\.heavy_on_not_heavy_a_and_not_heavy_b .*? OK')

        # The following should not be skipped, and should not list [implicit heavy] caveat.
        # (a little redundant, but I don't see a way to check for this and the OK test above, in one go)
        self.assertNotRegex(output, 'test_harness\.heavy_a .*? \[IMPLICT HEAVY\] OK')
        self.assertNotRegex(output, 'test_harness\.heavy_b .*? \[IMPLICT HEAVY\] OK')
        self.assertNotRegex(output, 'test_harness\.heavy_on_not_heavy .*? \[IMPLICT HEAVY\] OK')
        self.assertNotRegex(output, 'test_harness\.heavy_on_heavy_a_and_heavy_b .*? \[IMPLICT HEAVY\] OK')
        self.assertNotRegex(output, 'test_harness\.heavy_on_not_heavy_a_and_not_heavy_b .*? \[IMPLICT HEAVY\] OK')

        # The following should not run at all (the test is silent, and not displayed in the output)
        self.assertNotRegex(output, 'test_harness\.singleton.*?')
        self.assertNotRegex(output, 'test_harness\.not_heavy_on_singleton_a_and_singleton_b.*?')
