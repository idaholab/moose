"""Tool for applying environment variable to paths."""

import os
import re

def __sub(match):
    """Substitute function for environment variables."""
    env = match.group('env')
    if env in os.environ:
        return os.environ[env]
    return match.group()

def eval_path(path):
    """
    Import environment variables into paths.

    Inputs:
        path[str]: Path containing environment variable: e.g., ${MOOSE_DIR}/python
    """
    return re.sub(r'\$\{(?P<env>.*?)\}', __sub, path)
