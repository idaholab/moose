"""
For simplicity this module should be a stand-alone package, i.e., it should not use any
non-standard python packages such as mooseutils.
"""
import copy
from . import search

class Node(object):
    """
    Base class for tree nodes that accepts arbitrary attributes.

    Create a new node in the tree that is a child of *parent* with the given *name*. The supplied
    *parent* must be another `Node` object. All keyword arguments are stored as "attributes" and may
    be of any type.

    !alert warning title=Speed is Important!
    The need for this object comes from the MooseDocs package, which uses tree objects extensively.
    Originally, MooseDocs used the anytree package for these structures. As the MooseDocs system
    evolved as well as the amount of documentation, in particular the amount of generated HTML
    output, the speed in creating the tree nodes became critical. The anytree package is robust and
    well designed, but the construction of the nodes was not fast enough.
    """

    def __init__(self, parent, name, **kwargs):
        """
        This constructor must be as minimal as possible for speed purposes.

        IMPORTANT: Do not add more items to this unless you have good reason, it will impact
                   MooseDocs performance greatly.
        """
        self.__children = list()
        self.__parent = parent
        self.__name = name
        self.__attributes = kwargs

        if self.__parent is not None:
            parent.__children.append(self)

    @property
    def name(self):
        """Return the name of the Node."""
        return self.__name

    @property
    def parent(self):
        """Return the parent Node object, which is None for a root node."""
        return self.__parent

    @parent.setter
    def parent(self, new_parent):
        """Set the parent Node object to *new_parent*, use None to remove the node from the tree."""
        if (self.__parent is not None) and (self in self.__parent.__children):
            self.__parent.__children.remove(self)

        self.__parent = new_parent
        if self.__parent is not None:
            self.__parent.__children.append(self)

    @property
    def children(self):
        """Return a list of children.

        !alert note
        The list is a copy but the Node objects in the list are not.
        """
        return copy.copy(self.__children)

    @property
    def descendants(self):
        """Return a list of all descendants, children's children etc."""
        return search.iterate(self, method=search.IterMethod.PRE_ORDER)

    @property
    def count(self):
        """Return the number of all descendants"""
        count = len(self.__children)
        for child in self.__children:
            count += child.count
        return count

    def __iter__(self):
        """Iterate of the children (e.g., `for child in node:`)"""
        return iter(self.__children)

    def insert(self, idx, child):
        """Insert a nod *child* before the supplied *idx* in the list of children."""
        self.__children.insert(idx, child)
        child.__parent = self

    @property
    def path(self):
        """Return the nodes that lead to the root node of the tree from this node."""
        nodes = [self]
        parent = self.__parent
        while parent is not None:
            nodes.insert(0, parent)
            parent = parent.parent
        return nodes

    @property
    def root(self):
        """Return the root node of the tree."""
        return self.path[0]

    @property
    def is_root(self):
        """Return True if the Node is a root, i.e., is the parent node object set to None."""
        return self.__parent is None

    @property
    def siblings(self):
        """Return a list of sibling nodes."""
        if self.__parent is not None:
            children = self.__parent.children
            children.remove(self)
            return children
        return []

    @property
    def previous(self):
        """Return the previous sibling, if it exists."""
        if (self.__parent is not None) and (self.__parent.__children):
            idx = self.__parent.__children.index(self)
            if idx > 0:
                return self.__parent.__children[idx-1]

    @property
    def next(self):
        """Return the next sibling, if it exists."""
        if (self.__parent is not None) and (self.__parent.__children):
            idx = self.__parent.__children.index(self)
            if idx < len(self.__parent.__children) - 1:
                return self.__parent.__children[idx+1]

    def __call__(self, *args):
        """Return child nodes based on index."""
        child = self
        for index in args:
            child = child.__children[index]
        return child

    @property
    def attributes(self):
        """Return the a 'attributes' (key, value pairs supplied in construction) for this node."""
        return self.__attributes

    def __getitem__(self, key):
        """Retrieve an attribute using operator[]."""
        return self.__attributes[key]

    def __setitem__(self, key, value):
        """Set an attribute using operator[]."""
        self.__attributes[key] = value

    def __contains__(self, key):
        """Test if an attribute exists using the 'in' keyword."""
        return key in self.__attributes

    def get(self, key, default=None):
        """Return the value of an attribute *key* or *default* if it does not exist."""
        return self.__attributes.get(key, default)

    def items(self):
        """Return the dict() iterator to the attributes, i.e., `k, v in node.items()`."""
        return self.__attributes.items()

    def __len__(self):
        """Return the number of children."""
        return len(self.__children)

    def __bool__(self):
        """If this class exists then it should evaluate to True."""
        return True

    def __str__(self):
        """Return a unicode string showing the tree structure."""
        return self.__print()

    def __repr__(self):
        """Return the 'name' of the object as it should be printed in the tree."""
        if self.__attributes:
            return '{}: {}'.format(self.name, repr(self.__attributes))
        return self.name

    def __print(self, indent=u''):
        """Helper function printing to the screen."""
        if (self.parent is None) or (self.parent.children[-1] is self):
            out = u'{}\u2514\u2500 {}\n'.format(indent, repr(self))
            indent += u"   "

        else:
            out = u'{}\u251c\u2500 {}\n'.format(indent, repr(self))
            indent += u"\u2502  "

        for child in self.children:
            out += child.__print(indent)

        return out
