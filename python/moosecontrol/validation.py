# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

"""Defines structures and methods for validating WebServerControl responses."""

from dataclasses import dataclass
from typing import Optional, Tuple, Type

from requests import Response

from moosecontrol.exceptions import (
    UnexpectedResponse,
    UnexpectedStatus,
    WebServerControlError,
)


@dataclass(frozen=True)
class WebServerControlResponse:
    """Combined response for a POST or GET to the web server."""

    # The Response
    response: Response
    # The underlying data in the response (if any)
    _data: Optional[dict]

    @property
    def data(self) -> dict:
        """
        Get the underlying data in the response.

        Data must exist.
        """
        assert self._data is not None
        return self._data

    def has_data(self) -> bool:
        """Whether or not the response has data."""
        return self._data is not None


@dataclass(frozen=True)
class WebServerInitializedData:
    """Data received about the server on initialize."""

    # The underlying data
    _data: dict

    def __post_init__(self):
        """Perform type checking."""
        assert isinstance(self.data, dict)
        assert isinstance(self.control_name, str)
        assert isinstance(self.control_type, str)
        assert isinstance(self.execute_on_flags, list)
        assert all(isinstance(v, str) for v in self.execute_on_flags)

    @property
    def data(self) -> dict:
        """The underlying data."""
        return self._data

    @property
    def control_type(self) -> str:
        """Type of the WebServerControl."""
        return self.data["control_type"]

    @property
    def control_name(self) -> str:
        """Name of the WebServerControl."""
        return self.data["control_name"]

    @property
    def execute_on_flags(self) -> list[str]:
        """Execute on flags the WebServerControl is listening on."""
        return self.data["execute_on_flags"]


def process_response(
    response: Response, require_status: Optional[int] = None
) -> WebServerControlResponse:
    """
    Process a web server response (a GET or a POST request).

    Performs additional checking, parsing the JSON
    response (if any) and checking for an error.

    Parameters
    ----------
    response : Response
        The built response from the request.

    Optional Parameters
    -------------------
    require_status : Optional[int]
        Check that the status code is this if set.

    Returns
    -------
    WebServerControlResponse:
        The combined response, along with the JSON data if any.

    """
    # Parse the JSON response, if any, also checking for an error
    data = None
    if response.headers.get("content-type") == "application/json":
        data = response.json()
        assert isinstance(data, dict)
        if error := data.get("error"):
            raise WebServerControlError(response, error)

    # Force the required status code if any
    if require_status is not None and require_status != response.status_code:
        raise UnexpectedStatus(response, require_status)

    # Check for bad statuses
    response.raise_for_status()

    return WebServerControlResponse(response=response, _data=data)


def check_response_data(
    ws_response: WebServerControlResponse,
    expected: list[Tuple[str, Optional[Type | Tuple[Type]]]],
    optional: Optional[list[Tuple[str, Optional[Type | Tuple[Type]]]]] = None,
):
    """
    Check data from a webserver response containing expected values.

    None passed as an expected or optional type implies that
    the type can be anything and should not be checked.

    Parameters
    ----------
    ws_response : WebServerControlResponse
        The response to check.
    expected : list[Tuple[str, Optional[Type | Tuple[Type]]]]:
        List of expected key name -> type/types.

    Additional Parameters
    ---------------------
    optional : Optional[list[Tuple[str, Optional[Type | Tuple[Type]]]]]:
        List of optional key name -> type/types.

    """
    assert isinstance(ws_response, WebServerControlResponse)
    if optional is None:
        optional = []

    response = ws_response.response
    if not ws_response.has_data():
        raise UnexpectedResponse(response=response, message="does not contain data")
    data = ws_response.data

    expected_keys = [v[0] for v in expected]
    optional_keys = [v[0] for v in optional]
    all_keys = expected_keys + optional_keys

    def join_keys(keys):
        return ", ".join(keys)

    # Keys that shouldn't be there
    if unexpected := [k for k in data if k not in all_keys]:
        raise UnexpectedResponse(
            response=response, message=f"has unexpected key(s): {join_keys(unexpected)}"
        )

    # Keys that should be there
    missing = [k for k in expected_keys if k not in data]
    if missing:
        raise UnexpectedResponse(
            response=response, message=f"has missing key(s): {join_keys(missing)}"
        )

    # Values with the wrong type
    for key, v_type in expected + optional:
        if v_type is not None and key in data and not isinstance(data[key], v_type):
            raise UnexpectedResponse(
                response=response,
                message=f'key "{key}" has unexpected type "{type(data[key]).__name__}"',
            )
