#* This file is part of the MOOSE framework
#* https://mooseframework.inl.gov
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import os, io, glob
import unittest
import mock
import TestHarness
from contextlib import redirect_stdout

class TestHarnessTester(unittest.TestCase):
    def __init__(self, methodName='runTest'):
        super().__init__(methodName)

        self.MOOSE_DIR = os.getenv('MOOSE_DIR')
        self.all_methods = {'opt', 'oprof', 'dbg', 'devel'}

    def helperGetPresentMethods(self, directory):
        """
        Returns methods of executables matching the pattern "*-method" in location directory.
        """

        prev_dir = os.getcwd()
        os.chdir(os.path.join(self.MOOSE_DIR, directory))

        try:
            present_methods = set()
            for method in self.all_methods:
                matches = glob.glob(f"*-{method}")
                if matches:
                    present_methods.add(method)

        finally:
            os.chdir(prev_dir)

        return present_methods

    def helperTestGetExecutable(self, testroot_path, method, expect_fail):
        os.chdir(os.path.join(self.MOOSE_DIR, testroot_path))
        out = io.StringIO()
        with redirect_stdout(out):
            with self.assertRaises(SystemExit) as e:
                args = ['', method, '--test-root', 'testroot']
                TestHarness.TestHarness.buildAndRun(args, None, self.MOOSE_DIR)
            if expect_fail:
                self.assertNotEqual(e.exception.code, 0)
            else:
                self.assertEqual(e.exception.code, 0)
        return out.getvalue()

    def testMethodIsAbsentInTest(self):
        """
        Test for method that does not exist in test
        """
        test_dir = 'test'
        present_methods = self.helperGetPresentMethods(test_dir)
        absent_methods = self.all_methods - present_methods
        if absent_methods:
            method = list(absent_methods)[0]
        else:
            # Cannot test for an absent method when all methods are present
            return

        out = self.helperTestGetExecutable(test_dir, f'--{method}', True)
        self.assertIn('Failed to find MOOSE executable', out)

    def testMethodIsAbsentInCombined(self):
        """
        Test for method that does not exist in combined
        """
        test_dir = os.path.join('modules', 'combined')
        present_methods = self.helperGetPresentMethods(test_dir)
        absent_methods = self.all_methods - present_methods
        if absent_methods:
            method = list(absent_methods)[0]
        else:
            # Cannot test for an absent method when all methods are present
            return

        out = self.helperTestGetExecutable('modules', f'--{method}', True)
        self.assertIn('Failed to find MOOSE executable', out)

    def testMethodIsPresentInTest(self):
        """
        Test for method that exists in test
        """
        test_dir = 'test'
        present_methods = self.helperGetPresentMethods(test_dir)
        if present_methods:
            method = list(present_methods)[0]
        else:
            # Cannot test for a present method when all methods are absent
            return

        out = self.helperTestGetExecutable(test_dir, f'--{method}', False)
        self.assertNotIn('Failed to find MOOSE executable', out)

    def testMethodIsPresentInCombined(self):
        """
        Test for method that exists in combined
        """
        test_dir = os.path.join('modules', 'combined')
        present_methods = self.helperGetPresentMethods(test_dir)
        if present_methods:
            method = list(present_methods)[0]
        else:
            # Cannot test for a present method when all methods are absent
            return

        out = self.helperTestGetExecutable('modules', f'--{method}', False)
        self.assertNotIn('Failed to find MOOSE executable', out)

if __name__ == '__main__':
    unittest.main()
