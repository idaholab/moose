import os
import sys
import logging
import pytest

# Ensure the repository's python module directory is on the path
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '../../../../python'))

pythonfmu = pytest.importorskip('pythonfmu')
from MooseFMU import Moose2FMU


class _DummyMoose(Moose2FMU):
    """Minimal subclass used for testing the Moose2FMU base logic."""

    def do_step(self, current_time: float, step_size: float, no_set_fmu_state_prior: bool = False) -> bool:
        return True


def test_get_flag_with_retries_success(caplog):
    slave = _DummyMoose(instance_name="test", guid="1234")

    class Control:
        def __init__(self):
            self.calls = 0

        def getWaitingFlag(self):
            self.calls += 1
            return "READY" if self.calls >= 2 else None

    slave.control = Control()

    with caplog.at_level(logging.INFO):
        result = slave._get_flag_with_retries("READY", max_retries=5, wait_seconds=0)

    assert result == "READY"
    assert slave.control.calls == 2
    assert "Successfully got flag 'READY'" in caplog.text


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
        result = slave._get_flag_with_retries("READY", max_retries=3, wait_seconds=0)

    assert result is None
    assert slave.control.calls == 3
    assert "Failed to get flag 'READY' after 3 retries." in caplog.text


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
        result = slave._get_flag_with_retries("OK", max_retries=4, wait_seconds=0)

    assert result == "OK"
    assert slave.control.calls == 2
    assert "Attempt 1/4 failed" in caplog.text
