#* This file is part of the MOOSE framework
#* https://mooseframework.inl.gov
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

"""Defines exceptions."""

from requests import Response

class ControlNotWaiting(Exception):
    """
    Exception for the control not waiting when it should be.
    """
    def __init__(self):
        super().__init__('The control is not currently waiting')

class UnexpectedResponse(Exception):
    """
    Exception when response data doesn't match the expectation.
    """
    def __init__(self, response: Response, message: str):
        self.response: Response = response
        super().__init__(f'Response from {response.url} {message}')

class UnexpectedFlag(Exception):
    """
    Exception when the server is at a different flag than expected.
    """
    def __init__(self, flag: str):
        super().__init__(f'Unexpected execute on flag {flag}')

class BadStatus(Exception):
    """
    Exception for an unexpected Request status code.
    """
    def __init__(self, response: Response, expected_status: int):
        self.response = response
        super().__init__(
            f'Request {response.url} status {response.status_code}'
            f' != expected {expected_status}'
        )

class WebServerControlError(Exception):
    """
    Exception for the WebServerControl returning a Request with an error message.
    """
    def __init__(self, response: Response, error: str):
        self.response: Response = response
        self.error: str = error
        super().__init__(f'Request to {response.url}: {error}')

class InitializeTimeout(Exception):
    """
    Exception for timing out during init.
    """
    def __init__(self, waited_time: float):
        self.waited_time: float = waited_time
        message = f'Initialization timed out after {waited_time:.2f} seconds'
        super().__init__(message)
