# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

"""Contains tests for the moosecontrol module."""

import os
import sys
from importlib.util import find_spec

# Add the MOOSE python to PATH if needed
if find_spec("moosecontrol") is None:
    this_dir = os.path.dirname(__file__)
    moose_python = os.path.abspath(os.path.join(this_dir, "..", ".."))
    sys.path.append(moose_python)
    assert find_spec("moosecontrol")
