# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

"""Test using installation_type in capabilities."""

from unittest import main

from TestHarnessTestCase import TestHarnessTestCase


def buildTestSpecs(**kwargs) -> dict:
    """Build a dummy test spec."""
    return {
        "test": {"type": "RunApp", "input": "unused", "should_execute": False, **kwargs}
    }


class TestHarnessTester(TestHarnessTestCase):
    """Test using installation_type in capabilities."""

    def testInstalledGood(self):
        """Test a installation_type=installed spec and the app is installed."""
        out = self.runTests(
            "--no-color",
            extra_capabilities={"installation_type": ["installed", "unused doc"]},
            tests=buildTestSpecs(capabilities="installation_type=installed"),
            minimal_capabilities=True,
        ).output
        self.assertRegex(out, r"test\.test[\s.]+OK")

    def testInstalledSkip(self):
        """Test a installation_type=installed spec and the app is in tree."""
        out = self.runTests(
            "--no-color",
            extra_capabilities={"installation_type": ["in_tree", "unused doc"]},
            tests=buildTestSpecs(capabilities="installation_type=installed"),
            minimal_capabilities=True,
        ).output
        self.assertRegex(
            out, r"test\.test[\s.]+\[NEEDS: INSTALLATION_TYPE=INSTALLED\]\s+SKIP"
        )

    def testInTreeGood(self):
        """Test a installation_type=in_tree spec and the app is installed."""
        out = self.runTests(
            "--no-color",
            extra_capabilities={"installation_type": ["in_tree", "unused doc"]},
            tests=buildTestSpecs(capabilities="installation_type=in_tree"),
            minimal_capabilities=True,
        ).output
        self.assertRegex(out, r"test\.test[\s.]+OK")

    def testInTreeSkip(self):
        """Test a installation_type=in_tree spec and the app is installed."""
        out = self.runTests(
            "--no-color",
            extra_capabilities={"installation_type": ["installed", "unused doc"]},
            tests=buildTestSpecs(capabilities="installation_type=in_tree"),
            minimal_capabilities=True,
        ).output
        self.assertRegex(
            out, r"test\.test[\s.]+\[NEEDS: INSTALLATION_TYPE=IN_TREE\]\s+SKIP"
        )


if __name__ == "__main__":
    main()
