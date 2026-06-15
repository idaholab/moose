# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

"""Unit tests for helpers that expose FMU utilities."""

import logging
import unittest
from types import SimpleNamespace
from unittest.mock import patch

from moosefmu import fmu_utils


class TestConfigureFmuLogging(unittest.TestCase):
    """Validate configuration of FMU-specific logging helpers."""

    def setUp(self):
        """Capture existing logger configuration for restoration."""
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
        """Restore logger state after each test."""
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
        """INFO configuration should propagate and disable urllib noise."""
        logger = fmu_utils.configure_fmu_logging(logger_name="test.logger.info")

        self.assertEqual(logger.name, "test.logger.info")
        self.assertEqual(logger.level, logging.INFO)
        self.assertEqual(self._root_logger.level, logging.INFO)
        self.assertEqual(logging.getLogger("Moose2FMU").level, logging.INFO)

        urllib_logger = logging.getLogger("urllib3.connectionpool")
        self.assertTrue(urllib_logger.disabled)
        self.assertFalse(urllib_logger.propagate)

    def test_configure_sets_debug_levels_and_logs_message(self):
        """DEBUG configuration should log a helpful message."""
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
    """Check the higher-level FMU utility functions."""

    def test_fmu_info_logs_model_variables(self):
        """``fmu_info`` should log details about model variables."""
        calls = {}

        variables = {
            "temp": SimpleNamespace(
                causality="parameter",
                variability="continuous",
                type="Real",
                value_reference=1,
                start=273.15,
            ),
            "status": SimpleNamespace(
                causality="output",
                variability="discrete",
                type="String",
                value_reference=2,
                start="cold",
            ),
        }

        class _DummyModel:
            def get_model_variables(self):
                return variables

            def get_variable_start(self, name):
                return variables[name].start

            def terminate(self):
                calls["terminated"] = True

        def fake_load_fmu(path):
            calls["path"] = path
            return _DummyModel()

        with (
            patch.object(fmu_utils, "load_fmu", new=fake_load_fmu),
            self.assertLogs(fmu_utils.logger, level=logging.INFO) as logs,
        ):
            result = fmu_utils.fmu_info("model.fmu", "model.fmu")

        self.assertEqual(calls["path"], "model.fmu")
        self.assertTrue(calls["terminated"])
        self.assertEqual(result.modelVariables[0].name, "temp")
        self.assertEqual(result.modelVariables[1].name, "status")
        log_messages = "\n".join(logs.output)
        self.assertIn("Load FMU model description: model.fmu", log_messages)
        self.assertIn("FMU model info:", log_messages)
        self.assertIn("temp", log_messages)
        self.assertIn("status", log_messages)


class _DummyFmu:
    """Simple FMU stand-in used to verify accessor helpers."""

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


class _DummyNameAccessFmu:
    """FMU stand-in exposing pyfmi-style name-based get/set APIs."""

    def __init__(self):
        self.values = {
            "temperature": [12.5],
            "mode": ["ready"],
            "enabled": [1],
        }
        self.set_values = {}

    def get(self, name):
        return self.values[name]

    def set(self, name, value):
        self.set_values[name] = value


class TestFmuAccessorHelpers(unittest.TestCase):
    """Confirm helper functions map names to value references."""

    def test_get_and_set_helpers_use_value_references(self):
        """Helper functions should call the appropriate FMU getters/setters."""
        fmu = _DummyFmu()
        vr_map = {"temperature": 1, "mode": 2, "enabled": 3}

        # Set real value for the FMU variable referenced by "temperature"
        self.assertEqual(fmu_utils.get_real(fmu, vr_map, "temperature"), 12.5)
        # Set string value for the FMU variable referenced by "mode"
        self.assertEqual(fmu_utils.get_string(fmu, vr_map, "mode"), "ready")
        # Set bool value for the FMU variable referenced by "enabled"
        self.assertTrue(fmu_utils.get_bool(fmu, vr_map, "enabled"))

        fmu_utils.set_real(fmu, vr_map, "temperature", 98.6)
        fmu_utils.set_string(fmu, vr_map, "mode", "active")
        fmu_utils.set_bool(fmu, vr_map, "enabled", False)

        # Inspect the recorded call log to confirm each helper resolved the
        # variable name to its value reference via vr_map and invoked the
        # matching typed FMI accessor: getReal([1]) for "temperature",
        # getString([2]) for "mode", getBoolean([3]) for "enabled".
        self.assertEqual(fmu.calls["getReal"], [(1,)])
        self.assertEqual(fmu.calls["getString"], [(2,)])
        self.assertEqual(fmu.calls["getBoolean"], [(3,)])
        self.assertEqual(fmu.calls["setReal"], [((1,), (98.6,))])
        self.assertEqual(fmu.calls["setString"], [((2,), ("active",))])
        self.assertEqual(fmu.calls["setBoolean"], [((3,), (False,))])
        self.assertEqual(fmu.set_values, {1: 98.6, 2: "active", 3: False})

    def test_name_based_helpers_use_shared_scalar_conversion(self):
        """Helpers should support pyfmi-style name-based get/set APIs."""
        fmu = _DummyNameAccessFmu()

        self.assertEqual(fmu_utils.as_scalar([12.5]), 12.5)
        self.assertEqual(fmu_utils.get_scalar(fmu, "mode"), "ready")
        self.assertEqual(fmu_utils.get_float(fmu, "temperature"), 12.5)

        # No value-reference map is needed for name-based APIs.
        self.assertEqual(fmu_utils.get_real(fmu, {}, "temperature"), 12.5)
        self.assertEqual(fmu_utils.get_string(fmu, {}, "mode"), "ready")
        self.assertTrue(fmu_utils.get_bool(fmu, {}, "enabled"))

        fmu_utils.set_real(fmu, {}, "temperature", 98.6)
        fmu_utils.set_string(fmu, {}, "mode", "active")
        fmu_utils.set_bool(fmu, {}, "enabled", False)
        self.assertEqual(
            fmu.set_values,
            {"temperature": 98.6, "mode": "active", "enabled": False},
        )

    def test_as_scalar_rejects_non_scalar_sequences(self):
        """as_scalar should raise when a sequence has more than one value."""
        with self.assertRaises(ValueError):
            fmu_utils.as_scalar([1, 2])


if __name__ == "__main__":
    unittest.main()
