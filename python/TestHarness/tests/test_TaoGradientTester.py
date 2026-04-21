# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

"""Unit tests for the TaoGradientTester tester class."""

import os
import sys
import unittest
from types import SimpleNamespace

from TestHarnessTestCase import MOOSE_DIR, MOOSE_PYTHON

TESTER_DIR = os.path.join(MOOSE_PYTHON, "TestHarness", "testers")
if TESTER_DIR not in sys.path:
    sys.path.append(TESTER_DIR)

from TaoGradientTester import TaoGradientTester


def _make_params(**overrides):
    """Build a valid set of params for constructing a TaoGradientTester."""
    params = TaoGradientTester.validParams()
    # Required / expected params that Tester.__init__ reads
    params["type"] = "TaoGradientTester"
    params["test_name"] = "test_dir.test_name"
    params["test_name_short"] = "test_name"
    params["input"] = "input.i"
    params["executable"] = "/path/to/moose_test-opt"
    params["moose_dir"] = MOOSE_DIR
    params["moose_python_dir"] = MOOSE_PYTHON
    params["test_dir"] = "/tmp/test"
    params["spec_file"] = "tests"
    params["unique_test_id"] = "abc123"
    params["min_reported_time"] = 0
    # Private param normally set by Parser via Tester.augmentParams()
    params.addPrivateParam("_validation_classes", [])
    for key, value in overrides.items():
        params[key] = value
    return params


def _make_options():
    """Create a minimal mock options namespace for getCommand / processResults."""
    return SimpleNamespace(
        parallel=None,
        nthreads=1,
        distributed_mesh=False,
        error=False,
        error_unused=False,
        allow_unused=False,
        error_deprecated=False,
        valgrind_mode="",
        enable_recover=False,
        enable_restep=False,
        timing=False,
        scaling=False,
        colored=True,
        cli_args=None,
        hpc=False,
        hpc_srun=False,
        method="opt",
        compute_device="cpu",
        capture_perf_graph=False,
        append_runapp_cliarg=None,
        min_parallel=None,
        min_threads=None,
    )


class TestTaoGradientTesterValidParams(unittest.TestCase):
    """Test that validParams sets appropriate defaults."""

    def test_default_max_rel_tol(self):
        params = TaoGradientTester.validParams()
        self.assertEqual(params["max_rel_tol"], 1e-5)

    def test_default_tao_fd_delta_is_none(self):
        params = TaoGradientTester.validParams()
        self.assertIsNone(params["tao_fd_delta"])

    def test_valgrind_disabled(self):
        params = TaoGradientTester.validParams()
        self.assertEqual(params["valgrind"], "NONE")

    def test_recover_disabled(self):
        params = TaoGradientTester.validParams()
        self.assertFalse(params["recover"])

    def test_restep_disabled(self):
        params = TaoGradientTester.validParams()
        self.assertFalse(params["restep"])

    def test_max_threads_is_one(self):
        params = TaoGradientTester.validParams()
        self.assertEqual(params["max_threads"], 1)


class TestTaoGradientTesterGetCommand(unittest.TestCase):
    """Test that getCommand appends the correct PETSc/TAO CLI options."""

    def _get_command(self, **param_overrides):
        params = _make_params(**param_overrides)
        tester = TaoGradientTester("test", params)
        options = _make_options()
        return tester.getCommand(options)

    def test_tao_max_it(self):
        cmd = self._get_command()
        self.assertIn("Executioner/tao_solver=taobncg", cmd)

    def test_petsc_options_iname(self):
        cmd = self._get_command()
        self.assertIn("Executioner/petsc_options_iname=", cmd)
        self.assertIn("-tao_max_it", cmd)
        self.assertIn("-tao_fd_test", cmd)
        self.assertIn("-tao_test_gradient", cmd)
        self.assertIn("-tao_fd_gradient", cmd)
        self.assertIn("-tao_ls_type", cmd)

    def test_petsc_options_value(self):
        cmd = self._get_command()
        self.assertIn("Executioner/petsc_options_value=", cmd)
        self.assertIn("1 true true false unit", cmd)

    def test_petsc_options_view(self):
        cmd = self._get_command()
        self.assertIn("Executioner/petsc_options='-tao_test_gradient_view'", cmd)

    def test_verbose_enabled(self):
        cmd = self._get_command()
        self.assertIn("Executioner/verbose=true", cmd)

    def test_tao_fd_delta_not_present_by_default(self):
        cmd = self._get_command()
        self.assertNotIn("-tao_fd_delta", cmd)

    def test_tao_fd_delta_included_when_set(self):
        cmd = self._get_command(tao_fd_delta=1e-8)
        self.assertIn("-tao_fd_delta", cmd)
        self.assertIn("1e-08", cmd)


