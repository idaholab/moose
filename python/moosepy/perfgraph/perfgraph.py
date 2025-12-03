# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

"""Implements PerfGraph for representing MOOSE PerfGraphReporter data."""

from typing import Callable, Iterable

from moosepy.perfgraph.perfgraphnode import PerfGraphNode
from moosepy.perfgraph.perfgraphsection import PerfGraphSection


class PerfGraph:
    """Representation of the PerfGraph from MOOSE PerfGraphReporter data."""

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
