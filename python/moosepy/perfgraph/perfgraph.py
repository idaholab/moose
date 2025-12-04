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
from typing import Callable, Iterable, Optional

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

        # Setup the data
        setup_data = self._setup(data)
        root_data = setup_data.root_data

        # The nodes in the graph; node id -> node
        self._nodes: dict[int, PerfGraphNode] = setup_data.nodes
        # The sections in the graph; section name -> section
        self._sections: dict[str, PerfGraphSection] = setup_data.sections
        # The PerfGraphReporter version
        self._version: int = root_data.version
        # Maximum memory in MB for this rank, if available
        self._max_memory_this_rank: Optional[int] = root_data.max_memory_this_rank
        # Maximum memory per rank in MB, if available
        self._max_memory_per_rank: Optional[list[int]] = root_data.max_memory_per_rank

        # The root node in the graph
        self._root_node: PerfGraphNode = self._nodes[0]
        assert self._root_node.parent is None

    @dataclass
    class RootData:
        """Helper dataclass for parsing the root data."""

        # PerfGraphReporter schema version; from "version" key.
        version: int = 0
        # Name of the root node
        root_node_name: str = ""
        # Data for the root node
        root_node_data: dict = field(default_factory=dict)
        # Max memory for this rank; from "max_memory_this_rank" key.
        max_memory_this_rank: Optional[int] = None
        # Max memory for all ranks; from "max_memory_per_rank" key.
        max_memory_per_rank: Optional[list[int]] = None

    @staticmethod
    def _parse_root_data(data: dict) -> RootData:
        """Parse the root data (the main data structure)."""
        assert isinstance(data, dict)

        root_data = PerfGraph.RootData()

        # Graph entry
        graph: dict = data.pop("graph")
        assert isinstance(graph, dict)
        assert len(graph) > 0

        # Version, which is 0 if version doesn't exist
        root_data.version = data.pop("version", 0)
        assert isinstance(root_data.version, int)

        # Max memory options added in version 1
        if root_data.version > 0:
            # Will exist on every rank
            root_data.max_memory_this_rank = data.pop("max_memory_this_rank")
            assert isinstance(root_data.max_memory_this_rank, int)

            # Will only exist if we have data from rank 0
            if "max_memory_per_rank" in data:
                root_data.max_memory_per_rank = data.pop("max_memory_per_rank")
                assert isinstance(root_data.max_memory_per_rank, list)
                assert all(isinstance(v, int) for v in root_data.max_memory_per_rank)

        # Should have no data left
        assert not data

        # Root node
        root_data.root_node_name = next(iter(graph))
        root_data.root_node_data = graph[root_data.root_node_name]

        return root_data

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

    @dataclass
    class SetupData:
        """Helper dataclass for the data that is setup."""

        # The root data.
        root_data: "PerfGraph.RootData"
        # Mapping of node ID to node.
        nodes: dict[int, PerfGraphNode] = field(default_factory=dict)
        # Mapping of section name to section.
        sections: dict[str, PerfGraphSection] = field(default_factory=dict)

    @staticmethod
    def _setup(
        data: dict,
    ) -> SetupData:
        """
        Build the nodes and sections from the data.

        Parameters
        ----------
        data : dict
            The data from the PerfGraphReporter for the desired timestep.

        """
        root_data = PerfGraph._parse_root_data(data)
        setup_data = PerfGraph.SetupData(root_data)

        # For setting node IDs
        next_id: int = 0

        # Recursive function for processing a single node
        def process_data(name: str, data: dict, parent: Optional[PerfGraphNode]):
            # Determine ID for the node
            nonlocal next_id
            id = next_id
            next_id += 1

            node_data = PerfGraph._parse_node_data(data, root_data.version)

            # Get the node section or build it if needed
            section = setup_data.sections.get(name)
            if section is None:
                section = PerfGraphSection(name, node_data.level)
                setup_data.sections[name] = section
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
            setup_data.nodes[node.id] = node

            # Recursively add children nodes
            for child_name, child_data in node_data.children.items():
                process_data(child_name, child_data, node)

        # Recursively add all nodes
        process_data(root_data.root_node_name, root_data.root_node_data, None)

        return setup_data

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
        return self._version

    @property
    def max_memory(self) -> Optional[int]:
        """Get the max memory for this rank, if available."""
        return (
            self._max_memory_this_rank
            if self._max_memory_this_rank is not None
            else None
        )

    @property
    def max_memory_per_rank(self) -> Optional[list[int]]:
        """Get the max memory per rank, if available."""
        return (
            self._max_memory_per_rank if self._max_memory_per_rank is not None else None
        )

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
