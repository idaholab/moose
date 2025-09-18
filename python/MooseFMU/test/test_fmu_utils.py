import importlib.util
import logging
import sys
import unittest
from pathlib import Path
from types import SimpleNamespace

from unittest.mock import patch

_TEST_DIR = Path(__file__).resolve().parent
_SPEC = importlib.util.spec_from_file_location(
    "MooseFMU.test", _TEST_DIR / "__init__.py"
)
if "MooseFMU.test" not in sys.modules:
    _package_module = importlib.util.module_from_spec(_SPEC)
    sys.modules["MooseFMU.test"] = _package_module
    assert _SPEC.loader is not None
    _SPEC.loader.exec_module(_package_module)

from MooseFMU import fmu_utils


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
