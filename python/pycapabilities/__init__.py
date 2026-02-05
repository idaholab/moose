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

This will first attempt to load the built library from this
directory (if it was built during the normal MOOSE build
previously). If that import fails, it run "make capabilities"
within the MOOSE source test directory to build it and then
will attempt to import it again.
"""

import os
import subprocess

try:
    from . import _pycapabilities
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

Capabilities = _pycapabilities.Capabilities
CapabilityException = _pycapabilities.CapabilityException
CheckState = _pycapabilities.CheckState

__all__ = ["Capabilities", "CapabilityException", "CheckState"]
