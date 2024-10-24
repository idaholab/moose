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
    def testDoLastDuplicate(self):
        """
        Test for invalid use of multiple do_last params
        """
        with self.assertRaises(subprocess.CalledProcessError) as cm:
            self.runTests('-i', 'cyclic_do_last')

        e = cm.exception

        self.assertRegex(e.output, r'tests/test_harness.*?FAILED \(Cyclic or Invalid Dependency Detected!\)')

    def testDoLastDepends(self):
        """
        Test for invalid use where a test depends on a 'do_last' test
        """
        with self.assertRaises(subprocess.CalledProcessError) as cm:
            self.runTests('-i', 'cyclic_do_last_depends')

        e = cm.exception

        self.assertRegex(e.output, r'tests/test_harness.*?FAILED \(Cyclic or Invalid Dependency Detected!\)')

    def testDoLast(self):
        """
        Confirm 'do_last' tested last
        """
        output = self.runTests('--no-color', '-i', 'do_last')
        self.assertRegex(output, 'tests/test_harness.a.*?OK\ntests/test_harness.do_last.*?OK')

    def testDoLastSkipped(self):
        """
        Confirm 'do_last' is skipped if a test it depends on failed/skipped.
        """
        output = self.runTests('--no-color', '-i', 'do_last_skipped')
        self.assertRegex(output, 'test_harness.do_last.*?\[SKIPPED DEPENDENCY\] SKIP')

    def testDoLastName(self):
        """
        Test for invalid use where a test name is 'ALL' when 'prereq = ALL' is set
        """
        with self.assertRaises(subprocess.CalledProcessError) as cm:
            self.runTests('--no-color', '-i', 'do_last_name')

        e = cm.exception

        self.assertRegex(e.output, 'test_harness.*?FAILED \(Test named ALL when "prereq = ALL" elsewhere in test spec file!\)')
