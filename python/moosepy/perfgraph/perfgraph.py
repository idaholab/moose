# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

"""Implements PerfGraph for representing MOOSE PerfGraphReporter data."""

from typing import Iterable, Optional, Tuple

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
        # Build the nodes and sections from the data
        nodes, sections = self._setup_nodes(data)

        # The nodes in the graph; node id -> node
        self._nodes: dict[int, PerfGraphNode] = nodes
        # The sections in the graph; section name -> section
        self._sections: dict[str, PerfGraphSection] = sections
        # The root node in the graph
        self._root_node: PerfGraphNode = self._nodes[0]

    @staticmethod
    def _setup_nodes(
        data: dict,
    ) -> Tuple[dict[int, PerfGraphNode], dict[str, PerfGraphSection]]:
        """Build the nodes and sections from the data."""
        # Find the root node
        root_node_name = list(data.keys())[0]
        root_node_data = data[root_node_name]

        next_id: int = 0
        nodes: dict[int, PerfGraphNode] = {}
        sections: dict[str, PerfGraphSection] = {}

        # Recursive function for processing a single node
        def process_data(name: str, data: dict, parent: Optional[PerfGraphNode]):
            # Determine ID for the node
            nonlocal next_id
            id = next_id
            next_id += 1

            # Pull data that pertains to the node
            self_time = data.pop("time")
            num_calls = data.pop("num_calls")
            level = data.pop("level")
            # Currently unused
            data.pop("memory")

            # Get the node section or build it if needed
            section = sections.get(name)
            if section is None:
                section = PerfGraphSection(name, level)
                sections[name] = section

            # Build the node
            node = PerfGraphNode(
                id=id,
                name=name,
                self_time=self_time,
                num_calls=num_calls,
                section=section,
                parent=parent,
            )
            nodes[node.id] = node

            # Recursively add children nodes
            for child_name, child_data in data.items():
                process_data(child_name, child_data, node)

        # Recursively add all nodes
        process_data(root_node_name, root_node_data, None)

        return nodes, sections

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
        """Get the node with the given ID."""
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
