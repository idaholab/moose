# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

"""Implements the base PerfGraphObject for MOOSE PerfGraph nodes and sections."""

from typing import TYPE_CHECKING, Any, Callable

if TYPE_CHECKING:
    from moosepy.perfgraph.perfgraphnode import PerfGraphNode


class PerfGraphObject:
    """
    Base class PerfGraphNode and PerfGraphSection.

    This allows the interface for these two objects to be
    similar and reduces duplication.
    """

    def __init__(self, name: str, level: int):
        """
        Initialize state.

        Arguments:
        ---------
        name : str
            The section name.
        level : int
            The section level.

        """
        # The section name
        self._name: str = name
        # The section level
        self._level: int = level

        # The nodes associated with this object. For a PerfGraphNode,
        # this is a single node. For a PerfGraphSection this is one or
        # more nodes in said section.
        self._nodes: list["PerfGraphNode"] = []

    @property
    def name(self) -> str:
        """Get the name assigned to the object."""
        assert isinstance(self._name, str)
        return self._name

    @property
    def level(self) -> int:
        """Get the level assigned to the section."""
        assert isinstance(self._level, int)
        return self._level

    def _sum_all_nodes(self, do: Callable[["PerfGraphNode"], Any]) -> Any:
        """Sum an action across all nodes."""
        assert len(self._nodes) > 0
        return sum([do(node) for node in self._nodes])

    @property
    def num_calls(self) -> int:
        """Get the number of times this was called."""
        value = self._sum_all_nodes(lambda n: n.data["num_calls"])
        return value

    @property
    def self_time(self) -> float:
        """Get the time only this (not including children) tool in seconds."""
        value = self._sum_all_nodes(lambda n: n.time)
        assert isinstance(value, float)
        return value

    @property
    def children_time(self) -> float:
        """Get the time the children took in seconds."""
        value = self._sum_all_nodes(lambda n: sum([c.total_time for c in n.children]))
        assert isinstance(value, float)
        return value

    @property
    def total_time(self) -> float:
        """Get the time this plus its children took in seconds."""
        return self.self_time + self.children_time

    @property
    def percent_time(self):
        """Get the percentage of time this took relative to the total time."""
        return self.total_time * 100 / self.root_node.total_time

    @property
    def root_node(self) -> "PerfGraphNode":
        """Get the root node (the top node in the graph)."""
        assert len(self._nodes) > 0
        parent = self._nodes[0]
        while parent.parent is not None:
            parent = parent.parent
        return parent
