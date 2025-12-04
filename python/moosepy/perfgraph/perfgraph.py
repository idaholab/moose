# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

"""Implements PerfGraph for representing MOOSE PerfGraphReporter data."""

from typing import Callable, Iterable, Optional, Tuple

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
    def _parse_node_data(data: dict) -> Tuple[dict, int, list[Tuple[str, dict]]]:
        """
        Parse the data for a single node.

        Parameters
        ----------
        data : dict
            The node data.

        Returns
        -------
        dict :
            The data for this node.
        int :
            The node section level.
        list[Tuple[str, dict]]] :
            The child data.

        """
        # Pull out data specific to the node
        node_data = {k: data.pop(k) for k in ["time", "num_calls", "memory"]}

        # Section data
        level = data.pop("level")

        # And children (the rest of the keys)
        children_data = [
            (child_name, child_data) for child_name, child_data in data.items()
        ]

        return node_data, level, children_data

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

            node_data, level, child_data = PerfGraph._parse_node_data(data)

            # Memory currently unused
            node_data.pop("memory")

            # Get the node section or build it if needed
            section = sections.get(name)
            if section is None:
                section = PerfGraphSection(name, level)
                sections[name] = section

            # Build the node
            node = PerfGraphNode(
                id=id,
                name=name,
                **node_data,
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
    def num_nodes(self) -> int:
        """Get the number of nodes."""
        return len(self._nodes)

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

    def query_section(self, name: str) -> Optional[PerfGraphSection]:
        """Query a section by name."""
        assert isinstance(name, str)
        if (section := self._sections.get(name)) is not None:
            return section
        return None

    def has_section(self, name: str) -> bool:
        """Whether or not a section with the given name exists."""
        return self.query_section(name) is not None

    def get_section(self, name: str) -> PerfGraphSection:
        """Get a section by name."""
        assert isinstance(name, str)
        if (section := self.query_section(name)) is not None:
            return section
        raise KeyError(f"Section does not exist with name '{name}'")

    def recurse(
        self, act: Callable[[PerfGraphNode], None], node: Optional[PerfGraphNode] = None
    ):
        """
        Recursively perform on action on the whole tree.

        Parameters
        ----------
        act : Callable[[PerfGraphNode], None]
            The action to perform on a single node.

        Additional Parameters
        ---------------------
        node : Optional[PerfGraphNode]
            The node to start with; defaults to the root if None.

        """
        assert isinstance(act, Callable)
        assert isinstance(node, (type(None), PerfGraphNode))

        def recurse(node: PerfGraphNode):
            act(node)
            for child in node.children:
                recurse(child)

        recurse(node if node is not None else self.root_node)

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
