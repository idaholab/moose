#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from mooseutils.ReporterReader import ReporterReader

class PerfGraphNode:
    """
    A node in the graph for the PerfGraphReporterReader.
    These should really only be constructed internally within
    the PerfGraphReporterReader.

    Inputs:
        id[int]: The ID in the JSON graph
        parents[PerfGraphNode]: The node parent (None if root)
        data[dict]: The object that contains the full graph
    """
    def __init__(self, id, parent, data):
        node_data = None
        for entry in data:
            if 'id' not in entry or not isinstance(entry['id'], int):
                raise Exception('Entry missing ID\n{}'.format(entry))
            if entry['id'] == id:
                if node_data:
                    raise Exception('Duplicate ID {} found'.format(id))
                node_data = entry
        if node_data is None:
            raise Exception('Failed to find node with ID {}'.format(id))

        # Validate that the data in the node is as we expect
        self._validateNodeData(node_data)

        # Sets self._id, self._level, self._memory, etc...
        for key, val in node_data.items():
            if key not in ['parent_id', 'children_ids']:
                setattr(self, '_' + key, val)

        if parent is not None and not isinstance(parent, PerfGraphNode):
            raise TypeError('parent is not of type "PerfGraphNode"')
        self._parent = parent

        # Recursively add all of the children
        self._children = []
        for child_id in node_data.get('children_ids', []):
            self._children.append(PerfGraphNode(child_id, self, data))

    @staticmethod
    def _validateNodeData(node_data):
        """
        Internal method that validates all of the data within a single
        entry that represents a node
        """

        valid = {'children_ids': {'type': list, 'subtype': int},
                 'id': {'required': True, 'type': int},
                 'level': {'required': True, 'type': int},
                 'memory': {'required': True, 'type': int},
                 'name': {'required': True, 'type': str},
                 'num_calls': {'required': True, 'type': int},
                 'parent_id': {'type': int},
                 'time': {'required': True, 'type': float}}

        # Check each entry
        for key, info in valid.items():
            if info.get('required', False):
                if key not in node_data: # Required key is missing
                    raise Exception('Entry missing key "{}":\n{}'.format(key, node_data))
            elif key not in node_data: # Key is optional and doesn't exist
                continue

            val = node_data.get(key)

            # Verify the primary type
            if not isinstance(val, info.get('type')):
                raise Exception('Key "{}" in node entry is not the required type "{}"\n{}'.format(key, info.get('type'), node_data))

            # Verify the sub type (for example, list of ints)
            if 'subtype' in info:
                for subval in val:
                    if not isinstance(subval, info.get('subtype')):
                        raise Exception('Key "{}" in node entry is not the required type "{}" of "{}"\n{}'.format(key, info.get('type'), info.get('subtype'), node_data))

        # Check for extraneous entries
        for key in node_data:
            if key not in valid:
                raise Exception('Key "{}" in node entry is invalid\n{}'.format(key, node_data))

    def children(self):
        """
        Returns the nodes that are immediate children to this node
        """
        return self._children

    def id(self):
        """
        Returns the unique ID of this node
        """
        return self._id

    def level(self):
        """
        Returns the level assigned to the section that created this node
        """
        return self._level

    def name(self):
        """
        Returns the name assigned to the section that created this nod
        """
        return self._name

    def numCalls(self):
        """
        Returns the number of times this node was called
        """
        return self._num_calls

    def parent(self):
        """
        Returns the node that is an immediate parent to this node (None if root)
        """
        return self._parent

    def selfTime(self):
        """
        Returns the time this node took
        """
        return self._time

    def totalTime(self):
        """
        Returns the time this node plus its children took
        """
        return self.selfTime() + self.childrenTime()

    def childrenTime(self):
        """
        Returns the time this node's children took
        """
        return sum([child.totalTime() for child in self.children()])

    def percentTime(self, alt_root_node=None):
        """
        Returns the percentage of time this node took relative to the
        total time of the root node, which defaults to the top node
        in the graph.

        Inputs:
            alt_root_node[PerfGraphNode]: Alternate root node; if not given, use the top root node
        """
        return self.totalTime() * 100 / (self.rootNode() if not alt_root_node else alt_root_node).totalTime()

    def selfMemory(self):
        """
        Returns the amount of memory (in MB) added by this node
        """
        return self._memory

    def totalMemory(self):
        """
        Returns the amount of memory (in MB) added by this node and its children
        """
        return self.selfMemory() + self.childrenMemory()

    def childrenMemory(self):
        """
        Returns the amount of memory (in MB) added by this node's children
        """
        return sum([child.totalMemory() for child in self.children()])

    def percentMemory(self, alt_root_node=None):
        """
        Returns the percentage of memory this node added relative to the
        total memory added by the root node, which defaults to the top
        node in the graph.

        Inputs:
            alt_root_node[PerfGraphNode]: Alternate root node; if not given, use the top root node
        """
        return self.totalMemory() * 100 / (self.rootNode() if not alt_root_node else alt_root_node).totalMemory()

    def rootNode(self):
        """
        Returns the root (top node in the graph)
        """
        parent = self
        while parent.parent() is not None:
            parent = parent.parent()
        return parent

