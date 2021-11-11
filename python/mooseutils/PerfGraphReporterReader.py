#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from mooseutils.ReporterReader import ReporterReader

class PerfGraphObject:
    """
    Base class PerfGraphNode and PerfGraphSection.

    This allows the interface for these two objects to be
    similar and reduces duplication.
    """

    def __init__(self, name, level):
        """
        Inputs:

        - name\[str\]: The section name
        - level\[int\] The section level
        """
        self._name = name
        self._level = level

        # The nodes associated with this object. For a PerfGraphNode,
        # this is a single node. For a PerfGraphSection this is one or
        # more nodes in said section.
        self._nodes = []

    def _sumAllNodes(self, do):
        """
        Internal method for summing across all nodes.
        """
        return sum([do(node) for node in self._nodes])

    def info(self):
        """
        Returns the number of calls, the time, and the memory
        in a human readable form.
        """
        info_str = 'Num calls: {}'.format(self.numCalls())
        info_str += '\nLevel: {}'.format(self.level())
        info_str += '\nTime ({:.2f}%): Self {:.2f} s, Children {:.2f} s, Total {:.2f} s'.format(self.percentTime(), self.selfTime(), self.childrenTime(), self.totalTime())
        info_str += '\nMemory ({:.2f}%): Self {} MB, Children {} MB, Total {} MB'.format(self.percentMemory(), self.selfMemory(), self.childrenMemory(), self.totalMemory())
        return info_str

    def name(self):
        """
        Returns the name assigned to the section.
        """
        return self._name

    def level(self):
        """
        Returns the level assigned to the section.
        """
        return self._level

    def numCalls(self):
        """
        Returns the number of times this was called.
        """
        return self._sumAllNodes(lambda node: node._num_calls)

    def selfTime(self):
        """
        Returns the time only this (not including children) took in seconds.
        """
        return self._sumAllNodes(lambda node: node._time)

    def totalTime(self):
        """
        Returns the time this plus its children took in seconds.
        """
        return self.selfTime() + self.childrenTime()

    def childrenTime(self):
        """
        Returns the time the children took in seconds.
        """
        return self._sumAllNodes(lambda node: sum([child.totalTime() for child in node.children()]))

    def percentTime(self):
        """
        Returns the percentage of time this took relative to the
        total time of the root node.
        """
        return self.totalTime() * 100 / self.rootNode().totalTime()

    def selfMemory(self):
        """
        Returns the memory added by only this (not including children) in Megabytes.
        """
        return self._sumAllNodes(lambda node: node._memory)

    def totalMemory(self):
        """
        Returns the memory added by only this plus its children in Megabytes.
        """
        return self.selfMemory() + self.childrenMemory()

    def childrenMemory(self):
        """
        Returns the memory added by children in Megabytes.
        """
        return self._sumAllNodes(lambda node: sum([child.totalMemory() for child in node.children()]))

    def percentMemory(self):
        """
        Returns the percentage of memory this this took relative
        to the total time of the root node.
        """
        return self.totalMemory() * 100 / self.rootNode().totalMemory()

    def rootNode(self):
        """
        Returns the root (top node in the graph).
        """
        parent = self._nodes[0]
        while parent.parent() is not None:
            parent = parent.parent()
        return parent

class PerfGraphNode(PerfGraphObject):
    """
    A node in the graph for the PerfGraphReporterReader.
    These should really only be constructed internally within
    the PerfGraphReporterReader.

    Inputs:

    - name\[str\]: Section name for this node.
    - node_data\[dict\]: JSON output for this node.
    - parent\[PerfGraphNode\]: The parent to this node (None if root).
    """
    def __init__(self, name, node_data, parent):
        # Validate that the data in the node is as we expect
        valid_node_data = {'level': int, 'memory': int, 'num_calls': int, 'time': float}
        for key, type in valid_node_data.items():
            if key not in node_data: # Required key is missing
                raise Exception('Entry missing key "{}":\n{}'.format(key, node_data))
            if not isinstance(node_data.get(key), type):
                raise Exception('Key "{}" in node entry is not the required type "{}"\n{}'.format(key, type, node_data))

        self._memory = node_data['memory']
        self._num_calls = node_data['num_calls']
        self._time = node_data['time']

        super().__init__(name, node_data['level'])
        self._nodes.append(self)

        if parent is not None and not isinstance(parent, PerfGraphNode):
            raise TypeError('parent is not of type "PerfGraphNode"')
        self._parent = parent

        # Recursively add all of the children
        self._children = []
        for key, val in node_data.items():
            if key not in valid_node_data:
                self._children.append(PerfGraphNode(key, val, self))

        # We will fill the section after we build the graph
        self._section = None

    def __str__(self):
        return 'PerfGraphNode "' + '/'.join(self.path()) + '"'

    def __getitem__(self, name):
        return self.child(name)

    def info(self):
        """
        Returns the number of calls, the time, memory,
        and children in a human readable form.
        """
        info_str = 'PerfGraphNode\n'
        info_str += '  Path:\n'
        for i in range(0, len(self.path())):
            info_str += '    ' + ' ' * i + self.path()[i] + '\n'
        info_str += '  ' + super().info().replace('\n', '\n  ')
        if self.children():
            info_str += '\n  Children:'
            for child in self.children():
                info_str += '\n    ' + child.name() + ' ({} call(s), {:.1f}% time, {:.1f}% memory)'.format(child.numCalls(), child.percentTime(), child.percentMemory())
        return info_str

    def path(self):
        """
        Returns the full path in the graph for this node.
        """
        names = [self.name()]
        parent = self
        while parent.parent() is not None:
            names.append(parent.parent().name())
            parent = parent.parent()
        return names[::-1]

    def section(self):
        """
        Returns the PerfGraphSection that this node is in.
        """
        return self._section

    def children(self):
        """
        Returns the nodes that are immediate children to this node.
        """
        return self._children

    def child(self, name):
        """
        Returns the child node with the given name, if one exists, otherwise None.
        """
        if not isinstance(name, str):
            raise TypeError('"name" should be a str')
        for child in self.children():
            if child.name() == name:
                return child
        return None

    def parent(self):
        """
        Returns the node that is an immediate parent to this node (None if root).
        """
        return self._parent

