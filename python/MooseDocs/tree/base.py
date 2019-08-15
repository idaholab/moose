#pylint: disable=missing-docstring
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html
#pylint: enable=missing-docstring
import logging
import anytree
import mooseutils

LOG = logging.getLogger(__name__)

class NodeBase(anytree.NodeMixin):
    """
    Base class for tree nodes that accepts arbitrary attributes.

    The class behaves as a dict(), with respect to accessing the attributes.

    Inputs:
        parent[NodeBase]: The parent Node (use None for the root)
        name[str]: The name of the node.
        **kwargs: Arbitrary key, value pairs that are added to the __dict__ by the anytree.Node
                  class. These are accessible

    IMPORTANT: In a previous version of MOOSEDocs there was a property system that did automatic
               setters and getters with type checking and slew of other clever things. Well the
               cleverness was too slow. The constructor of these objects is called a lot, so the
               checking was hurting performance.

         NOTE: The anytree.NodeMixin was used rather than anytree.Node, which also supports
               arbitrary attributes, because it constructed a bit faster and accessing the
               attributes in anytree.Node is done via "." operator, which is less obvious than using
               dictionary items.
    """

    #The color to print (see mooseutils.colorText).
    COLOR = 'RESET'

    def __init__(self, name, parent, **kwargs):
        super(NodeBase, self).__init__()
        self.__attributes = kwargs
        self.name = name
        self.parent = parent

    @property
    def attributes(self):
        """Return the attributes for this Node."""
        return self.__attributes

    def console(self):
        """
        String returned from this function is for screen output. This allows coloring to be
        handled automatically.
        """
        return '{}: Attributes: {}'.format(self.name, repr(self.__attributes))

    def remove(self):
        """
        Remove the node and children from the tree.
        """
        self.parent = None
        for child in self.children:
            child.parent = None

    def insert(self, idx, child):
        """Insert a child node."""
        children = list(self.children)
        self.children = []
        children.insert(idx, child)
        for child in children:
            child.parent = self

    def iteritems(self):
        """Return dict key, value iterators."""
        return self.__attributes.items()

    def __repr__(self):
        """
        Prints the name of the token, this works in union with __str__ to print
        the tree structure of any given node.
        """
        return mooseutils.colorText(self.console(), self.COLOR)

    def __str__(self):
        """
        Print the complete tree beginning at this node.
        """
        return '\n' + str(anytree.RenderTree(self))

    def __call__(self, index):
        """
        Return a child given the numeric index.

        Inputs:
            index[int]: The numeric index of the child object to return, this is the same
                        as doing self.children[index].
        """
        if len(self.children) <= index:
            LOG.error('A child node with index %d does not exist, there are %d children.',
                      index, len(self.children))
            return None
        return self.children[index]

    def __iter__(self):
        """
        Allows for iterator access over the child nodes.
        """
        for child in self.children:
            yield child

    def __getitem__(self, key):
        """
        Return an attribute.
        """
        return self.__attributes[key]

    def __setitem__(self, key, value):
        """
        Create/set an attribute.
        """
        self.__attributes[key] = value

    def __contains__(self, key):
        """
        Allow for "in" operator to check for attributes.
        """
        return key in self.__attributes

    def __len__(self):
        """Return the number of children."""
        return len(self.children)

    def __bool__(self):
        """
        When __len__ is defined it is used for bool operations.

        If this class exists then it should evaluate to True. This is actually a requirement for
        the node to continue to work with anytree itself. Therefore, this method is defined
        to make sure that it always returns True when queried as a boolean.
        """
        return True

    def get(self, key, default=None):
        """
        Get an attribute with a possible default.
        """
        value = self.__attributes.get(key, default)
        if value is None:
            value = default
        return value

    def set(self, key, value):
        """
        Set the value of an attribute.
        """
        self.__attributes[key] = value

    def write(self):
        """
        Method for outputting content of node to a string.
        """
        out = ''
        for child in self.children:
            out += child.write()
        return out

    @property
    def previous(self):
        """Return the previous sibling, if it exists."""
        if self.parent is not None:
            idx = self.parent.children.index(self)
            if idx > 0:
                return self.parent.children[idx-1]

    @property
    def next(self):
        """Return the next sibling, if it exists."""
        if self.parent is not None:
            idx = self.parent.children.index(self)
            if idx < len(self.parent.children) - 1:
                return self.parent.children[idx+1]
