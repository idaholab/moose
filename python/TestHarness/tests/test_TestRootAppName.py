# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

"""Test the app_name resolution when running tests in another folder with --test-root."""

import os
import shutil
import tempfile
import unittest
from contextlib import redirect_stdout
from io import StringIO

from TestHarness.TestHarness import TestHarness
from TestHarness.tests.TestHarnessTestCase import MOOSE_DIR

APP_NAME = "test_harness_test"


class TestTestRootAppName(unittest.TestCase):
    def setUp(self):
        self.dir = os.path.realpath(tempfile.mkdtemp())
        self.orig_cwd = os.getcwd()

        # An application folder with a testroot naming the app and its executable
        self.app_dir = os.path.join(self.dir, "app")
        os.mkdir(self.app_dir)
        self.writeTestRoot(self.app_dir, f"app_name = {APP_NAME}\n")
        for method in ["opt", "oprof", "dbg", "devel"]:
            with open(os.path.join(self.app_dir, f"{APP_NAME}-{method}"), "w") as f:
                f.write("")

        # An external test suite folder
        self.suite_dir = os.path.join(self.dir, "suite")
        os.mkdir(self.suite_dir)

    def tearDown(self):
        os.chdir(self.orig_cwd)
        shutil.rmtree(self.dir, ignore_errors=True)

    @staticmethod
    def writeTestRoot(directory, content):
        with open(os.path.join(directory, "testroot"), "w") as f:
            f.write(content)

    def build(self, *args):
        return TestHarness.build(
            ["unused", "--minimal-capabilities", "--test-root", *args], None, MOOSE_DIR
        )

    def testFallbackNoAppName(self):
        """Suite testroot without an app_name uses the app of the current directory."""
        self.writeTestRoot(self.suite_dir, "allow_warnings = true\n")
        os.chdir(self.app_dir)
        harness = self.build(self.suite_dir)
        self.assertEqual(harness.app_name, APP_NAME)
        self.assertEqual(harness._rootdir, self.suite_dir)
        self.assertEqual(os.path.dirname(harness.executable), self.app_dir)
        # The suite testroot parameters are kept
        self.assertEqual(harness.root_params.get("allow_warnings"), True)

    def testFallbackNoTestRoot(self):
        """Suite without any testroot uses the testroot of the current directory."""
        os.chdir(self.app_dir)
        harness = self.build(self.suite_dir)
        self.assertEqual(harness.app_name, APP_NAME)
        self.assertEqual(harness._rootdir, self.app_dir)

    def testExplicitEmptyAppName(self):
        """An empty app_name in the suite testroot means no application."""
        self.writeTestRoot(self.suite_dir, "app_name = ''\n")
        os.chdir(self.app_dir)
        harness = self.build(self.suite_dir)
        self.assertIsNone(harness.app_name)
        self.assertIsNone(harness.executable)

    def testNoAppNameError(self):
        """No app_name in the suite testroot nor from the current directory errors."""
        self.writeTestRoot(self.suite_dir, "allow_warnings = true\n")
        os.chdir(self.dir)
        stdout = StringIO()
        with redirect_stdout(stdout), self.assertRaises(SystemExit):
            self.build(self.suite_dir)
        self.assertIn("does not specify an app_name", stdout.getvalue())
        self.assertIn("pass --app-name <name>", stdout.getvalue())

    def testCLIAppName(self):
        """--app-name is used without a fallback and without an error."""
        self.writeTestRoot(self.suite_dir, "allow_warnings = true\n")
        os.chdir(self.app_dir)
        harness = self.build(self.suite_dir, "--app-name", APP_NAME)
        self.assertEqual(harness.app_name, APP_NAME)


if __name__ == "__main__":
    unittest.main()
