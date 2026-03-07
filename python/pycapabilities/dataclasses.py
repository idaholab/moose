# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

"""Implement data classes for pycapabilities."""

from dataclasses import dataclass

from pycapabilities import CheckState


@dataclass
class CheckResult:
    """Storage from the result from Capabilities.check()."""

    state: CheckState
    """The state of the check."""
    capability_names: set[str]
    """The capability names that existed in the check string."""