class PerfGraphSection(PerfGraphObject):
    """
    A section in the graph for the PerfGraphReporterReader.
    These should really only be constructed internally within
    the PerfGraphReporterReader.

    Inputs:

    - name\[str\]: Section name for this node
    - node_data\[dict\]: JSON output for this node
    - parent\[PerfGraphNode\]: The parent to this node (None if root)
    """

    def __str__(self):
        return 'PerfGraphSection "' + self.name() + '"'

    def info(self):
        info_str = 'PerfGraphSection "' + self.name() + '":'
        info_str += '\n  ' + super().info().replace('\n', '\n  ')
        info_str += '\n  Nodes:'
        for node in self.nodes():
            for i in range(len(node.path())):
                info_str += '\n    ' + ('- ' if i == 0 else '  ')
                info_str += ' ' * i + node.path()[i]
                if i == len(node.path()) - 1:
                    info_str += ' ({} call(s), {:.1f}% time, {:.1f}% memory)'.format(node.numCalls(), node.percentTime(), node.percentMemory())
        return info_str

    def nodes(self):
        """
        Returns the nodes that are in this section.
        """
        return self._nodes

    def node(self, path):
        """
        Returns the node with the given path, if one exists, otherwise None.

        Inputs:

        - path\[list\]: Path in the graph to the node
        """
        for node in self.nodes():
            if node.path() == path:
                return node

        return None

class PerfGraphReporterReader:
    """
    A Reader for MOOSE PerfGraphReporterReader data.

    Inputs:

    - file\[str\]: JSON file containing PerfGraphReporter data.
    - part\[int\]: Part of the JSON file to obtain when using "file".

    The final timestep is used to capture the PerfGraph data.
    """
    def __init__(self, file, part=0):
        self._reader = None
        self._reader = ReporterReader(file)
        self._reader.update(part=part)

        # Find the Reporter variable that contains the PerfGraph graph
        perf_graph_var = None
        for var in self._reader.variables():
            if self._reader.info(var[0])['type'] == 'PerfGraphReporter' and var[1] == 'graph':
                perf_graph_var = var

        graph_data = self._reader[perf_graph_var]

        if len(graph_data) != 1:
            raise Exception('Single root node not found in data')

        # Build the graph; the PerfGraphNode constructor will recursively add children
        root_node_name = list(graph_data.keys())[0]
        root_node_data = graph_data[root_node_name]
        self._root_node = PerfGraphNode(root_node_name, root_node_data, None)

        # Setup all of the sections
        self._sections = {}
        def add_section(node):
            if node.name() in self._sections:
                section = self._sections.get(node.name())
            else:
                section = PerfGraphSection(node.name(), node.level())
                self._sections[node.name()] = section
            node._section = section
            section._nodes.append(node)
        self.recurse(add_section)

    def __getitem__(self, name):
        return self.rootNode().child(name)

    def recurse(self, act, *args, **kwargs):
        """
        Recursively do an action through the graph starting with the root node.

        Inputs:

        - act\[function\]: Action to perform on each node (input: a PerfGraphNode)
        """
        def _recurse(node, act, *args, **kwargs):
            act(node, *args, **kwargs)
            for child in node.children():
                _recurse(child, act, *args, **kwargs)

        _recurse(self.rootNode(), act, *args, **kwargs)

    def rootNode(self):
        """
        Returns the root PerfGraphNode.
        """
        return self._root_node

    def node(self, path):
        """
        Returns the node with the given path if one exists, otherwise None.

        Inputs:

        - path\[list\]: Path to the node
        """
        if len(path) == 0 or path[0] != self.rootNode().name():
            return None
        node = self.rootNode()
        for name in path[1:]:
            if node:
                node = node[name]
        return node

    def sections(self):
        """
        Returns all of the named sections in a list of PerfGraphSection objects.
        """
        return self._sections.values()

    def section(self, name):
        """
        Returns the PerfGraphSection with the given name if one exists, otherwise None.

        Inputs:

        - name\[str\]: The name of the section.
        """
        if not isinstance(name, str):
            raise TypeError('"name" should be a str')
        return self._sections.get(name, None)

    def heaviestNodes(self, num, memory=False):
        """
        Returns the heaviest nodes in the form of PerfGraphNode objects.

        Inputs:

        - num\[int\]: The number of nodes to return.
        - memory\[boolean\]: Whether or not to sort by memory.
        """
        if not isinstance(num, int):
            raise TypeError('"num" should be an int')
        if num < 1:
            raise ValueError('"num" should be >= 1')
        nodes = []
        add_node = lambda node: nodes.append(node)
        self.recurse(add_node)
        return sorted(nodes, key=lambda node: node.selfMemory() if memory else node.selfTime(), reverse=True)[0:num]

    def heaviestSections(self, num, memory=False):
        """
        Returns the heaviest sections in the form of PerfGraphSection objects.

        Inputs:

        - num\[int\]: The number of sections to return.
        - memory\[boolean\]: Whether or not to sort by memory.
        """
        if not isinstance(num, int):
            raise TypeError('"num" should be an int')
        if num < 1:
            raise ValueError('"num" should be >= 1')
        return sorted(self.sections(), key=lambda section: section.selfMemory() if memory else section.selfTime(), reverse=True)[0:num]
