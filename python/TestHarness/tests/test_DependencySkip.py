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
    def testDependencySkip(self):
        """
        Test skipping a test if its prereq is also skipped
        """
        output = self.runTests('--no-color', '-i', 'depend_skip_tests').output
        self.assertIn('[ALWAYS SKIPPED] SKIP', output)
        self.assertIn('[SKIPPED DEPENDENCY] SKIP', output)
