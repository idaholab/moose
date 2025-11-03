# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

"""
Helper __init__ that adds the in-tree MOOSE python to PATH for tests.

In the cases when we're testing in-tree, this file should be
symlinked in the test root for the package as the __init__.py
script. It removes the need to set PYTHONPATH when running tests.
"""

import os
import sys
from importlib.util import find_spec

if find_spec("moosepytest") is None:
    this_dir = os.path.dirname(__file__)
    moose_python = os.path.abspath(os.path.join(this_dir, "..", ".."))
    sys.path.append(moose_python)
    assert find_spec("moosepytest")
