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
    def testDependencySkip(self):
        """
        Test skipping a test if its prereq is also skipped
        (this is now considered a never running test e.g: an error)
        """
        with self.assertRaises(subprocess.CalledProcessError) as cm:
            self.runTests('--no-color', '-i', 'depend_skip_tests')

        e = cm.exception
        output = e.output.decode('utf-8')

        self.assertRegex(output, r'test_harness\.always_skipped.*? \[ALWAYS SKIPPED\] SKIP')
        self.assertRegex(output, r'test_harness\.needs_always_skipped.*? \[SKIPPED DEPENDENCY\] FAILED \(prereq test parameter: skip\)')
