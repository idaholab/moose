#* This file is part of the MOOSE framework
#* https://mooseframework.inl.gov
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from requests import Response

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
