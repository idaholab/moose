from copy import copy, deepcopy
from collections import deque

try:
    from collections import OrderedDict
except:
    from ordereddict import OrderedDict


class DAGValidationError(Exception):
    pass

# Added by the MOOSE group
class DAGEdgeDepError(Exception):
    pass

# Added by the MOOSE group
class DAGEdgeIndError(Exception):
    pass

class DAG(object):
    """ Directed acyclic graph implementation. """

    def __init__(self):
        """ Construct a new DAG with no nodes or edges. """
        self.__cached_graph = None
        self.reset_graph()

    # Added by the MOOSE group
    def __cacheGraph(self):
        """ Private method to cache the current state of the graph """
        if self.__cached_graph == None:
            self.__cached_graph = self.clone()
        return self.__cached_graph

    # Added by the MOOSE group
    def serialize_dag(self, graph=None):
        """ Serialize the DAG """
        if not graph:
            graph = self.graph
        queue = deque()
        for u in self.topological_sort():
            queue.append(u)
        while queue:
            u = queue.pop()
            if queue:
                v = queue.pop()
                self.add_edge(v, u)
                queue.append(v)
        return graph

    def add_node(self, node_name, graph=None):
        """ Add a node if it does not exist yet, or error out. """
        if not graph:
            graph = self.graph
        if node_name in graph:
            raise KeyError('node %s already exists' % node_name)
        graph[node_name] = set()

        # Invalidate cached graph because a node was added
        self.__cached_graph = None

    def node_exists(self, node_name, graph=None):
        """ Check if node exists """
        if not graph:
            graph = self.graph
        if node_name in graph:
            return True

    def add_node_if_not_exists(self, node_name, graph=None):
        try:
            self.add_node(node_name, graph=graph)
        except KeyError:
            pass

    # Modified by the MOOSE group
    def delete_node(self, node_name, graph=None):
        """ Deletes this node and all edges referencing it. """
        if not graph:
            graph = self.graph
        if node_name not in graph:
            raise KeyError('node %s does not exist' % node_name)

        graph.pop(node_name)

        for node, edges in graph.items():
            if node_name in edges:
                edges.remove(node_name)

    def delete_node_if_exists(self, node_name, graph=None):
        try:
            self.delete_node(node_name, graph=graph)
        except KeyError:
            pass

    # Modified by the MOOSE group
    def add_edge(self, ind_node, dep_node, graph=None):
        """ Add an edge (dependency) between the specified nodes. """
        if not graph:
            graph = self.graph

        if dep_node not in graph:
            raise DAGEdgeDepError()
        if ind_node not in graph:
            raise DAGEdgeIndError()
        graph[ind_node].add(dep_node)
        is_valid, message = self.validate(graph)
        if not is_valid:
            self.delete_edge(ind_node, dep_node)
            raise DAGValidationError()

    # Modified by the MOOSE group
    def delete_edge(self, ind_node, dep_node, graph=None):
        """ Delete an edge from the graph. """
        if not graph:
            graph = self.graph

        if dep_node not in graph.get(ind_node, []):
            raise KeyError('this edge does not exist in graph')
        graph[ind_node].remove(dep_node)

    def rename_edges(self, old_task_name, new_task_name, graph=None):
        """ Change references to a task in existing edges. """
        if not graph:
            graph = self.graph

        for node, edges in graph.items():

            if node == old_task_name:
                graph[new_task_name] = copy(edges)
                del graph[old_task_name]

            else:
                if old_task_name in edges:
                    edges.remove(old_task_name)
                    edges.add(new_task_name)

    def predecessors(self, node, graph=None):
        """ Returns a list of all predecessors of the given node """
        if graph is None:
            graph = self.graph
        return [key for key in graph if node in graph[key]]

    def downstream(self, node, graph=None):
        """ Returns a list of all nodes this node has edges towards. """
        if graph is None:
            graph = self.graph
        if node not in graph:
            raise KeyError('node %s is not in graph' % node)
        return list(graph[node])

    def all_downstreams(self, node, graph=None):
        """
        Returns a list of all nodes ultimately downstream
        of the given node in the dependency graph, in
        topological order."""
        if graph is None:
            graph = self.graph
        nodes = [node]
        nodes_seen = set()
        i = 0
        while i < len(nodes):
            downstreams = self.downstream(nodes[i], graph)
            for downstream_node in downstreams:
                if downstream_node not in nodes_seen:
                    nodes_seen.add(downstream_node)
                    nodes.append(downstream_node)
            i += 1
        return filter(lambda node: node in nodes_seen, self.topological_sort(graph=graph))

    def delete_downstreams(self, node, graph=None):
        """ Delete and return all nodes this node has edges towards. """
        if graph is None:
            graph = self.graph
        deleted_nodes = set([])
        if self.node_exists(node):
            for edge in self.all_downstreams(node):
                deleted_nodes.add(edge)
                self.delete_node_if_exists(edge)
        return deleted_nodes

    def all_leaves(self, graph=None):
        """ Return a list of all leaves (nodes with no downstreams) """
        if graph is None:
            graph = self.graph
        return [key for key in graph if not graph[key]]

    def from_dict(self, graph_dict):
        """ Reset the graph and build it from the passed dictionary.
        The dictionary takes the form of {node_name: [directed edges]}
        """

        self.reset_graph()
        for new_node in graph_dict.keys():
            self.add_node(new_node)
        for ind_node, dep_nodes in graph_dict.items():
            if not isinstance(dep_nodes, list):
                raise TypeError('dict values must be lists')
            for dep_node in dep_nodes:
                self.add_edge(ind_node, dep_node)

    def reset_graph(self):
        """ Restore the graph to an empty state. """
        self.graph = OrderedDict()

    # Modified by the MOOSE group
    def ind_nodes(self, graph=None):
        """
        Returns a list of all nodes in the graph with no dependencies.
        Creates a clone of the current state of the DAG.
        """
        if graph is None:
            graph = self.graph

        # cache current graph because someone is asking for concurrent nodes
        # and the graph is _probably_ the most complete at this stage
        self.__cacheGraph()
        dependent_nodes = set(node for dependents in graph.values() for node in dependents)
        return [node for node in graph.keys() if node not in dependent_nodes]

    def validate(self, graph=None):
        """ Returns (Boolean, message) of whether DAG is valid. """
        graph = graph if graph is not None else self.graph
        if len(self.ind_nodes(graph)) == 0:
            return (False, 'no independent nodes detected')
        try:
            self.topological_sort(graph)
        except ValueError:
            return (False, 'failed topological sort')
        return (True, 'valid')

    def topological_sort(self, graph=None):
        """ Returns a topological ordering of the DAG.
        Raises an error if this is not possible (graph is not valid).
        """
        if graph is None:
            graph = self.graph

        in_degree = {}
        for u in graph:
            in_degree[u] = 0

        for u in graph:
            for v in graph[u]:
                in_degree[v] += 1

        queue = deque()
        for u in in_degree:
            if in_degree[u] == 0:
                queue.appendleft(u)

        l = []
        while queue:
            u = queue.pop()
            l.append(u)
            for v in graph[u]:
                in_degree[v] -= 1
                if in_degree[v] == 0:
                    queue.appendleft(v)

        if len(l) == len(graph):
            return l
        else:
            raise ValueError('graph is not acyclic')

    def size(self):
        return len(self.graph)

    # Added by the MOOSE group
    def getOriginalDAG(self):
        """
        Returns a clone of the graph as it existed before calling the first ind_nodes
        """
        return self.__cacheGraph().graph

    # Added by the MOOSE group
    def clone(self):
        """ create a hard copy of the current graph with pointers
        to the original graph's contents.
        """
        new_graph = DAG()
        new_graph.graph = copy(self.graph)
        return new_graph

    # Added by the MOOSE group
    def reverse_clone(self, graph=None):
        """
        Return a new graph with reversed dependencies based
        on current graph.
        """
        if graph is None:
            graph = self.graph

        new_graph = DAG()
        for key in self.topological_sort(graph):
            new_graph.add_node(key)

        for key, value in graph.items():
            for downstream_node in value:
                new_graph.add_edge(downstream_node, key)
        return new_graph

    # Added by the MOOSE group
    def reverse_edges(self, graph=None):
        """ Reversed dependencies in current graph. """

        new_graph = self.reverse_clone()
        self.graph = new_graph.graph

    # Modified by the MOOSE group
    def delete_edge_if_exists(self, ind_node, dep_node, graph=None):
        """ Delete an edge from the graph. """
        if not graph:
            graph = self.graph

        if dep_node not in graph.get(ind_node, []):
            return
        graph[ind_node].remove(dep_node)

    # Added by the MOOSE group
    def is_dependency(self, ind_node, dep_node, graph=None):
        """ Returns whether or not dep_node depends on ind_node """
        if not graph:
            graph = self.graph

        deps = graph[ind_node]
        if dep_node in deps:
            return True
        for node in deps:
            if self.is_dependency(node, dep_node, graph=graph):
                return True
        return False
