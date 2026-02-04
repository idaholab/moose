# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

"""Test the --only-tests-that-require option."""

import unittest

from TestHarnessTestCase import TestHarnessTestCase

from TestHarness import TestHarness
from TestHarness.StatusSystem import StatusSystem
from TestHarness.util import getPlatform, getMachine
from TestHarness.capability_util import parseRequiredCapabilities

MACHINE = getMachine()
PLATFORM = getPlatform()
class TestRequireCapability(TestHarnessTestCase):
    """Test the --only-tests-that-require option."""

    def testParseRequiredCapabilities(self):
        """Test TestHarness.capability_util.parseRequiredCapabilities()."""
        # Has stripping
        self.assertEqual(parseRequiredCapabilities([" foo", "bar  "]), ["foo", "bar"])

        # Unallowed characters
        with self.assertRaisesRegex(
            ValueError, r"Capability 'foo!=\?' has unallowed characters"
        ):
            parseRequiredCapabilities(["foo!=?"])

        # Works
        self.assertEqual(parseRequiredCapabilities(["foo", "bar"]), ["foo", "bar"])

    def testTestHarnessOptions(self):
        """Test that the test harness will set the _required_capabilities option."""
        # Single
        res = self.runTests(
            "--only-tests-that-require",
            "platform",
            minimal_capabilities=True,
            run=False,
        )
        self.assertEqual(res.harness.options._required_capabilities, ["platform"])

        # Multiple
        res = self.runTests(
            "--only-tests-that-require",
            "hpc",
            "--only-tests-that-require",
            "machine",
            minimal_capabilities=True,
            run=False,
        )
        self.assertEqual(
            res.harness.options._required_capabilities,
            ["hpc", "machine"],
        )

    def testTesterSkip(self):
        """Test the Tester skipping tests with --only-tests-that-require."""

        def run_test(skip, require_capabilities, test_capabilities=None):
            test_name = "test"
            tests = {
                test_name: {
                    "type": "RunApp",
                    "input": "unused",
                    "should_execute": False,
                }
            }
            if test_capabilities:
                tests[test_name]["capabilities"] = f"'{test_capabilities}'"
            args = []
            for v in require_capabilities:
                args += ["--only-tests-that-require", v]
            res = self.runTests(
                *args,
                tests=tests,
                minimal_capabilities=True,
            )
            job = self.getJobWithName(res.harness, test_name)
            if skip:
                self.assertEqual(job.getStatus(), StatusSystem.skip)
                self.assertEqual(job.getCaveats(), set(["!" + ", !".join(skip)]))
            else:
                self.assertEqual(job.getStatus(), StatusSystem.finished)

        # Single requirement, test has no capabilities
        run_test(["foo"], ["foo"])
        # Single requirement, test has capabilities but not the right one
        run_test(["foo"], ["foo"], f"machine={MACHINE}")
        # Single requirement, test has capabilities and the right one
        run_test([], ["machine"], f"machine={MACHINE}")
        # Single requirement, test has capabilities and the right one
        run_test([], ["machine"], f"machine={MACHINE}")
        # Two requirements, test has no capabilities
        run_test(["foo", "bar"], ["foo", "bar"])
        # Two requirements, test has one of them
        run_test(["foo"], ["machine", "foo"], f"machine={MACHINE}")
        # Two requirements, test has both of them
        run_test(
            [], ["machine", "platform"], f"machine={MACHINE} & platform={PLATFORM}"
        )
        # Requirement from a Tester augmented capability
        # (mpi_procs is added in RunApp)
        run_test(["mpi_procs"], ["mpi_procs"], f"machine={MACHINE}")
        run_test([], ["mpi_procs"], "mpi_procs=1")


if __name__ == "__main__":
    unittest.main()
