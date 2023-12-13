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
        # Run a skipped test
        output = self.runTests('--no-color', '-i', 'heavy_on_not_heavy')
        self.assertRegex(output.decode('utf-8'), 'test_harness\.heavy.*? \[HEAVY\] SKIP')
        self.assertRegex(output.decode('utf-8'), 'test_harness\.singleton.*?OK')
        self.assertRegex(output.decode('utf-8'), 'test_harness\.not_heavy.*?OK')

    def testSoftHeavy(self):
        """
        Heavy test runs along with a non-heavy prereq test.
        Non-heavy non-prereq tests do not run.
        """
        # Run a skipped test
        output = self.runTests('--no-color', '-i', 'heavy_on_not_heavy', '--heavy')
        self.assertRegex(output.decode('utf-8'), 'test_harness\.heavy.*?OK')
        self.assertRegex(output.decode('utf-8'), 'test_harness\.not_heavy.*? \[IMPLICIT HEAVY\] OK')
        self.assertNotRegex(output.decode('utf-8'), 'test_harness\.singleton.*?OK')
