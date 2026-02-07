# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

"""Test the use of capabilites in the TestHarness."""

import unittest
from argparse import Namespace
from copy import deepcopy
from dataclasses import dataclass
from typing import Any, Optional, Tuple

from TestHarnessTestCase import TestHarnessTestCase

from TestHarness import TestHarness
from TestHarness.capability_util import addAugmentedCapability
from TestHarness.testers.Tester import Tester
from TestHarness.util import getMachine, getPlatform


class TestAugmentedCapabilities(TestHarnessTestCase):
    """Test the use of capabilites in the TestHarness."""

    def testAddAugmentedCapability(self):
        """Test TestHarness.capability_util.addAugmentedCapability."""
        # Name converts to lowercase
        augmented = {}
        addAugmentedCapability({}, augmented, "FOO", None, "doc")
        self.assertIn("foo", augmented)

        # String value converts to lowercase
        augmented = {}
        addAugmentedCapability({}, augmented, "foo", "FOO", "doc")
        self.assertEqual(augmented["foo"]["value"], "foo")

        # Enumeration can only be used for string capabilities
        with self.assertRaisesRegex(
            AssertionError, "Enumeration only valid for str capabilities"
        ):
            addAugmentedCapability({}, {}, "foo", True, "doc", enumeration=["foo"])
        with self.assertRaisesRegex(
            AssertionError, "Enumeration only valid for str capabilities"
        ):
            addAugmentedCapability({}, {}, "foo", 0, "doc", enumeration=["foo"])

        # Enumeration converted to lower
        augmented = {}
        addAugmentedCapability(
            {}, augmented, "foo", "value", "doc", enumeration=["VALUE"]
        )
        self.assertEqual(augmented["foo"]["enumeration"], ["value"])

        # Name already exists in main capabilities
        with self.assertRaisesRegex(ValueError, "Capability foo is defined by the app"):
            addAugmentedCapability({"foo": None}, {}, "foo", None, "doc")

        # Name already exists in augmented capabilities
        with self.assertRaisesRegex(
            ValueError, "Capability foo is already defined as an augmented capability."
        ):
            addAugmentedCapability({}, {"foo": None}, "foo", None, "doc")

        # Build a bool value
        for value, expected_value in [(None, False), (False, False), (True, True)]:
            augmented = {}
            addAugmentedCapability({}, augmented, "foo", value, "somedoc")
            self.assertEqual(
                augmented, {"foo": {"doc": "somedoc", "value": expected_value}}
            )

        # Build a int value
        for explicit in [None, False, True]:
            augmented = {}
            kwargs = {}
            if explicit is not None:
                kwargs["explicit"] = explicit
            addAugmentedCapability({}, augmented, "foo", 1, "somedoc", **kwargs)
            expected = {"foo": {"doc": "somedoc", "value": 1, **kwargs}}
            self.assertEqual(augmented, expected)

        # Build a string value
        for enumeration in [None, ["bar", "baz"]]:
            for explicit in [None, False, True]:
                augmented = {}
                kwargs = {}
                if explicit is not None:
                    kwargs["explicit"] = explicit
                if enumeration is not None:
                    kwargs["enumeration"] = enumeration
                addAugmentedCapability({}, augmented, "foo", "bar", "somedoc", **kwargs)
                expected = {"foo": {"doc": "somedoc", "value": "bar", **kwargs}}
                self.assertEqual(augmented, expected)

    def testGetCapabilitiesMinimal(self):
        """Test TestHarness.getCapabilities() with --minimal-capabilities."""
        default_options = Namespace()
        default_options.minimal_capabilities = True
        default_options.only_tests_that_require = None
        default_options.hpc = None

        def test_get_capabilities(
            check_capabilities: list[Tuple[str, Any]],
            check_required_capabilities: Optional[list[Tuple[str, bool]]] = None,
            **kwargs,
        ):
            options = deepcopy(default_options)
            for key, value in kwargs.items():
                setattr(options, key, value)

            capabilities, augmented_capabilities, required_capabilities = (
                TestHarness.getCapabilities(options, None, None)
            )
            self.assertIsInstance(capabilities.values, dict)
            self.assertIsInstance(augmented_capabilities, dict)
            self.assertIsInstance(required_capabilities, list)
            for capability, value in check_capabilities:
                self.assertIn(capability, capabilities.values)
                self.assertIn(capability, augmented_capabilities)
                entry = capabilities.values[capability]
                self.assertEqual(entry, augmented_capabilities[capability])
                self.assertEqual(entry["value"], value)
            if check_required_capabilities is None:
                self.assertEqual(len(required_capabilities), 0)
            else:
                self.assertEqual(
                    len(required_capabilities), len(check_required_capabilities)
                )
                for entry in check_required_capabilities:
                    self.assertIn(entry, check_required_capabilities)

        # Default case
        check_capabilities = [
            ("hpc", False),
            ("machine", getMachine()),
            ("platform", getPlatform()),
        ]
        test_get_capabilities(check_capabilities)

        # --hpc
        test_get_capabilities([("hpc", True)], hpc="foo")

        # --only-tests-that-require
        test_get_capabilities(
            [],
            check_required_capabilities=[("machine", False)],
            only_tests_that_require="machine",
        )

    @dataclass
    class CapabilityTestCase:
        """Define a test case for a capability test."""

        capabilities: str
        skip: bool
        params: Optional[dict] = None

    def runCapabilityTest(
        self, *cases: tuple, cli_args: Optional[list[str]] = None, exit_code: int = 0
    ) -> Tuple[TestHarnessTestCase.RunTestsResult, list]:
        """Run a capability test."""
        if cli_args is None:
            cli_args = []

        test_spec = {}
        for i, options in enumerate(cases):
            case = self.CapabilityTestCase(*options)
            test_name = f"test{i}"
            test_spec[test_name] = {
                "type": "RunApp",
                "input": "unused",
                "should_execute": False,
                "capabilities": f'"{case.capabilities}"',
            }
            if case.params:
                test_spec[test_name].update(case.params)

        result = self.runTests(
            *cli_args,
            tests=test_spec,
            minimal_capabilities=False,
            exit_code=exit_code,
        )
        harness = result.harness
        assert harness is not None

        jobs = []
        for i, options in enumerate(cases):
            case = self.CapabilityTestCase(*options)
            job = self.getJobWithName(harness, f"test{i}")
            self.assertEqual(job.getStatus(), job.skip if case.skip else job.finished)
            jobs.append(job)

        return result, jobs

    def test(self):
        """Test filtering tests using capabilities."""
        harness = self.runTests(run=False).harness
        assert harness is not None
        app_capabilities = harness.options._capabilities.values

        # Capability matched
        self.runCapabilityTest(
            (f"compiler={app_capabilities['compiler']['value']}", False)
        )

        # Capability not matched
        _, jobs = self.runCapabilityTest(("compiler=unknown", True))
        self.assertIn("Need compiler=unknown", jobs[0].getTester().getCaveats())

        # Bad parse, with error message in Tester status
        result, jobs = self.runCapabilityTest(("!", True), exit_code=132)
        self.assertEqual(jobs[0].getTester().getStatus(), jobs[0].error)
        self.assertEqual(jobs[0].getTester().getStatusMessage(), "INVALID CAPABILITIES")
        message = (
            f"{result.test_spec_file}:6:\n"
            "Tests/test0/capabilities:\n"
            "Unable to parse requested capabilities '!'."
        )
        self.assertIn(message, jobs[0].getTester().getOutput())

    def testAugmented(self):
        """Test filtering tests using augmented application capabilities."""

        def run(*cases: tuple, cli_args: Optional[list[str]] = None):
            if cli_args is None:
                cli_args = []

            test_spec = {}
            for i, options in enumerate(cases):
                case = self.CapabilityTestCase(*options)
                test_name = f"test{i}"
                test_spec[test_name] = {
                    "type": "RunApp",
                    "input": "unused",
                    "should_execute": False,
                    "capabilities": f'"{case.capabilities}"',
                }
                if case.params:
                    test_spec[test_name].update(case.params)

            result = self.runTests(
                *cli_args, tests=test_spec, minimal_capabilities=True
            )
            harness = result.harness
            assert harness is not None

            for i, options in enumerate(cases):
                case = self.CapabilityTestCase(*options)
                job = self.getJobWithName(harness, f"test{i}")
                self.assertEqual(
                    job.getStatus(), job.skip if case.skip else job.finished
                )

        # Augmented capability from TestHarness
        bad_machine = "arm64" if getMachine() == "x86_64" else "arm64"
        self.runCapabilityTest(
            (f"machine={bad_machine}", True), (f"machine={getMachine()}", False)
        )

        # Augmented capability from Tester (RunApp)
        self.runCapabilityTest(("mpi_procs=1", False), ("mpi_procs=2", True))

    def testDeprecatedParams(self):
        """Test that using a deprecated, now capability param reports an error."""
        for param, capability in Tester.capability_params:
            test_spec = {
                "test": {
                    "type": "RunApp",
                    "input": "unused",
                    "should_execute": False,
                    param: "foo",
                }
            }
            result = self.runTests(
                tests=test_spec, exit_code=128, minimal_capabilities=True
            )
            message = (
                f"{result.test_spec_file}:2: Failed to create Tester: The following "
                "parameters are deprecated and should use the 'capability' param "
                f"instead; '{param}'"
            )
            if capability:
                message += f" -> capability '{capability}'"
            self.assertIn(message, result.output)


if __name__ == "__main__":
    unittest.main()
