#* This file is part of the MOOSE framework
#* https://mooseframework.inl.gov
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from TestHarnessTestCase import TestHarnessTestCase
from unittest.mock import patch

class TestHarnessTester(TestHarnessTestCase):
    @patch('TestHarness.util.getMachine')
    def testNotSkipped(self, mock_get_machine):
        """
        Test should not be skipped, as it is set to run on any arch (ALL)
        """
        mock_get_machine.return_value = set(['ALL'])
        out = self.runTests('-i', 'always_ok', '-c').output
        self.assertRegex(out, r'tests\/test_harness\.always_ok[\s.]+OK')

    @patch('TestHarness.util.getMachine')
    def testSkipped(self, mock_get_machine):
        """
        Test that a non existing machine type is skipped (remove default of ALL)
        """
        mock_get_machine.return_value = set([''])
        out = self.runTests('-i', 'always_ok', '-c').output
        self.assertRegex(out, r'tests\/test_harness\.always_ok[\s.]+\[MACHINE!=ALL\]\s+SKIP')
