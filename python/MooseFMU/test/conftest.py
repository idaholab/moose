"""Shared pytest configuration and dependency stubs for MooseFMU tests."""

import os
import sys
from types import ModuleType

# Ensure the repository's python module directory is on the path
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
        output = "output"

    class _Fmi2Variability:
        tunable = "tunable"
        continuous = "continuous"

    enums.Fmi2Causality = _Fmi2Causality
    enums.Fmi2Variability = _Fmi2Variability
    pythonfmu.enums = enums
    sys.modules["pythonfmu.enums"] = enums

    variables = ModuleType("pythonfmu.variables")

    class _Variable:
        def __init__(self, name, causality=None, variability=None):
            self.name = name
            self.causality = causality
            self.variability = variability

    class Integer(_Variable):
        pass

    class Real(_Variable):
        pass

    class String(_Variable):
        pass

    variables.Integer = Integer
    variables.Real = Real
    variables.String = String
    pythonfmu.variables = variables
    sys.modules["pythonfmu.variables"] = variables

    default_experiment = ModuleType("pythonfmu.default_experiment")

    class DefaultExperiment:
        def __init__(self, start_time: float, stop_time: float, step_size: float):
            self.start_time = start_time
            self.stop_time = stop_time
            self.step_size = step_size

    default_experiment.DefaultExperiment = DefaultExperiment
    pythonfmu.default_experiment = default_experiment
    sys.modules["pythonfmu.default_experiment"] = default_experiment

if "MooseControl" not in sys.modules:  # pragma: no cover - exercised during import
    moose_control_module = ModuleType("MooseControl")

    class MooseControl:  # pylint: disable=too-few-public-methods
        """Simple stub matching the public API consumed during testing."""

        def __init__(self, *_, **__):
            pass

        def initialize(self):
            return None

        def finalize(self):
            return None

        def isListening(self):
            return True

        # The following methods are present so that Moose2FMU can interact
        # with a stand-in control object when real networking is unavailable.
        def setControllableReal(self, *_):
            return None

        def setContinue(self):
            return None

        def getWaitingFlag(self):
            return None

        def wait(self, *_):
            return None

        def getTime(self):
            return 0.0

        def getPostprocessor(self, *_):
            return 0.0

        def getReporterValue(self, *_):
            return 0.0

    moose_control_module.MooseControl = MooseControl
    sys.modules["MooseControl"] = moose_control_module

if "fmpy" not in sys.modules:  # pragma: no cover - exercised during import
    fmpy_module = ModuleType("fmpy")

    def read_model_description(_):
        class _Description:
            modelVariables = []

        return _Description()

    fmpy_module.read_model_description = read_model_description
    sys.modules["fmpy"] = fmpy_module
