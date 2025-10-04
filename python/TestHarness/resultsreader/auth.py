#* This file is part of the MOOSE framework
#* https://mooseframework.inl.gov
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from dataclasses import dataclass
from typing import Optional
import os
import yaml

NoneType = type(None)

@dataclass
class Authentication:
    """
    Helper class for storing the authentication to a mongo database
    """
    def __post_init__(self):
        assert isinstance(self.host, str)
        assert isinstance(self.username, str)
        assert isinstance(self.password, str)
        assert isinstance(self.port, (int, NoneType))

    # The host name
    host: str
    # The username
    username: str
    # The password
    password: str
    # The port
    port: Optional[int] = None

@staticmethod
def load_authentication(var_prefix: str) -> Optional[Authentication]:
    """
    Attempts to first load the authentication environment from
    env vars <var_prefix>_AUTH_[HOST,USERNAME,PASSWORD] if
    available. Otherwise, tries to load the authentication
    environment from the file set by env var
    <var_prefix>_AUTH_FILE if it is available.
    """
    # Helpers for getting authentication variables
    var_name = lambda k: f'{var_prefix}_AUTH_{k.upper()}'
    get_var = lambda k: os.environ.get(var_name(k))

    # Try to get authentication from env
    all_auth_keys = ['host', 'username', 'password']
    auth = {}
    for key in all_auth_keys:
        v = get_var(key)
        if v:
            auth[key] = v
    # Have all three set
    if len(auth) == 3:
        auth['port'] = get_var('port')
        return Authentication(**auth)
    # Have one or two but not all three set
    if len(auth) != 0:
        all_auth_vars = ' '.join(map(var_name, all_auth_keys))
        raise ValueError(f'All environment variables "{all_auth_vars}" must be set for authentication')

    # Try to get authentication from file
    auth_file = get_var('file')
    if auth_file is None:
        return None
    try:
        with open(auth_file, 'r') as f:
            values = yaml.safe_load(f)
        return Authentication(**values)
    except Exception as e:
        raise Exception(f"Failed to load credentials from '{auth_file}'") from e

@staticmethod
def has_authentication(var_prefix: str) -> bool:
    """
    Checks whether or not environment authentication is available
    """
    return load_authentication(var_prefix) is not None
