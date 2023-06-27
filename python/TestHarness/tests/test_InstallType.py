#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import os, sys, io
import unittest
import mock
import TestHarness
from contextlib import redirect_stdout

class TestHarnessTester(unittest.TestCase):
    @mock.patch.object(TestHarness.util, 'checkInstalled')
    def mocked_output(self, mocked, expect_fail, mocked_return):
        MOOSE_DIR = os.getenv('MOOSE_DIR')
        os.chdir(f'{MOOSE_DIR}/test')
        out = io.StringIO()
        with redirect_stdout(out):
            mocked_return.return_value=mocked
            harness = TestHarness.TestHarness(['', '-i', 'install_type', '-c'], MOOSE_DIR)
            if expect_fail:
                with self.assertRaises(SystemExit):
                    harness.findAndRunTests()
            else:
                harness.findAndRunTests()
        return out.getvalue()

    def testInstalled(self):
        """
        Test which only runs if binary is installed
        """
        out = self.mocked_output(set(['ALL', 'INSTALLED']), False)
        self.assertRegex(out, r'.*?SKIP.*?in_tree_type.*?"IN_TREE" binary]')
        self.assertRegex(out, r'.*?OK.*?installed_type')
        self.assertRegex(out, r'.*?OK.*?all_type')

    def testInTree(self):
        """
        Test which only runs if binary is in_tree
        """
        out = self.mocked_output(set(['ALL', 'IN_TREE']), False)
        self.assertRegex(out, r'.*?SKIP.*?installed_type.*?"INSTALLED" binary]')
        self.assertRegex(out, r'.*?OK.*?in_tree_type')
        self.assertRegex(out, r'.*?OK.*?all_type')
