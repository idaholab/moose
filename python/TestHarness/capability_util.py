# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

"""Define utilities for interacting with MOOSE capabilities."""

import re
import subprocess
import sys
from typing import TYPE_CHECKING, Iterable, Optional, Set, Union

if TYPE_CHECKING:
    from pycapabilities import Capabilities

from TestHarness.util import outputHeader, parseMOOSEJSON, runCommand


def getAppCapabilities(executable: str) -> dict:
    """Get an application's capabilities."""
    assert isinstance(executable, str)

    cmd = f"{executable} --show-capabilities"
    try:
        output = runCommand(cmd, force_mpi_command=True, check=True)
    except subprocess.CalledProcessError as e:
        print("ERROR: Failed to parse the application capabilities!")
        print(f"Command ran: {cmd}\n")
        print(outputHeader("Failed command output"))
        print(e.stdout)
        sys.exit(1)
    return parseMOOSEJSON(output, "--show-capabilities")


class CapabilityException(Exception):
    """Exception for a capability initialization or evaluation error."""


def checkAppCapabilities(
    capabilities: "Capabilities",
    required: str,
    certain: bool,
    add_capabilities: Optional[dict] = None,
    negate_capabilities: Optional[Iterable[str]] = None,
):
    """
    Check a capability requirement against known capabilities.

    Arguments:
    ---------
    capabilities : pycapabilities.Capabilities
        The built application capabilities.
    required : str
        The capability expression to check.
    certain : bool
        If True, don't allow a possible pass.

    Optional arguments:
    ------------------
    add_capabilities : Optional[dict]
        Capabilities to add to the registry during the check.
    negate_capabilities : Optional[Iterable[str]]
        Capabilities to negate in the registry during the check.

    """
    # This is one of the few places where we actually
    # load the pycapabilities module and that is
    # intentional as it can trigger a build
    import pycapabilities

    try:
        status, _, _ = capabilities.check(
            required,
            add_capabilities=add_capabilities,
            negate_capabilities=negate_capabilities,
        )
    # Re-cast this exception as one that doesn't
    # need the pycapabilities module to use
    except pycapabilities.CapabilityException as e:
        raise CapabilityException(e)

    return status == pycapabilities.CheckState.CERTAIN_PASS or (
        status == pycapabilities.CheckState.POSSIBLE_PASS and not certain
    )


def addAugmentedCapability(
    app_capabilities: Union["Capabilities", set[str]],
    augmented_capabilities: dict,
    name: str,
    value: Optional[Union[bool, str, int]],
    doc: str,
    enumeration: Optional[list[str]] = None,
    explicit: Optional[bool] = None,
):
    """
    Append a runtime augmented capability.

    Arguments:
    ---------
    app_capabilities : Union["Capabilities", set[str]]
        The application's Capabilities, or their names.
    augmented_capabilities : dict
        The augmented capabilities to fill add to.
    name : str
        The name of the capability.
    value : Optional[Union[bool, str, int]]
        The capability value; None will become False.
    doc : str
        The documentation string for the capability.

    Optional arguments:
    ------------------
    enumeration : Optional[list[str]]
        Enumerated options for the capability (string values only).
    explicit : Optional[bool]
        Whether or not the capability is explicit (non bool-valued capabilites only).

    """
    from pycapabilities import AUGMENTED_CAPABILITY_NAMES, Capabilities

    assert isinstance(app_capabilities, (Capabilities, set))
    assert isinstance(augmented_capabilities, dict)
    assert isinstance(name, str)
    assert isinstance(value, (bool, str, int, type(None)))
    assert isinstance(doc, str)
    assert isinstance(enumeration, (list, type(None)))
    assert isinstance(explicit, (bool, type(None)))

    if isinstance(app_capabilities, Capabilities):
        app_capabilities = set(app_capabilities.values)

    # Names must be lowercase
    name = name.lower()

    # Allow "none" to mean a bool capability with value False
    if value is None:
        value = False
    # String values must be lowercase
    elif isinstance(value, str):
        value = value.lower()

    if isinstance(enumeration, list):
        assert len(enumeration) > 0, "Enumeration is empty"
        assert isinstance(value, (str)), "Enumeration only valid for str capabilities"
        assert all(
            isinstance(v, str) for v in enumeration
        ), "Enumeration values not strs"
        enumeration = [v.lower() for v in enumeration]
        assert value in enumeration, "Value is not in enumeration"

    if name not in AUGMENTED_CAPABILITY_NAMES:
        raise ValueError(
            f"Capability {name} is not a registered augmented capability name"
        )
    if name in app_capabilities:
        raise ValueError(
            f"Capability {name} is defined by the app, but it is a reserved "
            "dynamic test harness capability. This is an application bug."
        )
    if name in augmented_capabilities:
        raise ValueError(
            f"Capability {name} is already defined as an augmented capability."
        )

    entry = {
        "doc": doc,
        "value": value,
    }
    if enumeration is not None:
        entry["enumeration"] = enumeration
    if explicit is not None:
        entry["explicit"] = explicit

    augmented_capabilities[name] = entry


def parseRequiredCapabilities(
    required: list[str], capabilities: "Capabilities"
) -> list[str]:
    """Parse the required capabilities from --only-tests-that-require."""
    assert isinstance(required, list)
    assert all(isinstance(v, str) for v in required)

    from pycapabilities import AUGMENTED_CAPABILITY_NAMES

    result = []
    for entry in required:
        entry = entry.strip()
        if not re.fullmatch(r"[a-z0-9_-]+", entry):
            raise ValueError(f"Capability '{entry}' has unallowed characters")
        if (cap := capabilities.values.get(entry)) is not None:
            if cap["value"] is False:
                raise ValueError(f"Capability '{entry}' is false")
        elif entry not in AUGMENTED_CAPABILITY_NAMES:
            raise ValueError(f"Capability '{entry}' is not registered")
        result.append(entry)

    return result
