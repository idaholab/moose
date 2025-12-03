# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

"""Common testing utilities for testing moosepy.prefgraph."""

from moosepy.perfgraph import PerfGraphNode


class DummyNode(PerfGraphNode):
    """Dummy PerfGraphNode for testing."""

    def __init__(self, **kwargs):
        """Fake init; does nothing."""
        self.__dict__["_name"] = "dummy_node"
        self.__dict__["_nodes"] = [self]
        for k, v in kwargs.items():
            self.__dict__[k] = v
