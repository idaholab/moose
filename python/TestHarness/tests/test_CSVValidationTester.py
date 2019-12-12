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

    def testCSVValidationTester(self):
        """
        Test for correct operation of CSV validation tester
        """

        with self.assertRaises(subprocess.CalledProcessError) as cm:
            self.runTests('-i', 'csv_validation_tester', '--no-color').decode('utf-8')

        e = cm.exception
        output = e.output.decode('utf-8')
        self.assertRegexpMatches(output, r'test_harness\.csv_validation_tester_01.*?OK')
        self.assertRegexpMatches(output, r'test_harness\.csv_validation_tester_02.*?FAILED \(DIFF\)')

    def testCSVValidationTesterVerbose(self):
        """
        Test for correct operation of CSV validation tester in verbose mode
        """

        with self.assertRaises(subprocess.CalledProcessError) as cm:
            self.runTests('-i', 'csv_validation_tester', '--verbose', '--no-color').decode('utf-8')

        e = cm.exception
        output = e.output.decode('utf-8')
        self.assertRegexpMatches(output, 'csv_validation_tester_01.csv                        | 0.00 \xb1 0.01          | 0.01 \xb1 0.01')
        self.assertRegexpMatches(output, 'csv_validation_tester_02.csv                        | 0.00 \xb1 0.01          | 0.01 \xb1 0.00')
