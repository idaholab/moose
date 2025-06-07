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
    @patch('TestHarness.util.checkInstalled')
    def testInstalled(self, mock_check_installed):
        """
        Test which only runs if binary is installed
        """
        mock_check_installed.return_value = set(['ALL', 'INSTALLED'])
        out = self.runTests('-i', 'install_type', '-c', no_capabilities=False).output
        self.assertRegex(out, r'tests\/test_harness\.in_tree_type[\s.]+\[TEST REQUIRES "IN_TREE" BINARY\]\s+SKIP')
        self.assertRegex(out, r'tests\/test_harness\.installed_type[\s.]+OK')
        self.assertRegex(out, r'tests\/test_harness\.all_type[\s.]+OK')

    @patch('TestHarness.util.checkInstalled')
    def testInTree(self, mock_check_installed):
        """
        Test which only runs if binary is in_tree
        """
        mock_check_installed.return_value = set(['ALL', 'IN_TREE'])
        out = self.runTests('-i', 'install_type', '-c', no_capabilities=False).output
        self.assertRegex(out, r'tests\/test_harness\.in_tree_type[\s.]+OK')
        self.assertRegex(out, r'tests\/test_harness\.installed_type[\s.]+\[TEST REQUIRES "INSTALLED" BINARY\]\s+SKIP')
        self.assertRegex(out, r'tests\/test_harness\.all_type[\s.]+OK')
