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

from TestHarness.capability_util import parseRequiredCapabilities
from TestHarness.StatusSystem import StatusSystem


class TestRequireCapability(TestHarnessTestCase):
    # """Test the --only-tests-that-require option."""

    def getAugmentedCapabilityNames(self) -> list[str]:
        """
        Get pycapabilities.AUGMENTED_CAPABILITY_NAMES.

        Done here instead of at file level to avoid pycapabilites build.
        """
        from pycapabilities import AUGMENTED_CAPABILITY_NAMES

        names = list(AUGMENTED_CAPABILITY_NAMES)
        self.assertGreater(len(names), 1)
        return names

    def testParseRequiredCapabilities(self):
        """Test TestHarness.capability_util.parseRequiredCapabilities()."""
        from pycapabilities import Capabilities

        names = self.getAugmentedCapabilityNames()
        name1 = names[0]
        name2 = names[1]

        capabilities = Capabilities(
            {
                "appvalue": {"doc": "doc", "value": "foo"},
                "falsevalue": {"doc": "doc", "value": False},
            }
        )

        # success
        required = parseRequiredCapabilities(["appvalue", name1, name2], capabilities)
        self.assertEqual(required, ["appvalue", name1, name2])

        # unallowed characters
        with self.assertRaisesRegex(
            ValueError, "Capability 'fOO!' has unallowed characters"
        ):
            parseRequiredCapabilities(["fOO!"], capabilities)

        # registered but false
        with self.assertRaisesRegex(ValueError, "Capability 'falsevalue' is false"):
            parseRequiredCapabilities(["falsevalue"], capabilities)

        # not registered
        with self.assertRaisesRegex(ValueError, "Capability 'foo' is not registered"):
            parseRequiredCapabilities(["foo"], capabilities)

    def testTestHarnessOptions(self):
        """Test that the test harness will set the _required_capabilities option."""
        # Single
        res = self.runTests(
            "--only-tests-that-require",
            "test_one",
            run=False,
            minimal_capabilities=False,
        )
        assert res.harness is not None
        self.assertEqual(res.harness.options._required_capabilities, ["test_one"])

        # Multiple
        res = self.runTests(
            "--only-tests-that-require",
            "test_one",
            "--only-tests-that-require",
            "test_string",
            run=False,
            minimal_capabilities=False,
        )
        assert res.harness is not None
        self.assertEqual(
            res.harness.options._required_capabilities, ["test_one", "test_string"]
        )

        # Catch exceptions with CLI error
        with self.assertRaisesRegex(
            SystemExit,
            "ERROR: --only-tests-that-require: Capability 'fOO!' has unallowed "
            "characters",
        ):
            self.runTests(
                "--only-tests-that-require", "fOO!", minimal_capabilities=True
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
                minimal_capabilities=False,
            )
            assert res.harness is not None
            job = self.getJobWithName(res.harness, test_name)
            if skip:
                self.assertEqual(job.getStatus(), StatusSystem.skip)
                self.assertEqual(job.getCaveats(), set(["!" + ", !".join(skip)]))
            else:
                self.assertEqual(job.getStatus(), StatusSystem.finished)

        # Single requirement, test has no capabilities
        run_test(["test_one"], ["test_one"])
        # Single requirement, test has capabilities but not the right one
        run_test(["test_one"], ["test_one"], "test_string")
        # Single requirement, test has capabilities and the right one
        run_test([], ["test_one"], "test_one")
        # Two requirements, test has no capabilities
        run_test(["test_one", "test_two_explicit"], ["test_one", "test_two_explicit"])
        # Two requirements, test has one of them
        run_test(["test_string"], ["test_one", "test_string"], "test_one")
        # Two requirements, test has both of them
        run_test([], ["test_string", "test_string"], "test_one & test_string")
        # Requirement from a Tester augmented capability
        # (mpi_procs is added in RunApp)
        run_test(["mpi_procs"], ["mpi_procs"], "test_one")
        run_test([], ["mpi_procs"], "mpi_procs=1")


if __name__ == "__main__":
    unittest.main()
