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
    def testDeleted(self):
        """
        Test that deleted tests returns a failed deleted test when extra info argument is supplied
        """
        out = self.runTests('--no-color', '-i', 'deleted', '-e', exit_code=131).output
        self.assertRegex(out, r'test_harness\.deleted.*? \[TEST DELETED TEST\] FAILED \(DELETED\)')

    def testNoExtraInfo(self):
        """
        Test that deleted tests do not run without -e (extra) option
        """
        output = self.runTests('--no-color', '-i', 'deleted').output
        self.assertNotIn('tests/test_harness.deleted', output)
