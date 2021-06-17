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
    def testDuplicateTestNames(self):
        """
        Test for duplicate test names
        """
        # Duplicate tests are considered a Fatal Parser Error, hence the 'with assertRaises'
        with self.assertRaises(subprocess.CalledProcessError) as cm:
            self.runTests('-i', 'duplicate_test_names', '--no-color')

        e = cm.exception

        self.assertRegex(e.output.decode('utf-8'), r'tests/test_harness.*? \[DUPLICATE TEST\] SKIP')
        self.assertRegex(e.output.decode('utf-8'), r'tests/test_harness.*?OK')
