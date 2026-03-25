# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

"""Utilities for interacting with a CIVET run in tests."""

import os


def is_civet_pull_request() -> bool:
    """Whether or not the running test is a CIVET pull request event."""
    return os.environ.get("CIVET_EVENT_CAUSE", "").startswith("Pull")


def is_civet_push() -> bool:
    """Whether or not the running test is a CIVET push event."""
    return os.environ.get("CIVET_EVENT_CAUSE", "").startswith("Push")
