# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

"""
Build the external dependencies for moosetools.capabilities.

This is used within:
    - moosetools.hit.__init__
    - the moosetools_hit target in moose.mk
"""


def setup_external():
    """Build the external libraries for moosetools.capabilities."""
    try:
        from moosetools._internal.setup_external import setup_external
    except ImportError:
        import os
        import sys

        moosetools_dir = os.path.abspath(
            os.path.join(os.path.dirname(__file__), "../..")
        )
        if moosetools_dir not in sys.path:
            sys.path.insert(0, os.path.join(os.path.dirname(__file__), "../.."))
        from moosetools._internal.setup_external import setup_external

    setup_external("setup_hit_external.py", "hit/hit")


if __name__ == "__main__":
    setup_external()
