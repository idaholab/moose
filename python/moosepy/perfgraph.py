# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

"""Implements PerfGraph for reading performance results from a database."""

from typing import Any, Callable, Iterable, Optional


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
        return sum([do(node) for node in self._nodes])

    @property
    def num_calls(self) -> int:
        """Get the number of times this was called."""
        return self._sum_all_nodes(lambda node: node._num_calls)

    @property
    def self_time(self) -> float:
        """Get the time only this (not including children) tool in seconds."""
        return self._sum_all_nodes(lambda node: node.time)

    @property
    def children_time(self) -> float:
        """Get the time the children took in seconds."""
        return self._sum_all_nodes(
            lambda node: sum([child.total_time for child in node.children])
        )

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
        parent = self._nodes[0]
        while parent.parent is not None:
            parent = parent.parent
        return parent


class PerfGraphSection(PerfGraphObject):
    """
    A section in the graph for the PerfGraph.

    These should really only be constructed internally within
    the PerfGraphReporterReader.
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
        super().__init__(name, level)

    def __str__(self):
        """Human-readable name for this section."""
        return 'PerfGraphSection "' + self.name + '"'

    @property
    def nodes(self) -> list["PerfGraphNode"]:
        """Get the nodes in this section."""
        return self._nodes

    def node(self, path: list[str]) -> Optional["PerfGraphNode"]:
        """Get the node with the given path if it exists."""
        assert isinstance(list, str)
        assert all(isinstance(v, str) for v in path)

        for node in self.nodes:
            if node.path == path:
                return node
        return None


class PerfGraphNode(PerfGraphObject):
    """
    A node in the graph for the PerfGraph.

    These should really only be constructed internally within
    the PerfGraphReporterReader.
    """

    def __init__(self, name: str, node_data: dict, parent: Optional["PerfGraphNode"]):
        """
        Initialize state.

        Arguments:
        ---------
        name : str
            Name of the node.
        node_data : dict
            Data for the node.
        parent : Optional[PerfGraphNode]
            The parent node, if any.

        """
        # A unique ID for this node
        self._id: Optional[int] = None
        # Current memory usage for the node
        self._memory: float = node_data.pop("memory")
        # Number of calls for the node
        self._num_calls: int = node_data.pop("num_calls")
        # Run time for the node
        self._time: float = node_data.pop("time")
        # Parent node, if any
        self._parent: Optional[PerfGraphNode] = parent

        super().__init__(name, node_data.pop("level"))
        self._nodes.append(self)

        # Recursively add all of the children
        self._children: dict[str, PerfGraphNode] = {
            key: PerfGraphNode(key, val, self) for key, val in node_data.items()
        }

        # Section that this node is in
        self._section: Optional["PerfGraphSection"] = None

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
    def memory(self) -> float:
        """Get the memory usage for this node."""
        assert isinstance(self._memory, float)
        return self._memory

    @property
    def time(self) -> float:
        """Get the run time for this node."""
        assert isinstance(self._time, float)
        return self._time

    @property
    def parent(self) -> Optional["PerfGraphNode"]:
        """Get the parent to this node, if any."""
        assert isinstance(self._parent, (type(None), PerfGraphNode))
        return self._parent

    @property
    def section(self) -> "PerfGraphSection":
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


class PerfGraph:
    """A Reader for MOOSE PerfGraph reporter data."""

    def __init__(self, data: dict):
        """
        Initialize state.

        Arguments:
        ---------
        data : dict
            The data from the JSON output.

        """
        # Find the root node
        root_node_name = list(data.keys())[0]
        root_node_data = data[root_node_name]

        # Setup the root node
        self._root_node: PerfGraphNode = PerfGraphNode(
            root_node_name, root_node_data, None
        )

        # Setup the node IDs and the ID -> node cache
        nodes = self._setup_nodes(self.root_node)
        # Setup the sections in each node and the section name -> section cache
        sections = self._setup_sections(nodes.values())

        # Cache for node ID -> node
        self._nodes: dict[int, PerfGraphNode] = nodes

        # Cache for section name -> section
        self._sections: dict[str, PerfGraphSection] = sections

    @staticmethod
    def recurse(node: PerfGraphNode, act: Callable[[PerfGraphNode], None]):
        """
        Recursively perform an action on each node.

        Parameters
        ----------
        node : PerfGraphNode
            The node to start recursively with.
        act : Callable[[PerfGraphNode], None]
            The action to perform on each node.

        """

        def _recurse(node: PerfGraphNode, aact: Callable[[PerfGraphNode], None]):
            act(node)
            [_recurse(child, act) for child in node.children]

        _recurse(node, act)

    @staticmethod
    def _setup_nodes(root_node: PerfGraphNode) -> dict[int, PerfGraphNode]:
        """Set an ID for each node and setup the ID to node map."""
        next_id = 0
        cache: dict[int, PerfGraphNode] = {}

        def process_node(node: PerfGraphNode):
            nonlocal next_id
            node._id = next_id
            next_id += 1
            cache[node.id] = node

        PerfGraph.recurse(root_node, process_node)

        return cache

    @staticmethod
    def _setup_sections(nodes: Iterable[PerfGraphNode]) -> dict[str, PerfGraphSection]:
        """Build sections, setup node sections, and setup the name to section map."""
        sections = {}
        for node in nodes:
            section = sections.get(node.name)
            if section is None:
                section = PerfGraphSection(node.name, node.level)
                sections[node.name] = section
            node._section = section
            section._nodes.append(node)
        return sections

    @property
    def root_node(self) -> PerfGraphNode:
        """Get the root PerfGraphNode."""
        return self._root_node

    @property
    def nodes(self) -> Iterable[PerfGraphNode]:
        """Get all of the nodes."""
        return self._nodes.values()

    @property
    def sections(self) -> Iterable[PerfGraphSection]:
        """Get all of the named sections."""
        return self._sections.values()

    @property
    def total_time(self) -> float:
        """Get the total time."""
        return self.root_node.total_time

    def get_node_by_id(self, id: int) -> PerfGraphNode:
        """Get the PerfGraphNode with the given ID."""
        if (node := self._nodes.get(id)) is not None:
            return node
        raise KeyError(f"Node does not exist with ID {id}")

    def has_section(self, name: str) -> bool:
        """Whether or not a section with the given name exists."""
        return name in self._sections

    def get_section(self, name: str) -> PerfGraphSection:
        """Get a named PerfGraphSection."""
        assert isinstance(name, str)
        if (section := self._sections.get(name)) is not None:
            return section
        raise KeyError(f"Section does not exist with name '{name}'")

    def get_heaviest_nodes(self, num: int) -> list[PerfGraphNode]:
        """
        Get the heaviest nodes in the graph.

        Parameters
        ----------
        num : int
            The number of nodes to return.

        """
        assert isinstance(num, int)
        assert num > 0
        return sorted(self.nodes, key=lambda n: n.self_time, reverse=True)[0:num]

    def get_heaviest_sections(self, num: int) -> list[PerfGraphSection]:
        """
        Get the heaviest sections in the graph.

        Parameters
        ----------
        num : int
            The number of sections to return.

        """
        assert isinstance(num, int)
        assert num > 0

        return sorted(
            self.sections,
            key=lambda s: s.self_time,
            reverse=True,
        )[0:num]
