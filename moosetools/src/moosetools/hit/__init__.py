# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

"""
Interface to the MOOSE hit contrib.

This module will attempt to build the moosetools.hit.hit
binding library if it has not already been built.
"""

# First try to import the hit library if it already exists
try:
    from .hit import Token, TokenType
    from .pyhit import Node, load, parse, tokenize, write
# Otherwise, setup (build) it in tree
except ImportError:  # pragma: no cover
    from ._setup_external import setup_external

    setup_external()

    from .hit import Token, TokenType
    from .pyhit import Node, load, parse, tokenize, write

__all__ = ["TokenType", "Token", "Node", "load", "write", "parse", "tokenize"]
