# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

"""
Interface to the Moose::Capabilities system.

This module will attempt to build the pycapabilities
library if it has not already been built.
"""

import os
import subprocess

# First try to import the built library from this directory,
# which might have been built during a MOOSE app build or
# individually with "make pycapabilities" in a MOOSE app dir
try:
    from . import _pycapabilities
# If the already-built import fails, attempt to build it
except ImportError:
    moose_dir = os.getenv(
        "MOOSE_DIR",
        os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "..")),
    )
    moose_test_dir = os.path.abspath(os.path.join(moose_dir, "test"))
    cmd = ["make", "pycapabilities"]
    print(f'INFO: Building capabilities with "{" ".join(cmd)}" in {moose_test_dir}')
    try:
        subprocess.run(cmd, cwd=moose_test_dir, check=True)
    except subprocess.CalledProcessError:
        raise SystemExit("ERROR: Failed to build capabilities")
    try:
        from . import _pycapabilities
    except Exception as e:
        print("ERROR: Failed to import capabilities after building")
        raise RuntimeError("ERROR: Failed to import capabilities after building") from e

AUGMENTED_CAPABILITY_NAMES = _pycapabilities.AUGMENTED_CAPABILITY_NAMES
Capabilities = _pycapabilities.Capabilities
CapabilityException = _pycapabilities.CapabilityException
CheckState = _pycapabilities.CheckState

# Need to add "platform" that can't exist in moose but needs
# to exist for the TestHarness --minimal-capabilities run
AUGMENTED_CAPABILITY_NAMES.add("platform")

__all__ = [
    "AUGMENTED_CAPABILITY_NAMES",
    "Capabilities",
    "CapabilityException",
    "CheckState",
]
