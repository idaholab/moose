import os
from datetime import datetime
from setuptools import setup, find_packages


version = os.environ.get("MOOSE_PYTHON_VERSION", None)
if not version:
    version = datetime.today().strftime("%Y.%m.%d")

setup(
    name="moose-python",
    version=version,
    description="Python utilities from MOOSE",
)
