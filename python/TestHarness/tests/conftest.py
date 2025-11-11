# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

"""Configures pytest for the TestHarness module."""

pytest_plugins = (
    # Configures pytest for finding and optionally
    # skipping tests that depend on MOOSE.
    "moosepytest.plugins.mooseexe",
)
