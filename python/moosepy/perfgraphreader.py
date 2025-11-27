# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

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

    # def info(self):
    #     """Get the number of calls, time, and memory in human readable form."""
    #     info_str = f"Num calls: {self.num_calls}"
    #     info_str += f"\nLevel: {self.level}"
    #     info_str += (
    #         "\nTime ({:.2f}%): Self {:.2f} s, Children {:.2f} s, Total {:.2f} s".format(
    #             self.percentTime(),
    #             self.selfTime(),
    #             self.childrenTime(),
    #             self.totalTime(),
    #         )
    #     )
    #     info_str += (
    #         "\nMemory ({:.2f}%): Self {} MB, Children {} MB, Total {} MB".format(
    #             self.percentMemory(),
    #             self.selfMemory(),
    #             self.childrenMemory(),
    #             self.totalMemory(),
    #         )
    #     )
    #     return info_str

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
            lambda node: sum([child.total_time for child in node.children.values()])
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
    A section in the graph for the PerfGraphReader.

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

    # def info(self):
    #     info_str = 'PerfGraphSection "' + self.name() + '":'
    #     info_str += "\n  " + super().info().replace("\n", "\n  ")
    #     info_str += "\n  Nodes:"
    #     for node in self.nodes():
    #         for i in range(len(node.path())):
    #             info_str += "\n    " + ("- " if i == 0 else "  ")
    #             info_str += " " * i + node.path()[i]
    #             if i == len(node.path()) - 1:
    #                 info_str += " ({} call(s), {:.1f}% time, {:.1f}% memory)".format(
    #                     node.numCalls(), node.percentTime(), node.percentMemory()
    #                 )
    #     return info_str

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
    A node in the graph for the PerfGraphReader.

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

    def __str__(self):
        """Human-readable name for this node."""
        return 'PerfGraphNode "' + "/".join(self.path) + '"'

    def __getitem__(self, name: str) -> Optional["PerfGraphNode"]:
        """Get a child node by name, if it exists."""
        return self.child(name)

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
    def children(self) -> dict[str, "PerfGraphNode"]:
        """Get the name, node pairs for the immediate children."""
        return self._children

    @property
    def path(self) -> list[str]:
        """Get the full path in the graph for this node."""
        names = [self.name]
        parent = self
        while parent.parent is not None:
            names.append(parent.parent.name)
            parent = parent.parent
        return names[::-1]

    def child(self, name: str) -> Optional["PerfGraphNode"]:
        """Get the child with the given name if it exists."""
        assert isinstance(name, str)
        return self._children.get(name)

    # def info(self):
    #     """
    #     Returns the number of calls, the time, memory,
    #     and children in a human readable form.
    #     """
    #     info_str = "PerfGraphNode\n"
    #     info_str += "  Path:\n"
    #     for i in range(0, len(self.path())):
    #         info_str += "    " + " " * i + self.path()[i] + "\n"
    #     info_str += "  " + super().info().replace("\n", "\n  ")
    #     if self.children():
    #         info_str += "\n  Children:"
    #         for child in self.children():
    #             info_str += (
    #                 "\n    "
    #                 + child.name()
    #                 + " ({} call(s), {:.1f}% time, {:.1f}% memory)".format(
    #                     child.numCalls(), child.percentTime(), child.percentMemory()
    #                 )
    #             )
    #     return info_str


class PerfGraphReader:
    """A Reader for MOOSE PerfGraphReporter data."""

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

        # Setup all of the sections
        self._sections: dict[str, PerfGraphSection] = {}

        # Recursively setup all of the sections
        def add_section(node: PerfGraphNode):
            section = self._sections.get(node.name)
            if section is None:
                section = PerfGraphSection(node.name, node.level)
                self._sections[node.name] = section
            node._section = section
            section._nodes.append(node)

        self.recurse(add_section)

    @property
    def root_node(self) -> PerfGraphNode:
        """Get the root PerfGraphNode."""
        return self._root_node

    def recurse(self, act: Callable, *args, **kwargs):
        """
        Recursively do an action through the graph starting with the root node.

        Inputs:

        - act\[function\]: Action to perform on each node (input: a PerfGraphNode)
        """

        def _recurse(node: PerfGraphNode, act: Callable, *args, **kwargs):
            act(node, *args, **kwargs)
            for child in node.children.values():
                _recurse(child, act, *args, **kwargs)

        _recurse(self.root_node, act, *args, **kwargs)

    def node(self, path: list[str]):
        """Get the node with the given path if one exists, otherwise None."""
        assert isinstance(list, str)
        assert all(isinstance(v, str) for v in path)

        if len(path) == 0 or path[0] != self.root_node.name:
            return None
        node = self.root_node
        for name in path[1:]:
            if node:
                node = node[name]
        return node

    @property
    def sections(self) -> Iterable[PerfGraphSection]:
        """Get all of the named sections."""
        return self._sections.values()

    def section(self, name: str) -> Optional[PerfGraphSection]:
        """Get a named PerfGraphSection if it exists."""
        assert isinstance(name, str)
        return self._sections.get(name)
