# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

"""Test using application names in capabilities."""

from unittest import main

from TestHarnessTestCase import TestHarnessTestCase


def buildTestSpecs(**kwargs) -> dict:
    """Build a dummy test spec."""
    return {
        "test": {"type": "RunApp", "input": "unused", "should_execute": False, **kwargs}
    }


class TestHarnessTester(TestHarnessTestCase):
    """Test using application names in capabilities."""

    def testAppExists(self):
        """Test a required application as a capability when it exists."""
        result = self.runTests(
            "--no-color", tests=buildTestSpecs(capabilities="moosetestapp")
        )

        out = result.output
        self.assertRegex(out, r"test\.test[\s.]+OK")

        self.checkStatus(result.harness, passed=1)

    def testAppMissing(self):
        """Test a required application as a capability when it does not exist."""
        result = self.runTests(
            "--no-color", tests=buildTestSpecs(capabilities="fooapp")
        )

        out = result.output
        self.assertRegex(out, r"test\.test[\s.]+\[NEED FOOAPP\]\s+SKIP")

        self.checkStatus(result.harness, skipped=1)


if __name__ == "__main__":
    main()
