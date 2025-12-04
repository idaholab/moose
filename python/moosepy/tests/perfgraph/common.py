# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

"""Common testing utilities for testing moosepy.prefgraph."""

from copy import deepcopy
from typing import Optional

from moosepy.perfgraph.perfgraphnode import PerfGraphNode
from moosepy.perfgraph.perfgraphsection import PerfGraphSection

SECTION_KWARGS = {"name": "dummy_section", "level": 1}
"""Default keyword arguments for building a test section."""


def build_test_section(
    nodes: Optional[list[PerfGraphNode]] = None, **kwargs
) -> PerfGraphSection:
    """
    Build a PerfGraphSection for testing.

    Optional Parameters
    -------------------
    nodes : Optional[list[PerfGraphNode]]
        List of nodes to add to the section.
    **kwargs :
        Keyword arguments that will override init arguments for the section.

    """
    section_kwargs = deepcopy(SECTION_KWARGS)
    section_kwargs.update(kwargs)
    section = PerfGraphSection(**section_kwargs)
    if nodes:
        [section._add_node(n) for n in nodes]
    return section


def build_node_data(**kwargs) -> dict:
    """
    Build node data for testing.

    Optional Parameters
    -------------------
    **kwargs :
        Keyword arguments that will override the data.
    """
    data = {"time": 1.234, "num_calls": 5, "level": 1, "memory": 5.678}
    data.update(kwargs)
    return data


NODE_KWARGS = {
    "id": 0,
    "name": "dummy_node",
    "time": 1.234,
    "num_calls": 5,
    "section": build_test_section(),
    "parent": None,
}
"""Default keyword arguments for building a test node."""


def build_test_node(**kwargs) -> PerfGraphNode:
    """
    Build a PerfGraphNode for testing.

    Optional Parameters
    -------------------
    **kwargs :
        Keyword arguments that will override init arguments for the section.

    """
    node_kwargs = deepcopy(NODE_KWARGS)
    node_kwargs.update(kwargs)
    node = PerfGraphNode(**node_kwargs)
    return node
