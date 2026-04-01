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

    def testGood(self):
        """Test a installation_type=in_tree spec and the app is in tree."""
        out = self.runTests(
            "--no-color",
            tests=buildTestSpecs(capabilities="installation_type=in_tree"),
        ).output
        self.assertRegex(out, r"test\.test[\s.]+OK")

    def testSkip(self):
        """Test a installation_type=relocated spec and the app is in tree."""
        out = self.runTests(
            "--no-color",
            tests=buildTestSpecs(capabilities="installation_type=relocated"),
        ).output
        self.assertRegex(
            out, r"test\.test[\s.]+\[NEED INSTALLATION_TYPE=RELOCATED\]\s+SKIP"
        )


if __name__ == "__main__":
    main()
