import importlib.util
import logging
import sys
import unittest
from unittest import mock
from pathlib import Path
from types import MethodType

_TEST_DIR = Path(__file__).resolve().parent
_SPEC = importlib.util.spec_from_file_location(
    "MooseFMU.test", _TEST_DIR / "__init__.py"
)
if "MooseFMU.test" not in sys.modules:
    _package_module = importlib.util.module_from_spec(_SPEC)
    sys.modules["MooseFMU.test"] = _package_module
    assert _SPEC.loader is not None
    _SPEC.loader.exec_module(_package_module)

from MooseFMU import Moose2FMU


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
    def test_get_flag_with_retries_success(self):
        slave = _DummyMoose(instance_name="test", guid="1234")

        class Control:
            def __init__(self):
                self.calls = 0

            def getWaitingFlag(self):
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
        slave = _DummyMoose(instance_name="test", guid="1234")

        calls = {"skip": [], "waiting": 0}

        class Control:
            def __init__(self):
                self._responses = iter(["foo", "READY"])

            def getWaitingFlag(self):
                calls["waiting"] += 1
                return next(self._responses)

        slave.control = Control()

        def fake_skip(self, flag):
            calls["skip"].append(flag)


        slave._skip_flag = MethodType(fake_skip, slave)

        result = slave.get_flag_with_retries({"READY"}, max_retries=3, wait_seconds=0)

        def getWaitingFlag(self):
            self.calls += 1
            return None

    def test_get_flag_with_retries_failure(self):
        slave = _DummyMoose(instance_name="test", guid="1234")

        class Control:
            def __init__(self):
                self.calls = 0

            def getWaitingFlag(self):
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
        slave = _DummyMoose(instance_name="test", guid="1234")

        class Control:
            def __init__(self):
                self.calls = 0

            def getWaitingFlag(self):
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
        slave = _DummyMoose(instance_name="test", guid="1234")

        class Control:
            def __init__(self):
                self.set_calls = []
                self.continue_calls = 0

            def setControllableReal(self, path, value):
                self.set_calls.append((path, value))

            def setContinue(self):
                self.continue_calls += 1

        slave.control = Control()

        self.assertTrue(slave.set_controllable_real("alpha", 1.0))
        self.assertEqual(slave.control.set_calls, [("alpha", 1.0)])
        self.assertEqual(slave.control.continue_calls, 1)

        self.assertFalse(slave.set_controllable_real("alpha", 1.0))
        self.assertEqual(slave.control.set_calls, [("alpha", 1.0)])

        self.assertTrue(slave.set_controllable_real("alpha", 1.0, force=True))
        self.assertEqual(
            slave.control.set_calls, [("alpha", 1.0), ("alpha", 1.0)]
        )

    def test_set_controllable_vector_infers_and_caches(self):
        slave = _DummyMoose(instance_name="test", guid="1234")

        class Control:
            def __init__(self):
                self.calls = []
                self.continue_calls = 0

            def setControllableVectorReal(self, path, value):
                self.calls.append(("real", path, value))

            def setControllableVectorInt(self, path, value):
                self.calls.append(("int", path, value))

            def setControllableVectorString(self, path, value):
                self.calls.append(("string", path, value))

            def setContinue(self):
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
        self.assertTrue(slave.set_controllable_vector("vec", [1, 2, 3], value_type="int"))
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
        slave = _DummyMoose(instance_name="test", guid="1234")

        class Control:
            def setControllableVectorReal(self, path, value):
                pass

            def setControllableVectorInt(self, path, value):
                pass

            def setControllableVectorString(self, path, value):
                pass

            def setContinue(self):
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

    def test_parse_flags_handles_strings_and_iterables(self):
        slave = _DummyMoose(instance_name="test", guid="1234")

        parsed = slave._parse_flags("initial, timestep_begin;custom | Another")
        self.assertEqual(
            parsed, {"INITIAL", "TIMESTEP_BEGIN", "CUSTOM", "ANOTHER"}
        )

        parsed_iterable = slave._parse_flags(["Ready", " done ", ""])
        self.assertEqual(parsed_iterable, {"READY", "DONE"})

    @mock.patch("MooseFMU.MOOSE2FMU.MooseControl")
    def test_exit_initialization_mode_rebuilds_command(self, mock_control):
        slave = _DummyMoose(instance_name="test", guid="1234")

        slave.moose_mpi = "mpiexec"
        slave.mpi_num = "4"
        slave.moose_executable = "/custom/moose-opt"
        slave.moose_inputfile = "custom_input.i"

        self.assertTrue(slave.exit_initialization_mode())

        expected_command = [
            "mpiexec",
            "-n",
            "4",
            "/custom/moose-opt",
            "-i",
            "custom_input.i",
        ]

        mock_control.assert_called_once_with(
            moose_command=expected_command,
            moose_control_name=slave.server_name,
        )
        mock_control.return_value.initialize.assert_called_once_with()
        self.assertEqual(slave.cmd, expected_command)

    def test_sync_with_moose_success(self):
        slave = _DummyMoose(instance_name="test", guid="1234")

        class Control:
            def __init__(self):
                self._times = iter([0.0, 1.0005])
                self.wait_calls = []
                self.continue_calls = 0

            def getTime(self):
                return next(self._times)

            def wait(self, flag):
                self.wait_calls.append(flag)

            def setContinue(self):
                self.continue_calls += 1

        slave.control = Control()

        flags = iter(["INITIAL", "TIMESTEP_BEGIN"])

        def fake_get_flag(self, allowed_flags, max_retries, wait_seconds=0.5):
            return next(flags)

        slave.get_flag_with_retries = MethodType(fake_get_flag, slave)

        moose_time, signal = slave.sync_with_moose(1.0, 0.1)

        self.assertEqual(signal, "TIMESTEP_BEGIN")
        self.assertAlmostEqual(moose_time, 1.0005, delta=1e-3)
        self.assertEqual(slave.control.wait_calls, ["INITIAL"])
        self.assertEqual(slave.control.continue_calls, 1)


        def test_ensure_control_listening(self):
            slave = _DummyMoose(instance_name="test", guid="1234")

        class Control:
            def __init__(self, listening: bool):
                self._listening = listening
                self.finalized = False

            def isListening(self):
                return self._listening

            def finalize(self):
                self.finalized = True

        slave.control = Control(True)
        self.assertTrue(slave.ensure_control_listening())
        self.assertFalse(slave.control.finalized)

        slave.control = Control(False)
        self.assertFalse(slave.ensure_control_listening())
        self.assertTrue(slave.control.finalized)

    def test_get_postprocessor_value(self):
        slave = _DummyMoose(instance_name="test", guid="1234")

        class Control:
            def __init__(self):
                self.waited = []
                self.continue_calls = 0
                self.finalized = False

            def wait(self, flag):
                self.waited.append(flag)

            def getPostprocessor(self, name):
                return {"temp": 42.0}[name]

            def setContinue(self):
                self.continue_calls += 1

            def finalize(self):
                self.finalized = True

        slave.control = Control()

        def fake_get_flag(self, allowed_flags, max_retries, wait_seconds=0.5):
            return "READY"

        slave.get_flag_with_retries = MethodType(fake_get_flag, slave)

        value = slave.get_postprocessor_value("READY", "temp", 1.0)

        self.assertEqual(value, 42.0)
        self.assertEqual(slave.control.waited, ["READY"])
        self.assertEqual(slave.control.continue_calls, 1)
        self.assertFalse(slave.control.finalized)

    def test_get_postprocessor_value_failure(self):
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

        result = slave.get_postprocessor_value("READY", "temp", 5.0)

        self.assertIsNone(result)
        self.assertTrue(slave.control.finalized)

    def test_get_reporter_value(self):
        slave = _DummyMoose(instance_name="test", guid="1234")

        class Control:
            def __init__(self):
                self.waited = []
                self.continue_calls = 0
                self.finalized = False

            def wait(self, flag):
                self.waited.append(flag)

            def getReporterValue(self, name):
                return {"flux": 3.14}[name]

            def setContinue(self):
                self.continue_calls += 1

            def finalize(self):
                self.finalized = True

        slave.control = Control()

        def fake_get_flag(self, allowed_flags, max_retries, wait_seconds=0.5):
            return "READY"

        slave.get_flag_with_retries = MethodType(fake_get_flag, slave)

        value = slave.get_reporter_value("READY", "flux", 2.0)



        self.assertEqual(value, 3.14)
        self.assertEqual(slave.control.waited, ["READY"])
        self.assertEqual(slave.control.continue_calls, 1)
        self.assertFalse(slave.control.finalized)

if __name__ == "__main__":
    unittest.main()
