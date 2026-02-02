# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

"""Test the augmentation of capabilities from the run time options."""

import unittest
from argparse import Namespace
from copy import deepcopy
from dataclasses import dataclass
from typing import Any, Optional, Tuple

from TestHarnessTestCase import TestHarnessTestCase

from TestHarness import TestHarness
from TestHarness.util import getMachine, getPlatform


class TestAugmentedCapabilities(TestHarnessTestCase):
    """Test the augmentation of capabilities from the run time options."""

    def testGetCapabilitiesMinimal(self):
        """Test TestHarness.getCapabilities() with --minimal-capabilities."""
        default_options = Namespace()
        default_options.minimal_capabilities = True
        default_options.scaling = None
        default_options.valgrind_mode = ""
        default_options.enable_recover = None
        default_options.enable_restep = None
        default_options.all_tests = None
        default_options.heavy_tests = None
        default_options.compute_device = None
        default_options.only_tests_that_require = None

        def test_get_capabilities(
            check_capabilities: list[Tuple[str, Any]],
            check_required_capabilities: Optional[list[Tuple[str, bool]]] = None,
            **kwargs,
        ):
            options = deepcopy(default_options)
            for key, value in kwargs.items():
                setattr(options, key, value)

            capabilities, required_capabilities = TestHarness.getCapabilities(
                options, None, None
            )
            self.assertIsInstance(capabilities, dict)
            self.assertIsInstance(required_capabilities, list)
            for capability, value in check_capabilities:
                self.assertIn(capability, capabilities)
                entry = capabilities[capability]
                self.assertIsInstance(entry, list)
                self.assertIsInstance(entry[1], str)
                self.assertEqual(entry[0], value)
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
            ("scale_refine", False),
            ("valgrind", False),
            ("recover", False),
            ("restep", False),
            ("heavy", False),
            ("compute_device", False),
            ("machine", getMachine()),
            ("platform", getPlatform()),
            ("installation_type", "in_tree"),
        ]
        test_get_capabilities(check_capabilities)

        # -s, --scale
        test_get_capabilities([("scale_refine", True)], scaling=True)

        # --valgrind, --valgrind-heavy
        test_get_capabilities([("valgrind", "normal")], valgrind_mode="NORMAL")
        test_get_capabilities([("valgrind", "heavy")], valgrind_mode="HEAVY")

        # --recover
        test_get_capabilities([("recover", True)], enable_recover=True)

        # --restep
        test_get_capabilities([("restep", True)], enable_restep=True)

        # --all-tests or --heavy
        test_get_capabilities([("heavy", True)], all_tests=True)
        test_get_capabilities([("heavy", True)], heavy_tests=True)

        # --compute-device
        test_get_capabilities([("compute_device", "foo")], compute_device="foo")

        # --only-tests-that-require
        test_get_capabilities(
            [],
            check_required_capabilities=[("platform", False)],
            only_tests_that_require="platform",
        )
        test_get_capabilities(
            [],
            check_required_capabilities=[("platform", True)],
            only_tests_that_require="!platform",
        )

    def test(self):
        """Test the augmentation of capabilities."""

        @dataclass
        class TestCase:
            capabilities: str
            skip: bool
            params: Optional[dict] = None

        def run(*cases: tuple, cli_args: Optional[list[str]] = []) -> dict:
            test_spec = {}
            for i, options in enumerate(cases):
                case = TestCase(*options)
                test_name = f"test{i}"
                test_spec[test_name] = {
                    "type": "RunApp",
                    "input": "unused",
                    "should_execute": False,
                    "capabilities": f'"platform & {case.capabilities}"',
                }
                if case.params:
                    test_spec[test_name].update(case.params)

            result = self.runTests(
                *cli_args, tests=test_spec, minimal_capabilities=True
            )
            harness = result.harness

            for i, options in enumerate(cases):
                case = TestCase(*options)
                test_name = f"test{i}"
                job = [
                    j
                    for j in harness.finished_jobs
                    if j.getTestNameShort() == test_name
                ][0]
                self.assertEqual(
                    job.getStatus(), job.skip if case.skip else job.finished
                )

        # Basic choices that match a command line option
        for option in ["valgrind", "recover", "heavy", "restep"]:
            # Option isn't set: capability '!option' will be ran and capability 'option' won't
            run((f"!{option}", False), (option, True))
            # Option is set: capability 'option' will be ran and capability '!option' won't
            params = {"heavy": True} if option == "heavy" else {}
            run(
                (f"{option}", False, params),
                (f"!{option}", True, params),
                cli_args=[f"--{option}"],
            )

        # Machine
        run(("machine=foo", True), (f"machine={getMachine()}", False))

        # Platform
        run(("platform=foo", True), (f"platform={getPlatform()}", False))

        # Installation type
        run(("installation_type=installed", True), ("installation_type=in_tree", False))

        # MPI procs
        run(("mpi_procs>1", True), ("mpi_procs=1", False))
        run(("mpi_procs>1", False), ("mpi_procs=1", True), cli_args=["-p", "2"])

        # Num threads
        run(("num_threads>1", True), ("num_threads=1", False))
        run(
            ("num_threads>1", False),
            ("num_threads=1", True),
            cli_args=["--n-threads", "2"],
        )

        # Device
        run(("compute_device=cpu", False), ("compute_device=foo", True))


if __name__ == "__main__":
    unittest.main()
