# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

"""Implements PerfGraphSection, which represents a section in the MOOSE PerfGraph."""

from typing import TYPE_CHECKING, Optional

if TYPE_CHECKING:
    from moosepy.perfgraph.perfgraphnode import PerfGraphNode


class PerfGraphSection:
    """
    A section in the graph for the PerfGraph.

    Should be constructed internally in the PerfGraph object.
    """

    def __init__(self, name: str, level: int):
        """
        Initialize state.

        Arguments:
        ---------
        name : str
            The name of the section.
        level : int
            The level for this section.

        """
        # The name for this section
        self._name: str = name
        # The level for this section
        self._level: int = level
        # The nodes in this section
        self._nodes: list["PerfGraphNode"] = []

    def __str__(self):
        """Human-readable name for this section."""
        return 'PerfGraphSection "' + self.name + '"'

    @property
    def name(self) -> str:
        """Get the name of this section."""
        assert isinstance(self._name, str)
        return self._name

    @property
    def level(self) -> int:
        """Get the level of this section."""
        assert isinstance(self._level, int)
        return self._level

    @property
    def nodes(self) -> list["PerfGraphNode"]:
        """Get the nodes in this section."""
        assert len(self._nodes) > 0
        return self._nodes

    @property
    def num_calls(self) -> int:
        """Get the number of times section was called across all nodes."""
        return sum([n.num_calls for n in self.nodes], 0)

    @property
    def self_time(self) -> float:
        """Get the self time in this section across all nodes in seconds."""
        return sum([n.self_time for n in self.nodes], 0.0)

    @property
    def children_time(self) -> float:
        """Get the children time in this section across all nodes in seconds."""
        return sum([n.children_time for n in self.nodes], 0.0)

    @property
    def total_time(self) -> float:
        """Get the total time in this section across all nodes in seconds."""
        return self.self_time + self.children_time

    @property
    def self_percent_time(self) -> float:
        """Get the self percentage time across all nodes."""
        return sum([n.self_percent_time for n in self.nodes], 0.0)

    @property
    def children_percent_time(self) -> float:
        """Get the children percentage time across all nodes."""
        return sum([n.children_percent_time for n in self.nodes], 0.0)

    @property
    def total_percent_time(self) -> float:
        """Get the total percentage time across all nodes."""
        return sum([n.total_percent_time for n in self.nodes], 0.0)

    def _add_node(self, node: "PerfGraphNode"):
        """
        Add a node to the section.

        Used internally within PerfGraphNode.__init__().
        """
        assert node not in self._nodes
        self._nodes.append(node)

    def query_node(self, path: list[str]) -> Optional["PerfGraphNode"]:
        """Query a node in the section by path."""
        assert isinstance(path, list)
        assert all(isinstance(v, str) for v in path)
        return next((node for node in self.nodes if node.path == path), None)

    def has_node(self, path: list[str]) -> bool:
        """Whether or not the section has the node by path."""
        return self.query_node(path) is not None

    def get_node(self, path: list[str]) -> "PerfGraphNode":
        """Get a node in the section by path."""
        if (node := self.query_node(path)) is not None:
            return node
        raise KeyError(f"Node with path {path} does not exist")
