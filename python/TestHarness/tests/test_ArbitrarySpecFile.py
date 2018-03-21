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
    def testArbitrarySpecFile(self):
        """
        Verify an arbitrary test will run when we use the --spec-file argument
        """
        # Test that we do not recursively find additional tests
        output = self.runTests('--spec-file', 'tests/test_harness/arbitrary_test')
        self.assertIn('tests/test_harness.always_ok', output)
        self.assertNotIn('tests/test_harness/arbitrary_directory.always_ok', output)

        # Test that we do find additional tests with recursion
        output = self.runTests('--spec-file', 'tests/test_harness', '-i', 'arbitrary_test')
        self.assertIn('tests/test_harness.always_ok', output)
        self.assertIn('tests/test_harness/arbitrary_directory.always_ok', output)

        # Test that we are not recursively finding our way backwards
        output = self.runTests('--spec-file', 'tests/test_harness/arbitrary_directory', '-i', 'arbitrary_test')
        self.assertIn('tests/test_harness/arbitrary_directory.always_ok', output)
        self.assertNotIn('tests/test_harness.always_ok', output)
