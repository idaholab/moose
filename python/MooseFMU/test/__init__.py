"""Test package initialization and dependency stubs for MooseFMU."""

import os
import sys
from types import ModuleType

# Ensure the repository's python module directory is on the path when running tests
sys.path.insert(0, os.path.join(os.path.dirname(__file__), "../.."))

try:  # pragma: no cover - exercised implicitly during imports
    import pythonfmu  # type: ignore  # noqa: F401
except ModuleNotFoundError:  # pragma: no cover - fallback used in tests when dependency missing
    pythonfmu = ModuleType("pythonfmu")
    sys.modules["pythonfmu"] = pythonfmu

    class _Fmi2Slave:
        """Minimal stand-in for :class:`pythonfmu.Fmi2Slave` used in tests."""

        def __init__(self, *args, **kwargs):
            self._registered_variables = []
            self.time = 0.0
            self.moose_time = 0.0

        def register_variable(self, variable):
            self._registered_variables.append(variable)

    pythonfmu.Fmi2Slave = _Fmi2Slave

    enums = ModuleType("pythonfmu.enums")

    class _Fmi2Causality:
        parameter = "parameter"
