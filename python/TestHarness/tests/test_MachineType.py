# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

"""Test machine capability."""

from unittest import main

from TestHarnessTestCase import TestHarnessTestCase

from TestHarness.util import getMachine

THIS_MACHINE = getMachine()
BAD_MACHINE = "arm64" if THIS_MACHINE == "x86_64" else "x86_64"


def buildTestSpecs(**kwargs) -> dict:
    """Build a dummy test spec."""
    return {
        "test": {"type": "RunApp", "input": "unused", "should_execute": False, **kwargs}
    }


class TestHarnessTester(TestHarnessTestCase):
    """Test machine capability."""

    def testNotSetMinimal(self):
        """No machine set with minimal capabilities; should not skip."""
        out = self.runTests(
            "-c", tests=buildTestSpecs(), minimal_capabilities=True
        ).output
        self.assertRegex(out, r"test\.test[\s.]+OK")

    def testNotSetFull(self):
        """No machine set with full capabilities; should not skip."""
        out = self.runTests(
            "-c", tests=buildTestSpecs(), minimal_capabilities=False
        ).output
        self.assertRegex(out, r"test\.test[\s.]+OK")

    def testEqualMinimal(self):
        """Correct machine set with minimal capabilities; should not skip."""
        test_specs = buildTestSpecs(capabilities=f"machine={THIS_MACHINE}")
        out = self.runTests("-c", tests=test_specs, minimal_capabilities=True).output
        self.assertRegex(out, r"test\.test[\s.]+OK")

    def testEqualFull(self):
        """Correct machine set with full capabilities; should not skip."""
        test_specs = buildTestSpecs(capabilities=f"machine={THIS_MACHINE}")
        out = self.runTests("-c", tests=test_specs, minimal_capabilities=True).output
        self.assertRegex(out, r"test\.test[\s.]+OK")

    def testSkippedMinimal(self):
        """Machine with minimal capabilities and a different machine; should skip."""
        test_specs = buildTestSpecs(capabilities=f"machine={BAD_MACHINE}")
        out = self.runTests(
            "-c",
            tests=test_specs,
            minimal_capabilities=True,
        ).output
        self.assertRegex(
            out, rf"test\.test[\s.]+\[NEED MACHINE={BAD_MACHINE.upper()}\]\s+SKIP"
        )

    def testSkippedFull(self):
        """Machine with capabilities and a different machine should be skipped."""
        test_specs = buildTestSpecs(capabilities=f"machine={BAD_MACHINE}")
        out = self.runTests("-c", tests=test_specs, minimal_capabilities=False).output
        self.assertRegex(
            out, rf"test\.test[\s.]+\[NEED MACHINE={BAD_MACHINE.upper()}\]\s+SKIP"
        )


if __name__ == "__main__":
    main()
