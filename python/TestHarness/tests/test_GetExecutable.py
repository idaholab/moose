# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

"""Test TestHarness.getExecutable()."""

import glob
import os
import unittest

from TestHarnessTestCase import MOOSE_DIR, TestHarnessTestCase


class TestGetExecutable(TestHarnessTestCase):
    """Test TestHarness.getExecutable()."""

    ALL_METHODS = {"opt", "oprof", "dbg", "devel"}

    @staticmethod
    def helperGetPresentMethods(directory):
        """Get executables matching the pattern "*-method" in location directory."""
        prev_dir = os.getcwd()
        os.chdir(os.path.join(MOOSE_DIR, directory))

        try:
            present_methods = set()
            for method in TestGetExecutable.ALL_METHODS:
                matches = glob.glob(f"*-{method}")
                if matches:
                    present_methods.add(method)

        finally:
            os.chdir(prev_dir)

        return present_methods

    def testMethodIsAbsent(self):
        """Test for method that does not exist in test."""
        test_dir = "test"
        present_methods = self.helperGetPresentMethods(test_dir)
        absent_methods = self.ALL_METHODS - present_methods
        if absent_methods:
            method = list(absent_methods)[0]
        else:
            # Cannot test for an absent method when all methods are present
            return

        with self.assertRaisesRegex(SystemExit, "Failed to find MOOSE executable"):
            self.runTests(f"--{method}")

    def testMethodIsPresent(self):
        """Test for method that exists in test."""
        test_dir = "test"
        present_methods = self.helperGetPresentMethods(test_dir)
        if present_methods:
            method = list(present_methods)[0]
        else:
            # Cannot test for a present method when all methods are absent
            return

        self.runTests(f"--{method}", run=False)


if __name__ == "__main__":
    unittest.main()
