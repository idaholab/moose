# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

"""Unit tests covering the Moose2FMU base class behavior."""

import logging
import unittest
from types import MethodType
from unittest import mock

from moosefmu import Moose2FMU


class _DummyMoose(Moose2FMU):
    """Minimal subclass used for testing the Moose2FMU base logic."""

    def do_step(
        self,
        current_time: float,
        step_size: float,
        no_set_fmu_state_prior: bool = False,
    ) -> bool:
        return True


class TestMoose2FMU(unittest.TestCase):
    """Validate Moose2FMU behavior through a lightweight subclass."""

    def test_logger_uses_module_hierarchy(self):
        """Logger name should reflect the module hierarchy."""
        slave = _DummyMoose(instance_name="test", guid="1234")
        expected = f"{_DummyMoose.__module__}.{_DummyMoose.__name__}"
        self.assertEqual(slave.logger.name, expected)

    def test_get_flag_with_retries_success(self):
        """Flag retrieval should succeed when available within retries."""
        slave = _DummyMoose(instance_name="test", guid="1234")

        class Control:
            def __init__(self):
                self.calls = 0

            def get_waiting_flag(self):
                self.calls += 1
                return " READY " if self.calls >= 2 else None

        slave.control = Control()

        with self.assertLogs(slave.logger, level=logging.INFO) as logs:
            result = slave.get_flag_with_retries(
                {"READY"}, max_retries=5, wait_seconds=0
            )

        self.assertEqual(result, " READY ")
        self.assertEqual(slave.control.calls, 2)
        self.assertIn(
            "Successfully got flag ' READY ' after 1 retries.",
            "\n".join(logs.output),
        )

    def test_get_flag_with_retries_skips_unexpected_flags(self):
        """Unexpected flags should be skipped until an allowed value arrives."""
        slave = _DummyMoose(instance_name="test", guid="1234")

        calls = {"skip": [], "waiting": 0}

        class Control:
            def __init__(self):
                self._responses = iter(["foo", "READY"])

            def get_waiting_flag(self):
                calls["waiting"] += 1
                return next(self._responses)

        slave.control = Control()

        def fake_skip(self, flag):
            calls["skip"].append(flag)

        slave._skip_flag = MethodType(fake_skip, slave)

        result = slave.get_flag_with_retries({"READY"}, max_retries=3, wait_seconds=0)

        self.assertEqual(result, "READY")
        self.assertEqual(calls["waiting"], 2)
        self.assertEqual(calls["skip"], ["foo"])

    def test_get_flag_with_retries_failure(self):
        """Retry loop should fail gracefully when no flag is received."""
        slave = _DummyMoose(instance_name="test", guid="1234")

        class Control:
            def __init__(self):
                self.calls = 0

            def get_waiting_flag(self):
                self.calls += 1
                return None

        slave.control = Control()

        with self.assertLogs(slave.logger, level=logging.ERROR) as logs:
            result = slave.get_flag_with_retries(
                {"READY"}, max_retries=3, wait_seconds=0
            )

        self.assertIsNone(result)
        self.assertEqual(slave.control.calls, 3)
        self.assertIn(
            "Failed to get one of ['READY'] after 3 retries.",
            "\n".join(logs.output),
        )

    def test_get_flag_with_retries_handles_exception(self):
        """Transient exceptions should be logged and retried."""
        slave = _DummyMoose(instance_name="test", guid="1234")

        class Control:
            def __init__(self):
                self.calls = 0

            def get_waiting_flag(self):
                self.calls += 1
                if self.calls < 2:
                    raise RuntimeError("temporary failure")
                return "OK"

        slave.control = Control()

        with self.assertLogs(slave.logger, level=logging.WARNING) as logs:
            result = slave.get_flag_with_retries({"OK"}, max_retries=4, wait_seconds=0)

        self.assertEqual(result, "OK")
        self.assertEqual(slave.control.calls, 2)
        self.assertIn("Attempt 1/4 failed", "\n".join(logs.output))

    def test_set_controllable_real_caches_values(self):
        """Real controllables should cache values and honor force flag."""
        slave = _DummyMoose(instance_name="test", guid="1234")

        class Control:
            def __init__(self):
                self.set_calls = []
                self.continue_calls = 0

            def set_real(self, path, value):
                self.set_calls.append((path, value))

            def set_continue(self):
                self.continue_calls += 1

        slave.control = Control()

        self.assertTrue(slave.set_controllable_real("alpha", 1.0))
        self.assertEqual(slave.control.set_calls, [("alpha", 1.0)])
        self.assertEqual(slave.control.continue_calls, 1)

        self.assertFalse(slave.set_controllable_real("alpha", 1.0))
        self.assertEqual(slave.control.set_calls, [("alpha", 1.0)])

        self.assertTrue(slave.set_controllable_real("alpha", 1.0, force=True))
        self.assertEqual(slave.control.set_calls, [("alpha", 1.0), ("alpha", 1.0)])

    def test_set_controllable_vector_infers_and_caches(self):
        """Vector controllables should infer types and avoid repeats."""
        slave = _DummyMoose(instance_name="test", guid="1234")

        class Control:
            def __init__(self):
                self.calls = []
                self.continue_calls = 0

            def set_vector_real(self, path, value):
                self.calls.append(("real", path, value))

            def set_vector_int(self, path, value):
                self.calls.append(("int", path, value))

            def set_vector_string(self, path, value):
                self.calls.append(("string", path, value))

            def set_continue(self):
                self.continue_calls += 1

        slave.control = Control()

        self.assertTrue(slave.set_controllable_vector("vec", [1.0, 2, 3.5]))
        self.assertEqual(
            slave.control.calls,
            [("real", "vec", [1.0, 2.0, 3.5])],
        )
        self.assertEqual(slave.control.continue_calls, 1)

        # Cached call should be skipped
        self.assertFalse(slave.set_controllable_vector("vec", [1, 2.0, 3.5]))
        self.assertEqual(
            slave.control.calls,
            [("real", "vec", [1.0, 2.0, 3.5])],
        )

        # Changing type should trigger new setter and cache entry
        self.assertTrue(
            slave.set_controllable_vector("vec", [1, 2, 3], value_type="int")
        )
        self.assertEqual(
            slave.control.calls,
            [
                ("real", "vec", [1.0, 2.0, 3.5]),
                ("int", "vec", [1, 2, 3]),
            ],
        )

        self.assertTrue(slave.set_controllable_vector("names", ("a", "b")))
        self.assertEqual(
            slave.control.calls[-1],
            ("string", "names", ["a", "b"]),
        )
        self.assertEqual(slave.control.continue_calls, 3)

    def test_set_controllable_vector_validation(self):
        """Invalid inputs should raise helpful exceptions."""
        slave = _DummyMoose(instance_name="test", guid="1234")

        class Control:
            def set_vector_real(self, path, value):
                pass

            def set_vector_int(self, path, value):
                pass

            def set_vector_string(self, path, value):
                pass

            def set_continue(self):
                pass

        slave.control = Control()

        with self.assertRaises(TypeError):
            slave.set_controllable_vector("bad", "not-a-vector")

        with self.assertRaises(ValueError):
            slave.set_controllable_vector("empty", [], value_type=None)

        with self.assertRaises(TypeError):
            slave.set_controllable_vector("mixed", [1, "two"], value_type=None)

        with self.assertRaises(TypeError):
            slave.set_controllable_vector("vec", [1, 2], value_type="complex")

        # Explicit type should allow empty sequences
        self.assertTrue(slave.set_controllable_vector("empty", [], value_type="real"))

    def test_set_controllable_real_waits_for_flag(self):
        """Real updates should wait for synchronization flags when requested."""
        slave = _DummyMoose(instance_name="test", guid="1234")

        class Control:
            def __init__(self):
                self.waited = []
                self.set_calls = []
                self.continue_calls = 0

            def wait(self, flag):
                self.waited.append(flag)

            def set_real(self, path, value):
                self.set_calls.append((path, value))

            def set_continue(self):
                self.continue_calls += 1

        slave.control = Control()

        captured = {}

        def fake_get_flag(self, allowed_flags, max_retries, wait_seconds=0.5):
            captured["allowed"] = set(allowed_flags)
            return "CUSTOM"

        slave.get_flag_with_retries = MethodType(fake_get_flag, slave)

        self.assertTrue(slave.set_controllable_real("alpha", 2.0, flag="custom"))
        self.assertEqual(slave.control.waited, ["CUSTOM"])
        self.assertEqual(slave.control.set_calls, [("alpha", 2.0)])
        self.assertEqual(slave.control.continue_calls, 1)
        self.assertEqual(captured["allowed"], {"CUSTOM"})

    def test_set_controllable_real_uses_user_defined_flag(self):
        """User-defined flags should be honored when sending real values."""
        slave = _DummyMoose(instance_name="test", guid="1234")
        slave.flag = "user_flag"

        class Control:
            def __init__(self):
                self.waited = []
                self.set_calls = []

            def wait(self, flag):
                self.waited.append(flag)

            def set_real(self, path, value):
                self.set_calls.append((path, value))

            def set_continue(self):
                pass

        slave.control = Control()
        test_self = self

        def fake_get_flag(self, allowed_flags, max_retries, wait_seconds=0.5):
            test_self.assertEqual(set(allowed_flags), {"USER_FLAG"})
            return "USER_FLAG"

        slave.get_flag_with_retries = MethodType(fake_get_flag, slave)

        self.assertTrue(slave.set_controllable_real("beta", 5.0))
        self.assertEqual(slave.control.waited, ["USER_FLAG"])
        self.assertEqual(slave.control.set_calls, [("beta", 5.0)])

    def test_get_postprocessor_value_merges_flags(self):
        """Flag sets from defaults and user input should merge before waiting."""
        slave = _DummyMoose(instance_name="test", guid="1234")
        slave.flag = "additional"

        class Control:
            def __init__(self):
                self.waited = []

            def wait(self, flag):
                self.waited.append(flag)

            def get_postprocessor(self, name):
                return 7.0

            def finalize(self):
                raise AssertionError("finalize should not be called")

        slave.control = Control()

        captured = {}

        def fake_get_flag(self, allowed_flags, max_retries, wait_seconds=0.5):
            captured["allowed"] = set(allowed_flags)
            return "CUSTOM"

        slave.get_flag_with_retries = MethodType(fake_get_flag, slave)

        value = slave.get_postprocessor_value("energy", 3.0, flag="custom")

        self.assertEqual(value, 7.0)
        self.assertEqual(slave.control.waited, ["CUSTOM"])
        self.assertEqual(
            captured["allowed"],
            {"MULTIAPP_FIXED_POINT_END", "CUSTOM", "ADDITIONAL"},
        )

    def test_parse_flags_handles_strings_and_iterables(self):
        """Flag parsing should support delimited strings and iterables."""
        slave = _DummyMoose(instance_name="test", guid="1234")

        parsed = slave._parse_flags("initial, timestep_begin;custom | Another")
        self.assertEqual(parsed, {"INITIAL", "TIMESTEP_BEGIN", "CUSTOM", "ANOTHER"})

        parsed_iterable = slave._parse_flags(["Ready", " done ", ""])
        self.assertEqual(parsed_iterable, {"READY", "DONE"})

    @mock.patch("moosefmu.moose2fmu.MooseControl")
    def test_exit_initialization_mode_rebuilds_command(self, mock_control):
        """exit_initialization_mode should rebuild the configured command."""
        slave = _DummyMoose(instance_name="test", guid="1234")

        slave.moose_command = "mpiexec -n 4 /custom/moose-opt -i custom_input.i"

        self.assertTrue(slave.exit_initialization_mode())

        expected_command = [
            "mpiexec",
            "-n",
            "4",
            "/custom/moose-opt",
            "-i",
            "custom_input.i",
        ]

        mock_runner.assert_called_once_with(
            command=expected_command,
            moose_control_name=slave.server_name,
        )
        mock_control.assert_called_once()
        args, kwargs = mock_control.call_args
        self.assertEqual(args[0], mock_runner.return_value)
        mock_control.return_value.initialize.assert_called_once_with()

    def test_sync_with_moose_success(self):
        """sync_with_moose should align times and return the trigger flag."""
        slave = _DummyMoose(instance_name="test", guid="1234")

        class Runner:
            def __init__(self):
                self._running_checks = iter([True, True])

            def is_process_running(self):
                try:
                    return next(self._running_checks)
                except StopIteration:
                    return False

        class Control:
            def __init__(self):
                self._times = iter([0.0, 1.0005])
                self.wait_calls = []
                self.continue_calls = 0
                self.runner = Runner()


            def get_time(self):
                return next(self._times)

            def wait(self, flag):
                self.wait_calls.append(flag)

            def set_continue(self):
                self.continue_calls += 1

            def get_dt(self):
                return 5e-4

        slave.control = Control()

        flags = iter(["INITIAL", "TIMESTEP_BEGIN"])

        def fake_get_flag(self, allowed_flags, max_retries, wait_seconds=0.5):
            return next(flags)

        slave.get_flag_with_retries = MethodType(fake_get_flag, slave)

        moose_time, signal = slave.sync_with_moose(1.0, 0.1)

        self.assertEqual(signal, "INITIAL")
        self.assertAlmostEqual(moose_time, 1.0005, delta=1e-3)
        self.assertEqual(slave.control.wait_calls, ["INITIAL"])
        self.assertEqual(slave.control.continue_calls, 1)

    def test_ensure_control_listening(self):
        """Control listening status should be checked before continuing."""
        slave = _DummyMoose(instance_name="test", guid="1234")

        class Runner:
            def __init__(self, listening: bool):
                self._listening = listening

            def is_listening(self):
                return self._listening

        class Control:
            def __init__(self, listening: bool):
                self.runner = Runner(listening)
                self.finalized = False

            def finalize(self):
                self.finalized = True

        slave.control = Control(True)
        self.assertTrue(slave.ensure_control_listening())
        self.assertFalse(slave.control.finalized)

        slave.control = Control(False)
        self.assertFalse(slave.ensure_control_listening())
        self.assertTrue(slave.control.finalized)

    def test_setup_experiment_saves_parameters(self):
        """setup_experiment should store inputs and report success."""
        slave = _DummyMoose(instance_name="test", guid="1234")

        result = slave.setup_experiment(start_time=0.5, stop_time=10.0, tolerance=1e-4)

        self.assertTrue(result)
        self.assertEqual(slave.start_time, 0.5)
        self.assertEqual(slave.stop_time, 10.0)
        self.assertEqual(slave.tolerance, 1e-4)

    def test_get_postprocessor_value(self):
        """Postprocessors should be retrievable when flags are received."""
        slave = _DummyMoose(instance_name="test", guid="1234")

        class Control:
            def __init__(self):
                self.waited = []
                self.continue_calls = 0
                self.finalized = False

            def wait(self, flag):
                self.waited.append(flag)

            def get_postprocessor(self, name):
                return {"temp": 42.0}[name]

            def set_continue(self):
                self.continue_calls += 1

            def finalize(self):
                self.finalized = True

        slave.control = Control()

        def fake_get_flag(self, allowed_flags, max_retries, wait_seconds=0.5):
            return "READY"

        slave.get_flag_with_retries = MethodType(fake_get_flag, slave)

        value = slave.get_postprocessor_value("temp", 1.0, flag="READY")

        self.assertEqual(value, 42.0)
        self.assertEqual(slave.control.waited, ["READY"])
        self.assertEqual(slave.control.continue_calls, 0)
        self.assertFalse(slave.control.finalized)

    def test_get_postprocessor_value_failure(self):
        """Failed flag waits should finalize control and return None."""
        slave = _DummyMoose(instance_name="test", guid="1234")

        class Control:
            def __init__(self):
                self.finalized = False

            def finalize(self):
                self.finalized = True

        slave.control = Control()

        def fake_get_flag(self, allowed_flags, max_retries, wait_seconds=0.5):
            return None

        slave.get_flag_with_retries = MethodType(fake_get_flag, slave)

        result = slave.get_postprocessor_value("temp", 5.0, flag="READY")

        self.assertIsNone(result)
        self.assertTrue(slave.control.finalized)

    def test_get_reporter_value(self):
        """Reporter values should be fetched after synchronization."""
        slave = _DummyMoose(instance_name="test", guid="1234")

        class Control:
            def __init__(self):
                self.waited = []
                self.continue_calls = 0
                self.finalized = False

            def wait(self, flag):
                self.waited.append(flag)

            def get_reporter(self, name):
                return {"flux": 3.14}[name]

            def set_continue(self):
                self.continue_calls += 1

            def finalize(self):
                self.finalized = True

        slave.control = Control()

        def fake_get_flag(self, allowed_flags, max_retries, wait_seconds=0.5):
            return "READY"

        slave.get_flag_with_retries = MethodType(fake_get_flag, slave)

        value = slave.get_reporter_value("flux", 2.0, flag="READY")

        self.assertEqual(value, 3.14)
        self.assertEqual(slave.control.waited, ["READY"])
        self.assertEqual(slave.control.continue_calls, 0)
        self.assertFalse(slave.control.finalized)


if __name__ == "__main__":
    unittest.main()
