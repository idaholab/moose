# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

"""Implement data classes for moosetools.capabilities."""

from dataclasses import dataclass, field

from moosetools.capabilities import CheckState


@dataclass(frozen=True)
class CheckOptions:
    """Options for moosetools.capabilities.Capabilities.check()."""

    certain: bool = True
    """Whether or not all capabilities must be known."""
    ignore_capabilities: set[str] = field(default_factory=set)
    """Capabilities to ignore; checks using them will always pass."""

    def __post_init__(self):
        """Perform type checking."""
        assert isinstance(self.certain, bool)
        assert isinstance(self.ignore_capabilities, set)
        assert all(isinstance(v, str) for v in self.ignore_capabilities)


@dataclass(frozen=True)
class CheckResult:
    """Storage from the result from Capabilities.check()."""

    state: CheckState
    """The state of the check."""
    capability_names: set[str]
    """The capability names that existed in the check string."""

    def __post_init__(self):
        """Perform type checking."""
        assert isinstance(self.state, CheckState)
        assert isinstance(self.capability_names, set)
        assert all(isinstance(v, str) for v in self.capability_names)
