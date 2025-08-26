from pathlib import Path
from pytest import Config
import importlib.util


def pytest_ignore_collect(collection_path: Path, config: Config) -> bool:
    """Tells pytest to ignore all chigger tests if the VTK module is not loaded."""
    if importlib.util.find_spec("vtk") is None:
        return True
