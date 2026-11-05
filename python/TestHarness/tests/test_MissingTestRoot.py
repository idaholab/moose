# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

import os
import shutil
import tempfile
import unittest
from contextlib import redirect_stdout
from io import StringIO

from TestHarness.TestHarness import TestHarness
from TestHarness.tests.TestHarnessTestCase import MOOSE_DIR


class TestMissingTestRoot(unittest.TestCase):
    def setUp(self):
        self.dir = os.path.realpath(tempfile.mkdtemp())
        self.orig_cwd = os.getcwd()

    def tearDown(self):
        shutil.rmtree(self.dir, ignore_errors=True)
        os.chdir(self.orig_cwd)

    def buildWithoutTestRoot(self, argv):
        """Builds the TestHarness without an app_name and returns the exit code and output"""
        stdout = StringIO()
        with redirect_stdout(stdout), self.assertRaises(SystemExit) as e:
            TestHarness.build(["unused", "--no-color"] + argv, None, MOOSE_DIR)
        return e.exception.code, stdout.getvalue()

    def testFromCurrentDirectory(self):
        os.chdir(self.dir)
        code, output = self.buildWithoutTestRoot([])
        self.assertEqual(code, 1)
        self.assertIn(
            f"ERROR: Failed to find a testroot file in {self.dir} or any directory above it",
            output,
        )
        self.assertIn("--test-root/-C", output)

    def testFromTestRootArgument(self):
        test_dir = os.path.join(self.dir, "test")
        os.mkdir(test_dir)
        code, output = self.buildWithoutTestRoot(["--test-root", test_dir])
        self.assertEqual(code, 1)
        self.assertIn(f"Failed to find a testroot file in {test_dir} ", output)


if __name__ == "__main__":
    unittest.main()
