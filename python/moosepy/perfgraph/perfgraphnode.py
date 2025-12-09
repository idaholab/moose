# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

"""Implements PerfGraphNode, which represents a node in the MOOSE PerfGraph."""

from typing import Optional

from moosepy.perfgraph.perfgraphsection import PerfGraphSection


class PerfGraphNode:
    """
    A node in the graph for the MOOSE PerfGraph.

    Should be constructed internally in the PerfGraph object.
    """

    def __init__(
        self,
        id: int,
        name: str,
        time: float,
        num_calls: int,
        section: PerfGraphSection,
        parent: Optional["PerfGraphNode"],
    ):
        """
        Initialize state.

        Arguments:
        ---------
        id : int
            Unique ID for the node.
        name : str
            Name of the node.
        time : float
            The time spent in the node across all calls.
        num_calls : int
            The number of calls to the node.
        section : PerfGraphSection
            The section the node is in.
        parent : Optional[PerfGraphNode]
            The parent node, if any.

        """
        self._id: int = id
        """A unique ID for this node."""
        self._name: str = name
        """The name for this node."""
        self._self_time: float = time
        """Self time for this node in seconds."""
        self._num_calls: int = num_calls
        """The number of calls to this node."""
        self._section: PerfGraphSection = section
        """Section that this node is in."""
        self._parent: Optional[PerfGraphNode] = parent
        """Parent node, if any."""
        self._children: list[PerfGraphNode] = []
        """The children to this node."""

        # Add to the parent
        if self._parent is not None:
            self._parent._add_child(self)
        # Add to the section
        self._section._add_node(self)

    def __str__(self) -> str:
        """Human-readable name for this node."""
        return f"PerfGraphNode {self.name}"

    @property
    def id(self) -> int:
        """Get a unique ID for this node."""
        assert isinstance(self._id, int)
        return self._id

    @property
    def name(self) -> str:
        """Get the name of this node."""
        assert isinstance(self._name, str)
        return self._name

    @property
    def self_time(self) -> float:
        """Get the self time for this node in seconds."""
        assert isinstance(self._self_time, float)
        return self._self_time

    @property
    def num_calls(self) -> int:
        """Get the number of calls to the node."""
        assert isinstance(self._num_calls, int)
        return self._num_calls

    @property
    def section(self) -> PerfGraphSection:
        """Get the section this node is in."""
        assert isinstance(self._section, PerfGraphSection)
        return self._section

    @property
    def parent(self) -> Optional["PerfGraphNode"]:
        """Get the parent to this node, if any."""
        assert isinstance(self._parent, (type(None), PerfGraphNode))
        return self._parent

    @property
    def children(self) -> list["PerfGraphNode"]:
        """Get the children of this node."""
        return self._children

    @property
    def level(self) -> int:
        """Get the level of the section the node is in."""
        return self.section.level

    @property
    def children_time(self) -> float:
        """Get the total children time for this node in seconds."""
        return sum([c.total_time for c in self.children], 0.0)

    @property
    def total_time(self) -> float:
        """Get the total time (self + children) for this node in seconds."""
        return self.self_time + self.children_time

    @property
    def self_percent_time(self) -> float:
        """Get the percentage of self time for this node relative to the total."""
        return self.self_time * 100 / self.root_node.total_time

    @property
    def children_percent_time(self) -> float:
        """Get the percentage of children time for this node relative to the total."""
        return self.children_time * 100 / self.root_node.total_time

    @property
    def total_percent_time(self) -> float:
        """Get the percentage of children time for this node relative to the total."""
        return self.total_time * 100 / self.root_node.total_time

    @property
    def root_node(self) -> "PerfGraphNode":
        """Get the root node (the top node in the graph)."""
        parent = self.parent
        if parent is None:
            return self
        while parent.parent is not None:
            parent = parent.parent
        return parent

    @property
    def path(self) -> list[str]:
        """Get the full path in the graph for this node."""
        names = [self.name]
        parent = self
        while parent.parent is not None:
            names.append(parent.parent.name)
            parent = parent.parent
        return names[::-1]

    def _add_child(self, node: "PerfGraphNode"):
        """
        Add a child.

        Used internally within PerfGraphNode.__init__().
        """
        assert node not in self._children
        assert node.parent == self
        self._children.append(node)

    def query_child(self, name: str) -> Optional["PerfGraphNode"]:
        """Query a child by name."""
        assert isinstance(name, str)
        return next((c for c in self.children if c.name == name), None)

    def has_child(self, name: str) -> bool:
        """Whether or not a child exists with the given name."""
        return self.query_child(name) is not None

    def get_child(self, name: str) -> "PerfGraphNode":
        """Get the child with the given name if it exists."""
        if (child := self.query_child(name)) is not None:
            return child
        raise KeyError(f"Child with name '{name}' does not exist")

    def get_path_to_root(self) -> tuple[set, set]:
        """Get set of edges and nodes along the path for given node to root."""
        path_edges = set()
        nodes_to_include = set()

        current = self
        while current:
            nodes_to_include.add(current)
            if current.parent:
                path_edges.add((current.parent.id, current.id))
                nodes_to_include.add(current.parent)
            current = current.parent
        return path_edges, nodes_to_include
