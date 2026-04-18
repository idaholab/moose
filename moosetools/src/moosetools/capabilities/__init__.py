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

This module will attempt to build the moosetools.capabilities._external
binding library if it has not already been built.
"""

# First try to import the external library if it already exists
try:
    from ._external import AUGMENTED_CAPABILITY_NAMES, Capabilities, CheckState
# Otherwise, setup (build) in tree
except ImportError:  # pragma: no cover
    from ._setup_external import setup_external

    setup_external()
    from ._external import AUGMENTED_CAPABILITY_NAMES, Capabilities, CheckState

# Need to add "platform" that can't exist in moose but needs
# to exist for the TestHarness --minimal-capabilities run
AUGMENTED_CAPABILITY_NAMES.add("platform")

__all__ = [
    "AUGMENTED_CAPABILITY_NAMES",
    "Capabilities",
    "CheckState",
]
