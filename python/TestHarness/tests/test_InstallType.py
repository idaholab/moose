#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import os, io
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
            cmd = ['', '-i', 'install_type', '-c', '--term-format', 'njCst']
            with self.assertRaises(SystemExit) as e:
                TestHarness.TestHarness.buildAndRun(cmd, None, MOOSE_DIR)
            if expect_fail:
                self.assertNotEqual(e.exception.code, 0)
            else:
                self.assertEqual(e.exception.code, 0)
        return out.getvalue()

    def testInstalled(self):
        """
        Test which only runs if binary is installed
        """
        out = self.mocked_output(set(['ALL', 'INSTALLED']), False)
        self.assertRegex(out, r'tests\/test_harness\.in_tree_type[\s.]+\[TEST REQUIRES "IN_TREE" BINARY\]\s+SKIP')
        self.assertRegex(out, r'tests\/test_harness\.installed_type[\s.]+OK')
        self.assertRegex(out, r'tests\/test_harness\.all_type[\s.]+OK')

    def testInTree(self):
        """
        Test which only runs if binary is in_tree
        """
        out = self.mocked_output(set(['ALL', 'IN_TREE']), False)
        self.assertRegex(out, r'tests\/test_harness\.in_tree_type[\s.]+OK')
        self.assertRegex(out, r'tests\/test_harness\.installed_type[\s.]+\[TEST REQUIRES "INSTALLED" BINARY\]\s+SKIP')
        self.assertRegex(out, r'tests\/test_harness\.all_type[\s.]+OK')
