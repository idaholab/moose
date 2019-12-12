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
    def testValidation(self):
        """
        Test for correct operation with validation tests
        """
        output = self.runTests('--validation', '-i', 'validation', '--no-color').decode('utf-8')
        self.assertRegexpMatches(output, 'test_harness.validation_01.*?OK')

    def testValidationTester(self):
        """
        Test for correct operation of validation tester
        """
        output = self.runTests('--validation', '-i', 'validation', '--verbose', '--no-color').decode('utf-8')
        self.assertRegexpMatches(output, 'validation_01.csv                        | 0.00 \xc2\xb1 0.01          | 0.01 \xc2\xb1 0.01')
