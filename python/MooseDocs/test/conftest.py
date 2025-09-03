import os
import pytest

from MooseDocs import MOOSE_DIR


@pytest.fixture(autouse=True)
def setMooseDir(monkeypatch):
    """Sets MOOSE_DIR for all tests in MooseDocs"""
    if "MOOSE_DIR" not in os.environ:
        monkeypatch.setenv("MOOSE_DIR", MOOSE_DIR)
    yield
