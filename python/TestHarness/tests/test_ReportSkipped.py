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
        Test for --no-report (do not report skipped tests)
        """

        # Verify the skipped test _does_ appear
        output = self.runTests('--no-color', '-i', 'ignore_skipped')
        self.assertIn('[ALWAYS SKIPPED] SKIP', output)

        # Verify the skipped test does _not_ appear
        output = self.runTests('--no-color', '--no-report', '-i', 'ignore_skipped')
        self.assertNotIn('[ALWAYS SKIPPED] SKIP', output)
