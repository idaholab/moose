import os
import sys
import pytest

# Ensure the repository's python module directory is on the path
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '../../../../python'))

pythonfmu = pytest.importorskip('pythonfmu')
from MooseFMU import MooseSlave


class _DummyMoose(MooseSlave):
    """Minimal subclass used for testing the base class."""

    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

    def do_step(self, current_time: float, step_size: float, no_set_fmu_state_prior: bool = False) -> bool:
        return True


def test_moose_slave_initialization():
    slave = _DummyMoose(instance_name="test", guid="1234")
    assert slave.exit_initialization_mode()
