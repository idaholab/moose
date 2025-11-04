#* This file is part of the MOOSE framework
#* https://mooseframework.inl.gov
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from TestHarnessTestCase import TestHarnessTestCase

class TestHarnessTester(TestHarnessTestCase):
    def testDoLastDuplicate(self):
        """
        Test for invalid use of multiple do_last params
        """
        out = self.runTests('-i', 'cyclic_do_last', exit_code=132).output
        self.assertRegex(out, r'tests/test_harness.*?FAILED \(Cyclic or Invalid Dependency Detected!\)')

    def testDoLastDepends(self):
        """
        Test for invalid use where a test depends on a 'do_last' test
        """
        out = self.runTests('-i', 'cyclic_do_last_depends', exit_code=132).output
        self.assertRegex(out, r'tests/test_harness.*?FAILED \(Cyclic or Invalid Dependency Detected!\)')

    def testDoLast(self):
        """
        Confirm 'do_last' tested last
        """
        out = self.runTests('--no-color', '-i', 'do_last').output
        self.assertRegex(out, 'tests/test_harness.a.*?OK\ntests/test_harness.do_last.*?OK')

    def testDoLastSkipped(self):
        """
        Confirm 'do_last' is skipped if a test it depends on failed/skipped.
        """
        out = self.runTests('--no-color', '-i', 'do_last_skipped').output
        self.assertRegex(out, 'test_harness.do_last.*?\[SKIPPED DEPENDENCY\] SKIP')

    def testDoLastName(self):
        """
        Test for invalid use where a test name is 'ALL' when 'prereq = ALL' is set
        """
        out = self.runTests('--no-color', '-i', 'do_last_name', exit_code=132).output
        self.assertRegex(out, 'test_harness.*?FAILED \(Test named ALL when "prereq = ALL" elsewhere in test spec file!\)')
