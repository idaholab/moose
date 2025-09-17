import logging
from types import SimpleNamespace

from MooseFMU import fmu_utils


def test_fmu_info_logs_model_variables(monkeypatch, caplog):
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

    monkeypatch.setattr(fmu_utils, "read_model_description", fake_read_model_description)

    with caplog.at_level(logging.INFO):
        result = fmu_utils.fmu_info("model.fmu", "model.fmu")

    assert result is description
    assert calls["path"] == "model.fmu"
    logs = "\n".join(caplog.messages)
    assert "Load FMU model description: model.fmu" in logs
    assert "FMU model info:" in logs
    assert "temp" in logs
    assert "status" in logs


class _DummyFmu:
    def __init__(self):
        self.calls = {"getReal": [], "getString": [], "getBoolean": [], "setReal": [], "setString": [], "setBoolean": []}
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


def test_get_and_set_helpers_use_value_references():
    fmu = _DummyFmu()
    vr_map = {"temperature": 1, "mode": 2, "enabled": 3}

    assert fmu_utils.get_real(fmu, vr_map, "temperature") == 12.5
    assert fmu_utils.get_string(fmu, vr_map, "mode") == "ready"
    assert fmu_utils.get_bool(fmu, vr_map, "enabled") is True

    fmu_utils.set_real(fmu, vr_map, "temperature", 98.6)
    fmu_utils.set_string(fmu, vr_map, "mode", "active")
    fmu_utils.set_bool(fmu, vr_map, "enabled", False)

    assert fmu.calls["getReal"] == [(1,)]
    assert fmu.calls["getString"] == [(2,)]
    assert fmu.calls["getBoolean"] == [(3,)]
    assert fmu.calls["setReal"] == [((1,), (98.6,))]
    assert fmu.calls["setString"] == [((2,), ("active",))]
    assert fmu.calls["setBoolean"] == [((3,), (False,))]
    assert fmu.set_values == {1: 98.6, 2: "active", 3: False}
