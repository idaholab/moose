# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

"""Implements PerfGraphNode, which represents a node in the MOOSE PerfGraph."""

from typing import Iterable, Optional

from moosepy.perfgraph.perfgraphobject import PerfGraphObject
from moosepy.perfgraph.perfgraphsection import PerfGraphSection


class PerfGraphNode(PerfGraphObject):
    """
    A node in the graph for the MOOSE PerfGraph.

    Should be constructed internally in the PerfGraph object.
    """

    def __init__(self, name: str, data: dict, parent: Optional["PerfGraphNode"]):
        """
        Initialize state.

        Arguments:
        ---------
        name : str
            Name of the node.
        data : dict
            Data for the node.
        parent : Optional[PerfGraphNode]
            The parent node, if any.

        """
        # A unique ID for this node
        self._id: Optional[int] = None
        # The data for this node
        self._data: dict = data
        # Parent node, if any
        self._parent: Optional[PerfGraphNode] = parent

        # Initialize the PerfGraphObject
        super().__init__(name, data["level"])

        # Setup the nodes for the PerfGraphObject, which, for
        # a single node is just this node
        self._nodes.append(self)

        # Separate out data for children
        child_data = {
            k: v
            for k, v in data.items()
            if k not in ["memory", "time", "num_calls", "level"]
        }

        # Recursively add all of the children
        self._children: dict[str, PerfGraphNode] = {
            key: PerfGraphNode(key, val, self) for key, val in child_data.items()
        }

        # Section that this node is in
        self._section: Optional[PerfGraphSection] = None

    def __str__(self) -> str:
        """Human-readable name for this node."""
        return f"PerfGraphNode {self.name}"

    def __getitem__(self, name: str) -> "PerfGraphNode":
        """Get a child node by name."""
        return self.get_child(name)

    @property
    def id(self) -> int:
        """Get a unique ID for this node."""
        assert isinstance(self._id, int)
        return self._id

    @property
    def data(self) -> dict:
        """Get the underlying data for the node."""
        return self._data

    @property
    def parent(self) -> Optional["PerfGraphNode"]:
        """Get the parent to this node, if any."""
        assert isinstance(self._parent, (type(None), PerfGraphNode))
        return self._parent

    @property
    def memory(self) -> float:
        """Get the memory usage for this node."""
        value = self.data["memory"]
        assert isinstance(value, float)
        return value

    @property
    def time(self) -> float:
        """Get the run time for this node."""
        value = self.data["time"]
        assert isinstance(value, float)
        return value

    @property
    def section(self) -> PerfGraphSection:
        """Get the section this node is in."""
        assert self._section is not None
        return self._section

    @property
    def children(self) -> Iterable["PerfGraphNode"]:
        """Get the children of this node."""
        return self._children.values()

    @property
    def path(self) -> list[str]:
        """Get the full path in the graph for this node."""
        names = [self.name]
        parent = self
        while parent.parent is not None:
            names.append(parent.parent.name)
            parent = parent.parent
        return names[::-1]

    def get_child(self, name: str) -> "PerfGraphNode":
        """Get the child with the given name if it exists."""
        assert isinstance(name, str)
        if (child := self._children.get(name)) is not None:
            return child
        raise KeyError(f"Child with name '{name}' does not exist")
