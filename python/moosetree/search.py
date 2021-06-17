"""Tools for iterating and locating nodes."""
import sys
from enum import Enum

class IterMethod(Enum):
    """
    An 'Enum' for defining iteration process.
    https://en.wikipedia.org/wiki/Tree_traversal
    """
    PRE_ORDER = 1
    BREADTH_FIRST = 2

def findall(node, func=None, method=None, **kwargs):
    """
    Find all nodes matching the supplied function.

    The root for the search is provided in *node*. A search is conducted by calling *func* at each
    descendant of the root node. If this function evaluates to `True` the resulting node object is
    added to the list of nodes that is returned.

    The search *method* defaults to a breath first search, but any IterMethod can be supplied.

    If a function is not provided then the default for *func* is used, which checks that the supplied
    keyword arguments match the attributes of the node.
    """
    if (func is None) and (kwargs):
        func = lambda n: any(n.attributes.get(key, None)==value for key, value in kwargs.items())
    return iterate(node, func, False, method)

def find(node, func=None, method=None, **kwargs):
    """
    Operates in the same fashion as "findall"; however, if a match is found the search is terminated
    and the node is returned.
    """
    if (func is None) and (kwargs):
        func = lambda n: any(n.attributes.get(key, None)==value for key, value in kwargs.items())
    nodes = list(iterate(node, func, True, method))
    return nodes[0] if nodes else None

def iterate(node, func=None, abort_on_find=False, method=None):
    """
    Iterates over the descendants of *node*.

    The search *method* defaults to a breath first search, but any IterMethod can be supplied. The
    iteration can be limited to certain nodes by defining *func*. This function is evaluated at
    each node during iteration and only nodes that result in `True` are considered. If
    *abort_on_find* is set to `True` the iteration will stop after the first evaluation of the
    function that is `True.
    """
    if func is None:
        func = lambda n: True

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
