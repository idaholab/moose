# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

"""Implements PerfGraph for representing MOOSE PerfGraphReporter data."""

from dataclasses import dataclass, field
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
        assert isinstance(data, dict)
        assert all(isinstance(v, str) for v in data)

        # Build the nodes and sections from the data
        nodes, sections, version = self._setup(data)

        # The nodes in the graph; node id -> node
        self._nodes: dict[int, PerfGraphNode] = nodes
        # The sections in the graph; section name -> section
        self._sections: dict[str, PerfGraphSection] = sections
        # The PerfGraphReporter version
        self._version: int = version
        # The root node in the graph
        self._root_node: PerfGraphNode = self._nodes[0]
        assert self._root_node.parent is None

    @dataclass
    class NodeData:
        """Helper dataclass for parsing node data."""

        # The data for this node.
        data: dict = field(default_factory=dict)
        # The level for this node's section.
        level: int = 0
        # Children in this node (if any); name -> data.
        children: dict[str, dict] = field(default_factory=dict)

    @staticmethod
    def _parse_node_data(data: dict, version: int) -> NodeData:
        """
        Parse the data for a single node.

        Parameters
        ----------
        data : dict
            The node data.
        version : int
            The PerfGraphReporter version.

        """
        assert isinstance(data, dict)
        assert isinstance(version, int)

        node_data = PerfGraph.NodeData()

        # Remove the memory entry; not used
        if version == 0:
            data.pop("memory")

        # Pull out data specific to the node
        node_data.data = {k: data.pop(k) for k in ["time", "num_calls"]}

        # Section data
        node_data.level = data.pop("level")

        # Children in version 0, contained within the root
        # (the rest of the keys)
        if version == 0:
            node_data.children = data
        # After version 0, stored separately in "children" key
        else:
            if "children" in data:
                node_data.children = data.pop("children")
                assert node_data.children
            assert not data

        assert isinstance(node_data.children, dict)
        assert all(isinstance(v, str) for v in node_data.children)
        assert all(isinstance(v, dict) for v in node_data.children.values())

        return node_data

    @staticmethod
    def _setup(
        data: dict,
    ) -> Tuple[dict[int, PerfGraphNode], dict[str, PerfGraphSection], int]:
        """
        Build the nodes and sections from the data.

        Parameters
        ----------
        data : dict
            The data from the PerfGraphReporter for the desired timestep.

        Returns
        -------
        dict[int, PerfGraphNode] :
            Mapping of node ID to node.
        dict[str, PerfGraphSection] :
            Mapping of section name to section.
        int :
            The PerfGraphReporter version.

        """
        assert isinstance(data, dict)

        graph: dict = data["graph"]
        assert isinstance(graph, dict)
        assert len(graph) > 0

        version: int = data.get("version", 0)
        assert isinstance(version, int)

        # Find the root node
        root_node_name = next(iter(graph))
        root_node_data = graph[root_node_name]

        next_id: int = 0
        nodes: dict[int, PerfGraphNode] = {}
        sections: dict[str, PerfGraphSection] = {}

        # Recursive function for processing a single node
        def process_data(name: str, data: dict, parent: Optional[PerfGraphNode]):
            # Determine ID for the node
            nonlocal next_id
            id = next_id
            next_id += 1

            node_data = PerfGraph._parse_node_data(data, version)

            # Get the node section or build it if needed
            section = sections.get(name)
            if section is None:
                section = PerfGraphSection(name, node_data.level)
                sections[name] = section
            else:
                assert section.level == node_data.level

            # Build the node
            node = PerfGraphNode(
                id=id,
                name=name,
                **node_data.data,
                section=section,
                parent=parent,
            )
            nodes[node.id] = node

            # Recursively add children nodes
            for child_name, child_data in node_data.children.items():
                process_data(child_name, child_data, node)

        # Recursively add all nodes
        process_data(root_node_name, root_node_data, None)

        return nodes, sections, version

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
    def version(self) -> int:
        """Get the PerfGraphReporter version for this data."""
        assert isinstance(self._version, int)
        return self._version

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
