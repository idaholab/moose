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
import inspect
import logging

import anytree

import mooseutils

from MooseDocs.common import exceptions

LOG = logging.getLogger(__name__)

class Property(object):
    """
    A descriptor object for creating properties for the NodeBase class defined below.

    A system using this object and the NodeBase class was created to allow for dynamic property
    creation on nodes that allows defaults, types, and a required status to be defined for the
    properties.

    When developing the tokens it was desirable to create properties (via @property) etc. to access
    token data, but it became a bit tedious so an automatic method was created, see the
    documentation on the NodeBase class for information on using the automatic system.

    This property class can also be inherited from to allow for arbitrary checks to be performed,
    for example that a number is positive or a list is the correct length.
    """
    def __init__(self, name, default=None, ptype=None, required=False):
        self.name = name
        self.__type = ptype
        self.__required = required
        self.__default = default

        if (ptype is not None) and (not isinstance(ptype, type)):
            msg = "The supplied property type (ptype) must be of type 'type', but '{}' provided."
            raise exceptions.MooseDocsException(msg, type(ptype).__name__)

        if (ptype is not None) and (default is not None) and (not isinstance(default, ptype)):
            msg = "The default for property must be of type '{}', but '{}' was provided."
            raise exceptions.MooseDocsException(msg, ptype.__name__, type(default).__name__)

    @property
    def default(self):
        """Return the default for this property."""
        return self.__default

    @property
    def type(self):
        """The required property type."""
        return self.__type

    @property
    def required(self):
        """Return the required status for the property."""
        return self.__required

    def __set__(self, instance, value):
        """Set the property value."""
        if (self.__type is not None) and (not isinstance(value, self.__type)):
            msg = "The supplied property '{}' must be of type '{}', but '{}' was provided."
            raise exceptions.MooseDocsException(msg, self.name, self.type.__name__,
                                                type(value).__name__)
        instance._NodeBase__properties[self.name] = value #pylint: disable=protected-access

    def __get__(self, instance, key):
        """Get the property value."""
        return instance._NodeBase__properties.get(self.name, self.default) #pylint: disable=protected-access

class NodeBase(anytree.NodeMixin):
    """
    Base class for tree nodes that accepts defined properties and arbitrary attributes.

    Properties, in the python sense, may be created using the class PROPERTIES variable.
    For example,

        class ExampleNode(NodeBase):
            PROPERTIES = Property('foo', required=True)

        node = ExampleNode(foo=42)
        node.foo = 43

    The PROPERTIES from all parent classes are automatically retrieved.

    Additionally, arbitrary attributes can be stored on creation or by using the dict() style
    set/get methods. By convention any leading or trailing underscores used in defining the
    attribute in the constructor are removed for storage.

        node = ExampleNode(foo=42, class_='fancy')
        node['class'] = 'not fancy'


    Inputs:
        parent[NodeBase]: (Optional) Set the parent node of the node being created, if not
                          supplied the resulting node will be the root node.
        kwargs: (Optional) Any key, value pairs supplied are stored as properties or attributes.
    """
    COLOR = 'RESET'
    PROPERTIES = []

    def __init__(self, parent=None, name=None, **kwargs):
        anytree.NodeMixin.__init__(self)

        # Create a set of unique properties from all classes in the inheritance chain
        properties = set()
        for cls in inspect.getmro(type(self)):
            properties.update(getattr(cls, 'PROPERTIES', []))

        # anytree.NodeMixin properties
        self.parent = parent
        self.name = name if name is not None else self.__class__.__name__

        # NodeBase content
        self.__properties = dict() # storage for property values
        self.__attributes = dict() # storage for attributes (i.e., unknown key, values)

        # Check PROPERTIES type
        if not isinstance(self.PROPERTIES, list):
            raise exceptions.MooseDocsException("The class attribute 'PROPERTIES' must be a list.")

        # Apply the default values
        for prop in properties:
            if not isinstance(prop, Property):
                msg = "The supplied property must be a Property object, but {} provided."
                raise exceptions.MooseDocsException(msg, type(prop).__name__)

            setattr(self.__class__, prop.name, prop)
            self.__properties[prop.name] = prop.default

        # Update the properties from the key value pairs
        for key, value in kwargs.iteritems():
            if value is None:
                continue
            if key in self.__properties:
                setattr(self, key, value)
            else:
                self.__attributes[key.strip('_')] = value

        # Check required
        for prop in properties:
            if prop.required and self.__properties[prop.name] is None:
                raise exceptions.MooseDocsException("The property '{}' is required.", prop.name)

    def console(self):
        """
        String returned from this function is for screen output. This allows coloring to be
        handled automatically.
        """
        return '{}: Properties: {}, Attributes: {}'. \
            format(self.name, repr(self.__properties), repr(self.__attributes))

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
        return str(anytree.RenderTree(self))

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

    def __nonzero__(self):
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

    @property
    def attributes(self):
        """
        Return the attributes for the object.
        """
        return self.__attributes

    def write(self):
        """
        Abstract method for outputting content of node to a string.
        """
        out = ''
        for child in self.children:
            out += child.write()
        return out

    def find(self, name, attr=None, maxlevel=None):
        """
        Search for a node, by name.
        """
        if attr is None:
            for node in anytree.PreOrderIter(self, maxlevel=maxlevel):
                if node.name == name:
                    return node
        else:
            for node in anytree.PreOrderIter(self, maxlevel=maxlevel):
                if (attr in node) and (name in node[attr]):
                    return node
