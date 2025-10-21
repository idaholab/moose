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
from typing import Any, Optional, Tuple, Type

from requests import Response

from moosecontrol.exceptions import BadStatus, UnexpectedResponse, WebServerControlError


@dataclass(frozen=True)
class WebServerControlResponse:
    """
    Combined response for a POST or GET to the web server.
    """

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
        """
        Whether or not the response has data.
        """
        return self._data is not None


@staticmethod
def process_response(
    response: Response, require_status: Optional[int] = None
) -> WebServerControlResponse:
    """
    Processes a web server response (a GET or a POST request).

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
        if error := data.get("error"):
            raise WebServerControlError(response, error)

    # Force the required status code if any
    if require_status is not None and require_status != response.status_code:
        raise BadStatus(response, require_status)

    # Check for bad statuses
    response.raise_for_status()

    return WebServerControlResponse(response=response, _data=data)


def check_response_data(
    ws_response: WebServerControlResponse,
    expected: list[Tuple[str, Type | Any | Tuple[Type]]],
    optional: Optional[list[Tuple[str, Type | Any | Tuple[Type]]]] = None,
):
    """
    Checks that the given webserver response data contains
    the given keys and associated types.

    Parameters
    ----------
    ws_response : WebServerControlResponse
        The response to check.
    expected : list[Tuple[str, Type | Tuple[Type]]]:
        List of expected key name -> type/types.

    Additional Parameters
    ---------------------
    optional : list[Tuple[str, Type | Tuple[Type]]]:
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
        if v_type is not Any and key in data:
            if not isinstance(data[key], v_type):
                raise UnexpectedResponse(
                    response=response,
                    message=f'key "{key}" has unexpected type "'
                    f'{type(data[key]).__name__}"',
                )
