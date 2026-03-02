# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

"""Test the use of capabilites in the TestHarness."""

import json
import os
import unittest
from argparse import Namespace
from copy import deepcopy
from dataclasses import dataclass
from pathlib import Path
from shlex import quote
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
        from pycapabilities import AUGMENTED_CAPABILITY_NAMES, Capabilities

        capabilities = Capabilities({})
        name = next(iter(AUGMENTED_CAPABILITY_NAMES))

        # Name converts to lowercase
        augmented = {}
        addAugmentedCapability(capabilities, augmented, name.upper(), None, "doc")
        self.assertIn(name, augmented)

        # String value converts to lowercase
        augmented = {}
        addAugmentedCapability(capabilities, augmented, name, "FOO", "doc")
        self.assertEqual(augmented[name]["value"], "foo")

        # Enumeration can only be used for string capabilities
        with self.assertRaisesRegex(
            AssertionError, "Enumeration only valid for str capabilities"
        ):
            addAugmentedCapability(
                capabilities, {}, "foo", True, "doc", enumeration=["foo"]
            )
        with self.assertRaisesRegex(
            AssertionError, "Enumeration only valid for str capabilities"
        ):
            addAugmentedCapability(
                capabilities, {}, "foo", 0, "doc", enumeration=["foo"]
            )

        # Enumeration converted to lower
        augmented = {}
        addAugmentedCapability(
            capabilities, augmented, name, "value", "doc", enumeration=["VALUE"]
        )
        self.assertEqual(augmented[name]["enumeration"], ["value"])

        # Not an augmented capability
        with self.assertRaisesRegex(
            ValueError, "Capability foo is not a registered augmented capability name"
        ):
            addAugmentedCapability(set("foo"), {}, "foo", None, "doc")

        # Name already exists in main capabilities
        with self.assertRaisesRegex(
            ValueError, rf"Capability {name} is defined by the app"
        ):
            addAugmentedCapability(AUGMENTED_CAPABILITY_NAMES, {}, name, None, "doc")

        # Name already exists in augmented capabilities
        with self.assertRaisesRegex(
            ValueError,
            rf"Capability {name} is already defined as an augmented capability.",
        ):
            addAugmentedCapability(capabilities, {name: None}, name, None, "doc")

        # Build a bool value
        for value, expected_value in [(None, False), (False, False), (True, True)]:
            augmented = {}
            addAugmentedCapability(capabilities, augmented, name, value, "somedoc")
            self.assertEqual(
                augmented, {name: {"doc": "somedoc", "value": expected_value}}
            )

        # Build a int value
        for explicit in [None, False, True]:
            augmented = {}
            kwargs = {}
            if explicit is not None:
                kwargs["explicit"] = explicit
            addAugmentedCapability(
                capabilities, augmented, name, 1, "somedoc", **kwargs
            )
            expected = {name: {"doc": "somedoc", "value": 1, **kwargs}}
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
                addAugmentedCapability(
                    capabilities, augmented, name, "bar", "somedoc", **kwargs
                )
                expected = {name: {"doc": "somedoc", "value": "bar", **kwargs}}
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
                TestHarness.getCapabilities(options, None)
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

        # Test not filtering due to --ignore: don't skip and don't even check
        _, jobs = self.runCapabilityTest(
            ("compiler=unknown", False), cli_args=["--ignore"]
        )
        self.assertNotIn("Need compiler=unknown", jobs[0].getTester().getCaveats())

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
        bad_machine = "arm64" if getMachine() == "x86_64" else "x86_64"
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

    def testRequiredCapabilities(self):
        """Test that required and augmented capabilities are set in a RunApp run."""

        def post_run(
            tmp_dir: bool,
            test_name: str,
            has_augmented: bool,
            capabilities: str,
            result: TestHarnessTestCase.RunTestsResult,
        ):
            assert result.harness is not None
            job = self.getJobWithName(result.harness, test_name)

            # Should have ran the app with the given capabilities
            cmd_ran = job.getTester().getCommandRan()
            self.assertIn(f"--required-capabilities={quote(capabilities)}", cmd_ran)

            capabilities_file = job.getTester().getCapabilitiesFilePath(job.options)

            # No augmented capabilities, shouldn't have a capabilities file
            if not has_augmented:
                self.assertFalse(os.path.exists(capabilities_file))
                return

            # Augmented capabilities file was written, check for it
            capabilities_file = job.getTester().getCapabilitiesFilePath(job.options)
            self.assertTrue(os.path.isfile(capabilities_file))
            if tmp_dir:
                assert result.tmp_dir is not None
                self.assertTrue(
                    Path(capabilities_file).is_relative_to(Path(result.tmp_dir))
                )
            else:
                self.assertTrue(
                    Path(capabilities_file).is_relative_to(job.getTestDir())
                )
            with open(capabilities_file, "r") as f:
                capabilities = json.load(f)
            # Should only contain those two capabilities that
            # we used if we did this right
            self.assertTrue(len(capabilities), 2)
            self.assertIn("mpi_procs", capabilities)
            self.assertIn("machine", capabilities)

            # And should have ran the app with the given capabilities
            # and the augmented capabilities file
            cmd_ran = job.getTester().getCommandRan()
            self.assertIn(
                (
                    "--required-capabilities='moosetestapp & machine!=unknown "
                    "& mpi_procs=1'"
                ),
                cmd_ran,
            )

        # Run with capabilities from the app, from the TestHarness,
        # and from RunApp, with the augmented output stored in
        # the same directory as the test
        self.runTests(
            "-i",
            "capabilities",
            "--re",
            "has_augmented",
            minimal_capabilities=False,
            post_run=lambda result: post_run(
                tmp_dir=False,
                test_name="has_augmented",
                has_augmented=True,
                capabilities="moosetestapp & machine!=unknown & mpi_procs=1",
                result=result,
            ),
            tmp_output=False,
        )

        # Run with capabilities from the app, from the TestHarness,
        # and from RunApp, with the augmented output stored in
        # a different directory
        self.runTests(
            "-i",
            "capabilities",
            "--re",
            "has_augmented",
            minimal_capabilities=False,
            post_run=lambda result: post_run(
                tmp_dir=True,
                test_name="has_augmented",
                has_augmented=True,
                capabilities="moosetestapp & machine!=unknown & mpi_procs=1",
                result=result,
            ),
        )

        # Run with capabilities from the app, from the TestHarness,
        # and from RunApp, with the augmented output stored in
        # a different directory
        self.runTests(
            "-i",
            "capabilities",
            "--re",
            "no_augmented",
            minimal_capabilities=False,
            post_run=lambda result: post_run(
                tmp_dir=True,
                test_name="no_augmented",
                has_augmented=False,
                capabilities="moosetestapp",
                result=result,
            ),
        )


if __name__ == "__main__":
    unittest.main()