class TestTaoGradientTesterProcessResults(unittest.TestCase):
    """Test that processResults correctly parses TAO gradient test output."""

    def _process(self, runner_output, max_rel_tol=1e-5, exit_code=0):
        params = _make_params(max_rel_tol=max_rel_tol)
        tester = TaoGradientTester("test", params)
        options = _make_options()
        output = tester.processResults(MOOSE_DIR, options, exit_code, runner_output)
        return tester, output

    # -- Passing cases --

    def test_passing_gradient(self):
        """A max-norm below tolerance should pass."""
        runner_output = (
            "Taylor test  max-norm ||G - Gfd||/||G|| = 1.23456e-08, "
            "max-norm ||G - Gfd|| = 4.56789e-10"
        )
        tester, output = self._process(runner_output)
        self.assertFalse(tester.isFail())
        self.assertIn("successful", output)
        self.assertIn("1.23e-08", output)

    def test_passing_at_tolerance_boundary(self):
        """A max-norm exactly at tolerance should pass (not strictly greater)."""
        runner_output = (
            "max-norm ||G - Gfd||/||G|| = 1.00000e-05, ||G - Gfd|| = 1.0e-06"
        )
        tester, output = self._process(runner_output, max_rel_tol=1e-5)
        self.assertFalse(tester.isFail())

    def test_custom_tolerance(self):
        """Should pass when value is below a custom tolerance."""
        runner_output = "max-norm ||G - Gfd||/||G|| = 5.0e-03, ||G - Gfd|| = 1.0e-04"
        tester, output = self._process(runner_output, max_rel_tol=1e-2)
        self.assertFalse(tester.isFail())

    # -- Failing cases --

    def test_failing_gradient_too_large(self):
        """A max-norm above tolerance should fail with MAX-NORM TOO LARGE."""
        runner_output = "max-norm ||G - Gfd||/||G|| = 2.5e-02, ||G - Gfd|| = 1.0e-03"
        tester, output = self._process(runner_output, max_rel_tol=1e-5)
        self.assertTrue(tester.isFail())
        self.assertIn("too large", output)
        self.assertIn("2.50e-02", output)

    def test_missing_output(self):
        """If expected pattern is not in output, should fail with EXPECTED OUTPUT NOT FOUND."""
        runner_output = "Some unrelated output with no gradient info"
        tester, output = self._process(runner_output)
        self.assertTrue(tester.isFail())
        self.assertIn("not found", output)

    def test_empty_output(self):
        """Empty output should fail."""
        tester, output = self._process("")
        self.assertTrue(tester.isFail())
        self.assertIn("not found", output)

    # -- Multiple matches: only the first is used --

    def test_multiple_matches_uses_first(self):
        """Only the first gradient comparison should be checked."""
        runner_output = (
            "max-norm ||G - Gfd||/||G|| = 1.0e-08, ||G - Gfd|| = 1.0e-10\n"
            "max-norm ||G - Gfd||/||G|| = 9.0e-01, ||G - Gfd|| = 1.0e-02\n"
        )
        tester, output = self._process(runner_output)
        # Should pass because the first match (1.0e-08) is below tolerance
        self.assertFalse(tester.isFail())

    def test_multiple_matches_first_fails(self):
        """If the first match fails, test should fail even if later ones pass."""
        runner_output = (
            "max-norm ||G - Gfd||/||G|| = 9.0e-01, ||G - Gfd|| = 1.0e-02\n"
            "max-norm ||G - Gfd||/||G|| = 1.0e-08, ||G - Gfd|| = 1.0e-10\n"
        )
        tester, output = self._process(runner_output)
        self.assertTrue(tester.isFail())

    # -- Regex edge cases --

    def test_trailing_dot_stripped(self):
        """PETSc sometimes outputs values with a trailing dot (e.g., '0.')."""
        runner_output = "max-norm ||G - Gfd||/||G|| = 0., ||G - Gfd|| = 0."
        tester, output = self._process(runner_output)
        self.assertFalse(tester.isFail())


if __name__ == "__main__":
    unittest.main()
