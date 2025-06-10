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
    def testShouldExecute(self):
        """
        Test should_execute logic
        """

        out = self.runTests('-i', 'should_execute', exit_code=129).output
        self.assertRegex(out, r'test_harness\.should_execute_true_ok.*?OK')
        self.assertRegex(out, r'test_harness\.should_execute_false_ok.*?OK')
        self.assertRegex(out, r'test_harness\.should_execute_true_fail.*?FAILED \(EXODIFF\)')
