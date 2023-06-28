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
    def testDiffs(self):
        """
        Test for error in input to CSVDiff
        """
        with self.assertRaises(subprocess.CalledProcessError) as cm:
            self.runTests('-i', 'csvdiffs')

        e = cm.exception
        self.assertRegex(e.output.decode('utf-8'), r'test_harness\.test_csvdiff.*?FAILED \(Override inputs not the same length\)')
        self.assertRegex(e.output.decode('utf-8'), r'test_harness\.test_badfile.*?FAILED \(MISSING GOLD FILE\)')
        self.checkStatus(e.output.decode('utf-8'), failed=2)

    def testMissingComparison(self):
        """
        Verify the TestHarness will detect and report a missing comparison file error
        """
        with self.assertRaises(subprocess.CalledProcessError) as cm:
            self.runTests('-i', 'csvdiff_missing_comparison_file')

        e = cm.exception
        self.assertRegex(e.output.decode('utf-8'), r'test_harness\.test_csvdiff_comparison_file_missing.*?FAILED \(MISSING COMPARISON FILE\)')
        self.checkStatus(e.output.decode('utf-8'), failed=1)

    def testCSVDiffScript(self):
        """
        Test features of the csvdiff.py script via the TestHarness specification test file.

        Due to the fact we can not pass csvdiff arguments via the ./run_tests script, and that these tests are creating actual
        CSV output files, the tests will compare against intentionally faulty gold files. These tests would otherwise fail if
        certain options being passed to csvdiff did not work.
        """
        self.runTests('-i', 'csvdiff_tests')
