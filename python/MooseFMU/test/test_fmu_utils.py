import importlib.util
import logging
import sys
import unittest
from pathlib import Path
from types import SimpleNamespace

from unittest.mock import patch

_TEST_DIR = Path(__file__).resolve().parent
_TEST_SPEC = importlib.util.spec_from_file_location(
    "MooseFMU.test", _TEST_DIR / "__init__.py"
)
if "MooseFMU.test" not in sys.modules:
    _package_module = importlib.util.module_from_spec(_TEST_SPEC)
    sys.modules["MooseFMU.test"] = _package_module
    assert _TEST_SPEC.loader is not None
    _TEST_SPEC.loader.exec_module(_package_module)

from MooseFMU import fmu_utils


class TestConfigureFmuLogging(unittest.TestCase):
    def setUp(self):
        self._root_logger = logging.getLogger()
        self._root_level = self._root_logger.level
        self._root_handlers = list(self._root_logger.handlers)
        for handler in list(self._root_logger.handlers):
            self._root_logger.removeHandler(handler)

        self._moose_logger = logging.getLogger("Moose2FMU")
        self._moose_level = self._moose_logger.level
        self._moose_handlers = list(self._moose_logger.handlers)

        self._urllib_logger = logging.getLogger("urllib3.connectionpool")
        self._urllib_level = self._urllib_logger.level
        self._urllib_disabled = self._urllib_logger.disabled
        self._urllib_propagate = self._urllib_logger.propagate
        self._urllib_handlers = list(self._urllib_logger.handlers)

    def tearDown(self):
        for handler in list(self._root_logger.handlers):
            self._root_logger.removeHandler(handler)
        for handler in self._root_handlers:
            self._root_logger.addHandler(handler)
        self._root_logger.setLevel(self._root_level)

        for handler in list(self._moose_logger.handlers):
            self._moose_logger.removeHandler(handler)
        for handler in self._moose_handlers:
            self._moose_logger.addHandler(handler)
        self._moose_logger.setLevel(self._moose_level)

        for handler in list(self._urllib_logger.handlers):
            self._urllib_logger.removeHandler(handler)
        for handler in self._urllib_handlers:
            self._urllib_logger.addHandler(handler)
        self._urllib_logger.setLevel(self._urllib_level)
        self._urllib_logger.disabled = self._urllib_disabled
        self._urllib_logger.propagate = self._urllib_propagate

    def test_configure_sets_info_levels_and_disables_urllib3(self):
        logger = fmu_utils.configure_fmu_logging(logger_name="test.logger.info")

        self.assertEqual(logger.name, "test.logger.info")
        self.assertEqual(logger.level, logging.INFO)
        self.assertEqual(self._root_logger.level, logging.INFO)
        self.assertEqual(logging.getLogger("Moose2FMU").level, logging.INFO)

        urllib_logger = logging.getLogger("urllib3.connectionpool")
        self.assertTrue(urllib_logger.disabled)
        self.assertFalse(urllib_logger.propagate)

    def test_configure_sets_debug_levels_and_logs_message(self):
        with self.assertLogs("test.logger.debug", level="DEBUG") as captured:
            logger = fmu_utils.configure_fmu_logging(
                debug=True, logger_name="test.logger.debug"
            )

            self.assertEqual(logger.level, logging.DEBUG)
            self.assertEqual(self._root_logger.level, logging.DEBUG)
            self.assertEqual(logging.getLogger("Moose2FMU").level, logging.DEBUG)

        self.assertEqual(logger.getEffectiveLevel(), logging.DEBUG)

        log_output = "\n".join(captured.output)
        self.assertIn("FMU debug logging is enabled", log_output)


class TestFmuUtils(unittest.TestCase):
    def test_fmu_info_logs_model_variables(self):
        variables = [
            SimpleNamespace(
                name="temp",
                causality="parameter",
                variability="continuous",
                type="Real",
                start=273.15,
                valueReference=1,
            ),
            SimpleNamespace(
                name="status",
                causality="output",
                variability="discrete",
                type="String",
                start="cold",
                valueReference=2,
            ),
        ]

        description = SimpleNamespace(modelVariables=variables)

        calls = {}

        def fake_read_model_description(path):
            calls["path"] = path
            return description

        with patch.object(
            fmu_utils, "read_model_description", new=fake_read_model_description
        ):
            with self.assertLogs(fmu_utils.logger, level=logging.INFO) as logs:
                result = fmu_utils.fmu_info("model.fmu", "model.fmu")

        self.assertIs(result, description)
        self.assertEqual(calls["path"], "model.fmu")
        log_messages = "\n".join(logs.output)
        self.assertIn("Load FMU model description: model.fmu", log_messages)
        self.assertIn("FMU model info:", log_messages)
        self.assertIn("temp", log_messages)
        self.assertIn("status", log_messages)

class _DummyFmu:
    def __init__(self):
        self.calls = {
            "getReal": [],
            "getString": [],
            "getBoolean": [],
            "setReal": [],
            "setString": [],
            "setBoolean": [],
        }
        self.values = {1: 12.5, 2: "ready", 3: True}
        self.set_values = {}

    def getReal(self, refs):
        self.calls["getReal"].append(tuple(refs))
        return [self.values[refs[0]]]

    def getString(self, refs):
        self.calls["getString"].append(tuple(refs))
        return [self.values[refs[0]]]

    def getBoolean(self, refs):
        self.calls["getBoolean"].append(tuple(refs))
        return [self.values[refs[0]]]

    def setReal(self, refs, values):
        self.calls["setReal"].append((tuple(refs), tuple(values)))
        self.set_values[refs[0]] = values[0]

    def setString(self, refs, values):
        self.calls["setString"].append((tuple(refs), tuple(values)))
        self.set_values[refs[0]] = values[0]

    def setBoolean(self, refs, values):
        self.calls["setBoolean"].append((tuple(refs), tuple(values)))
        self.set_values[refs[0]] = values[0]


class TestFmuAccessorHelpers(unittest.TestCase):
    def test_get_and_set_helpers_use_value_references(self):
        fmu = _DummyFmu()
        vr_map = {"temperature": 1, "mode": 2, "enabled": 3}

        self.assertEqual(fmu_utils.get_real(fmu, vr_map, "temperature"), 12.5)
        self.assertEqual(fmu_utils.get_string(fmu, vr_map, "mode"), "ready")
        self.assertTrue(fmu_utils.get_bool(fmu, vr_map, "enabled"))

        fmu_utils.set_real(fmu, vr_map, "temperature", 98.6)
        fmu_utils.set_string(fmu, vr_map, "mode", "active")
        fmu_utils.set_bool(fmu, vr_map, "enabled", False)

        self.assertEqual(fmu.calls["getReal"], [(1,)])
        self.assertEqual(fmu.calls["getString"], [(2,)])
        self.assertEqual(fmu.calls["getBoolean"], [(3,)])
        self.assertEqual(fmu.calls["setReal"], [((1,), (98.6,))])
        self.assertEqual(fmu.calls["setString"], [((2,), ("active",))])
        self.assertEqual(fmu.calls["setBoolean"], [((3,), (False,))])
        self.assertEqual(fmu.set_values, {1: 98.6, 2: "active", 3: False})

if __name__ == "__main__":
    unittest.main()
