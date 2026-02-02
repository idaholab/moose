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
from TestHarness.util import getPlatform


class TestRequireCapability(TestHarnessTestCase):
    """Test the --only-tests-that-require option."""

    def testTestHarnessBuildRequiredCapabilities(self):
        """Test TestHarness.buildRequiredCapabilities()."""
        # Not registered
        with self.assertRaisesRegex(
            SystemExit, 'Required capability "bar" is not registered'
        ):
            TestHarness.buildRequiredCapabilities(["foo"], ["bar"])

        # True value -> false augmented value
        res = TestHarness.buildRequiredCapabilities(["foo"], ["foo"])
        self.assertEqual(res, [("foo", False)])

        # False value -> true augmented value
        res = TestHarness.buildRequiredCapabilities(["foo"], ["!foo"])
        self.assertEqual(res, [("foo", True)])

        # Has stripping
        res = TestHarness.buildRequiredCapabilities(["foo"], ["  foo "])
        self.assertEqual(len(res), 1)
        self.assertEqual(res[0][0], "foo")

        # Multiple
        res = TestHarness.buildRequiredCapabilities(["foo", "bar"], ["foo", "!bar"])
        self.assertEqual(len(res), 2)
        self.assertEqual(res[0], ("foo", False))
        self.assertEqual(res[1], ("bar", True))

    def testTestHarnessOptions(self):
        """Test that the test harness will set the _required_capabilities option."""
        # Not registered
        with self.assertRaisesRegex(
            SystemExit, 'Required capability "foo" is not registered'
        ):
            self.runTests("--only-tests-that-require", "foo", minimal_capabilities=True)

        # Is registered and is set
        res = self.runTests(
            "--only-tests-that-require",
            "platform",
            minimal_capabilities=True,
            run=False,
        )
        self.assertEqual(
            res.harness.options._required_capabilities, [("platform", False)]
        )

        # Multiple
        res = self.runTests(
            "--only-tests-that-require",
            "platform",
            "--only-tests-that-require",
            "installation_type",
            minimal_capabilities=True,
            run=False,
        )
        self.assertEqual(
            res.harness.options._required_capabilities,
            [("platform", False), ("installation_type", False)],
        )

    def testTesterSkip(self):
        """Test the Tester skipping tests based on --required-capabilities."""

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
                self.assertEqual(
                    job.getCaveats(), set(["Missing required capabilities"])
                )
            else:
                self.assertEqual(job.getStatus(), StatusSystem.finished)

        # Capability is not in the test spec capabilities
        run_test(True, ["platform"])
        run_test(True, ["platform"], "machine")

        # Capability is in the test spec capabilities
        run_test(False, ["installation_type"], "installation_type=in_tree")

        # Test spec has complex capabilities and still runs
        run_test(
            False,
            ["installation_type"],
            f"installation_type=in_tree & platform={getPlatform()}",
        )

        # Multiple --only-tests-that-require
        run_test(
            False, ["installation_type", "platform"], "installation_type & platform"
        )
        run_test(
            False,
            ["installation_type", "platform"],
            "installation_type & platform & compute_device",
        )


if __name__ == "__main__":
    unittest.main()