class PerfGraphReporterReader:
    """
    A Reader for MOOSE PerfGraphReporterReader data.

    Inputs:
        file[str]: JSON file containing PerfGraphReporter data
        raw[json]: Raw JSON containing the 'graph' entry from a PerfGraphReporter/perf_graph value
        part[int]: Part of the JSON file to obtain when using "file"

    Must provide either "file" or "raw".

    The final timestep is used to capture the PerfGraph data.
    """
    def __init__(self, file=None, raw=None, part=None):
        if not file and not raw:
            raise Exception('Must provide either "file" or "raw"')
        if file and raw:
            raise Exception('Cannot provide both "file" and "raw"')
        if not file and part is not None:
            raise Exception('"part" is not used with "raw"')

        self._reader = None
        if file:
            self._reader = ReporterReader(file, part)

            # Only read from the final timestep
            final_time = self._reader.times()[-1]
            self._reader.update(final_time)

            # Find the Reporter variable that contains the PerfGraph graph
            perf_graph_var = None
            for var in self._reader.variables():
                if self._reader.info(var[0])['type'] == 'PerfGraphReporter' and var[1] == 'nodes':
                    if perf_graph_var:
                        raise Exception('Multiple PerfGraphReporter values were found')
                    perf_graph_var = var

            graph_data = self._reader[perf_graph_var]
        else:
            graph_data = raw

        # Build the graph; the PerfGraphNode constructor will recursively add children
        self._root_node = None
        for node_data in graph_data:
            if 'parent_id' not in node_data:
                if self._root_node:
                    raise Exception('Multiple root nodes found')
                self._root_node = PerfGraphNode(node_data['id'], None, graph_data)

        # Dict of node ID -> node for convenience, built on initial request
        self._nodes = None
        # Dict of section name -> nodes in that section for convenience, built on initial request
        self._sections = None

    def recursivelyDo(self, do, alt_root_node=None, *args, **kwargs):
        """
        Recursively do an action through the graph.
        If alt_root_node is not provided, act through the entire graph,
        starting with the graph root node.

        Inputs:
            do[function]: Action to perform on each node (input: a PerfGraphNode)
            alt_root_node[PerfGraphNode]: Alternate node to start with
        """
        def recurse(node, do, *args, **kwargs):
            do(node, *args, **kwargs)
            for child in node.children():
                recurse(child, do, *args, **kwargs)

        root_node = self._root_node if alt_root_node is None else alt_root_node
        recurse(root_node, do, *args, **kwargs)

    def nodes(self):
        """
        Returns a dict of all of the nodes in the graph,
        in which the key is the ID.
        """
        if self._nodes is None:
            self._nodes = {}
            def add_to_nodes(node):
                self._nodes[node.id()] = node
            self.recursivelyDo(add_to_nodes)
        return self._nodes

    def node(self, id):
        """
        Returns the PerfGraphNode with the given ID
        """
        return self.nodes().get(id)

    def rootNode(self):
        """
        Returns the root PerfGraphNode
        """
        return self._root_node

    def sections(self):
        """
        Returns a dict of all of the sections. The key
        is the section name and the entries are a list of
        PerfGraphNode that are a part of said section.
        """
        if self._sections is None:
            self._sections = {}
            def add_to_sections(node):
                if node.name() not in self._sections:
                    self._sections[node.name()] = []
                self._sections[node.name()].append(node)
            self.recursivelyDo(add_to_sections)
        return self._sections

    def section(self, name):
        """
        Returns all of the PerfGraphNode objects that
        are contained within a given section.

        Inputs:
            name[str]: The name of the section
        """
        return self.sections().get(name)
