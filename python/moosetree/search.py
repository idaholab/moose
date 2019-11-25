"""Tools for iterating and locating nodes."""
import sys
if sys.version_info[0] == 3:
    from enum import Enum

    class IterMethod(Enum):
        """
        An 'Enum' for defining iteration process.
        https://en.wikipedia.org/wiki/Tree_traversal
        """
        PRE_ORDER = 1
        BREADTH_FIRST = 2
else:
    class IterMethod(object):
        PRE_ORDER = 1
        BREADTH_FIRST = 2

def findall(node, func=None, method=None, **kwargs):
    """
    Find all nodes matching the supplied function.

    Input:
        node[moosetree.Node]: The node from which to begin the search.
        func[]: A function that returns True/False.
        method[Scheme]: The method for searching, see iterator.py
    """
    if (func is None) and (kwargs):
        func = lambda n: any(n.attributes.get(key, None)==value for key, value in kwargs.items())
    return iterate(node, func, False, method)

def find(node, func=None, method=None, **kwargs):
    """
    Find single node matching the supplied function.

    Input:
        node[moosetree.Node]: The node from which to begin the search.
        func[]: A function that returns True/False.
        method[Scheme]: The method for searching, see iterator.py
    """
    if (func is None) and (kwargs):
        func = lambda n: any(n.attributes.get(key, None)==value for key, value in kwargs.items())
    nodes = list(iterate(node, func, True, method))
    return nodes[0] if nodes else None

def iterate(node, func=lambda n: True, abort_on_find=False, method=None):
    """Function for performing tree iteration."""

    if (method is None) or (method == IterMethod.BREADTH_FIRST):
        return __breadthfirst_iterate(node, func, abort_on_find)

    elif method == IterMethod.PRE_ORDER:
        return __preorder_iterate(node, func, abort_on_find)

def __breadthfirst_iterate(node, func, abort_on_find):
    """Breadth-first iteration"""
    stack = node.children
    while stack:
        child = stack.pop(0)
        if func(child):
            yield child
            if abort_on_find:
                return
        stack += child.children

def __preorder_iterate(node, func, abort_on_find):
    """Pre-Order iteration"""
    stack = node.children
    while stack:
        child = stack.pop(0)
        if func(child):
            yield child
            if abort_on_find:
                return
        stack = child.children + stack
