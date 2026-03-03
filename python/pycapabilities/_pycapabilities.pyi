# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

from enum import IntEnum
from typing import Iterable, Optional, Set

from pycapabilities.dataclasses import CheckResult

AUGMENTED_CAPABILITY_NAMES: Set[str]
"""
Capabilities that are reserved and can only be augmented.

Loaded from Moose::CapabilityUtils::augmented_capability_names.
"""

class CheckState(IntEnum):
    """
    Return state for a capability check.

    This is a direct representation of
    Moose::internal::CapabilityRegistry::CheckState.
    """

    CERTAIN_FAIL = 0
    """A certain failure."""
    POSSIBLE_FAIL = 1
    """A possible failure."""
    UNKNOWN = 2
    """An unknown failure."""
    POSSIBLE_PASS = 3
    """A possible pass."""
    CERTAIN_PASS = 4
    """A certain pass."""

class Capabilities:
    """Python representation of the MOOSE::Capabilities system."""

    def __init__(self, capabilities: dict) -> None:
        """
        Initialize state.

        The capabilities should be obtained from the
        `--show-capabilities` command line option
        on the MOOSE application.

        Parameters
        ----------
        capabilities : dict
            The dict representation of the capabilities.

        """
        ...

    @property
    def values(self) -> dict:
        """Get the underlying registry dictionary."""
        ...

    def check(
        self,
        requirement: str,
        certain: bool = True,
        add_capabilities: Optional[dict] = None,
        negate_capabilities: Optional[Iterable[str]] = None,
    ) -> CheckResult:
        """
        Check capabilities against the registry.

        The TestHarness requires that we augment the application's
        capabilities with additional capabilities for things like
        command line options and other tester parameters. Thus,
        you can also temporarily (within the call to this method)
        augment the registry.

        Parameters
        ----------
        requirement : str
            The capability requirement string.

        Optional Parameters
        -------------------
        certain : bool
            Whether or not the check must be certain (don't allow unknown capabilities).
        add_capabilities : Optional[dict]
            Additional capabilities to temporarily add to the registry.
        negate_capabilities : Optional[Iterable[str]]
            Capabilities to temporarily negate in the registry.

        Returns
        -------
        pycapabilities.dataclasses.CheckResult
            The result of the check.

        """
        ...
