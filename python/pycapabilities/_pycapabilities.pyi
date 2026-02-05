from enum import IntEnum
from typing import Iterable, Optional, Tuple

class CheckState(IntEnum):
    """Return state for a capability check."""

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

class CapabilityException(Exception):
    """Exception for a capability initialization or evaluation error."""

    ...

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
        add_capabilities: Optional[dict] = None,
        negate_capabilities: Optional[Iterable[str]] = None,
    ) -> Tuple[CheckState, str, str]:
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
        add_capabilities : Optional[dict]
            Additional capabilities to temporarily add to the registry.
        negate_capabilities : Optional[Iterable[str]]
            Capabilities to temporarily negate in the registry.

        Returns
        -------
        CheckState
            The state of the check.
        str
            A reason associated with the check (currently unused).
        str
            Documentation associated with the check (currently unused).

        """
        ...
