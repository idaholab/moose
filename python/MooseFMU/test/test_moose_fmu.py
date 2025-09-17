import logging
from types import MethodType
import pytest
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


def test_get_flag_with_retries_success(caplog):
    slave = _DummyMoose(instance_name="test", guid="1234")

    class Control:
        def __init__(self):
            self.calls = 0

        def getWaitingFlag(self):
            self.calls += 1
            return " READY " if self.calls >= 2 else None

    slave.control = Control()

    with caplog.at_level(logging.INFO):
        result = slave.get_flag_with_retries({"READY"}, max_retries=5, wait_seconds=0)

    assert result == " READY "
    assert slave.control.calls == 2
    assert "Successfully got flag ' READY ' after 1 retries." in caplog.text

def test_get_flag_with_retries_skips_unexpected_flags(monkeypatch):
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

    assert result == "READY"
    assert calls["waiting"] == 2
    assert calls["skip"] == ["foo"]


def test_get_flag_with_retries_failure(caplog):
    slave = _DummyMoose(instance_name="test", guid="1234")

    class Control:
        def __init__(self):
            self.calls = 0

        def getWaitingFlag(self):
            self.calls += 1
            return None

    slave.control = Control()

    with caplog.at_level(logging.ERROR):
        result = slave.get_flag_with_retries({"READY"}, max_retries=3, wait_seconds=0)

    assert result is None
    assert slave.control.calls == 3
    assert "Failed to get one of ['READY'] after 3 retries." in caplog.text

def test_get_flag_with_retries_handles_exception(caplog):
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

    with caplog.at_level(logging.WARNING):
        result = slave.get_flag_with_retries({"OK"}, max_retries=4, wait_seconds=0)

    assert result == "OK"
    assert slave.control.calls == 2
    assert "Attempt 1/4 failed" in caplog.text

def test_set_controllable_real_caches_values():
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

    assert slave.set_controllable_real("alpha", 1.0) is True
    assert slave.control.set_calls == [("alpha", 1.0)]
    assert slave.control.continue_calls == 1

    assert slave.set_controllable_real("alpha", 1.0) is False
    assert slave.control.set_calls == [("alpha", 1.0)]

    assert slave.set_controllable_real("alpha", 1.0, force=True) is True
    assert slave.control.set_calls == [("alpha", 1.0), ("alpha", 1.0)]


def test_parse_flags_handles_strings_and_iterables():
    slave = _DummyMoose(instance_name="test", guid="1234")

    parsed = slave._parse_flags("initial, timestep_begin;custom | Another")
    assert parsed == {"INITIAL", "TIMESTEP_BEGIN", "CUSTOM", "ANOTHER"}

    parsed_iterable = slave._parse_flags(["Ready", " done ", ""])
    assert parsed_iterable == {"READY", "DONE"}


def test_sync_with_moose_success(monkeypatch):
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

    moose_time, signal = slave.sync_with_moose(1.0)

    assert signal == "TIMESTEP_BEGIN"
    assert pytest.approx(moose_time, rel=0.0, abs=1e-3) == 1.0005
    assert slave.control.wait_calls == ["INITIAL"]
    assert slave.control.continue_calls == 1


def test_ensure_control_listening(monkeypatch):
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
    assert slave.ensure_control_listening() is True
    assert slave.control.finalized is False

    slave.control = Control(False)
    assert slave.ensure_control_listening() is False
    assert slave.control.finalized is True


def test_get_postprocessor_value(monkeypatch):
    slave = _DummyMoose(instance_name="test", guid="1234")

    class Control:
        def __init__(self):
            self.waited = []
            self.continue_calls = 0

        def wait(self, flag):
            self.waited.append(flag)

        def getPostprocessor(self, name):
            return {"temp": 42.0}[name]

        def setContinue(self):
            self.continue_calls += 1

        def finalize(self):
            raise AssertionError("should not finalize on success")

    slave.control = Control()

    def fake_get_flag(self, allowed_flags, max_retries, wait_seconds=0.5):
        return "READY"

    slave.get_flag_with_retries = MethodType(fake_get_flag, slave)

    value = slave.get_postprocessor_value({"READY"}, "temp", 1.0)

    assert value == 42.0
    assert slave.control.waited == ["READY"]
    assert slave.control.continue_calls == 1


def test_get_postprocessor_value_failure(monkeypatch):
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

    result = slave.get_postprocessor_value({"READY"}, "temp", 5.0)

    assert result is None
    assert slave.control.finalized is True


def test_get_reporter_value(monkeypatch):
    slave = _DummyMoose(instance_name="test", guid="1234")

    class Control:
        def __init__(self):
            self.waited = []
            self.continue_calls = 0

        def wait(self, flag):
            self.waited.append(flag)

        def getReporterValue(self, name):
            return {"flux": 3.14}[name]

        def setContinue(self):
            self.continue_calls += 1

        def finalize(self):
            raise AssertionError("should not finalize on success")

    slave.control = Control()

    def fake_get_flag(self, allowed_flags, max_retries, wait_seconds=0.5):
        return "READY"

    slave.get_flag_with_retries = MethodType(fake_get_flag, slave)

    value = slave.get_reporter_value({"READY"}, "flux", 2.0)

    assert value == 3.14
    assert slave.control.waited == ["READY"]
    assert slave.control.continue_calls == 1
