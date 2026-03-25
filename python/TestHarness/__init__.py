# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

from importlib.util import find_spec

if find_spec("moosetools") is None:
    import os
    import sys

    this_dir = os.path.dirname(__file__)
    moosetools_root_dir = os.path.abspath(
        os.path.join(this_dir, "..", "..", "moosetools", "src")
    )
    sys.path.append(moosetools_root_dir)
    assert find_spec("moosetools") is not None

from .TestHarness import TestHarness
from .OutputInterface import OutputInterface
from .TestHarness import findDepApps
