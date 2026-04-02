# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

"""Defines the interfaces used to produce runners for the MooseControl."""

from .subprocessrunnerinterface import SubprocessRunnerInterface

__all__ = [
    "SubprocessRunnerInterface",
]
