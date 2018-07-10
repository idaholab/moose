#pylint: disable=missing-docstring, no-member
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html
import os
import logging
import copy

import anytree

from .base import NodeBase, Property

LOG = logging.getLogger(__name__)

class SyntaxNodeBase(NodeBase):
    """
    Node for MOOSE syntax that serves as the parent for actions/objects.
    """
    PROPERTIES = [Property('hidden', ptype=bool, default=False),
                  Property('removed', ptype=bool, default=False),
                  Property('parameters', ptype=dict),
                  Property('description', ptype=unicode),
                  Property('alias', ptype=unicode)]

    def __init__(self, *args, **kwargs):
        NodeBase.__init__(self, *args, **kwargs)
        self._groups = set()

    @property
    def groups(self):
        """
        Return groups associated with this node or entire tree (i.e., where the syntax is defined).
        """
        out = copy.copy(self._groups)
        for node in self.descendants:
            out.update(node.groups)
        return out

    @property
    def fullpath(self):
        """
        Return the node full path.
        """
        out = []
        node = self
        while node is not None:
            out.append(node.name)
            node = node.parent
        return '/'.join(reversed(out))

    def markdown(self):
        """Return the 'required' markdown filename."""
        raise NotImplementedError()

    def findfull(self, name, maxlevel=None):
        """
        Search for a node, by full name.
        """
        for node in anytree.PreOrderIter(self, maxlevel=maxlevel):
            if (node.fullpath == name) or (node.alias == name):
                return node

    def syntax(self, *args, **kwargs):
        """
        Return SyntaxNode nodes (see __nodeFinder).
        """
        return self.__nodeFinder(SyntaxNode, *args, **kwargs)

    def objects(self, *args, **kwargs):
        """
        Return MooseObjectNode nodes (see __nodeFinder).
        """
        return self.__nodeFinder(MooseObjectNode, *args, **kwargs)

    def actions(self, *args, **kwargs):
        """
        Return ActionNode nodes (see __nodeFinder).
        """
        return self.__nodeFinder(ActionNode, *args, **kwargs)

    def __nodeFinder(self, node_type, syntax='', group=None, recursive=False):
        """
        A helper method for finding nodes of a given type, syntax, and group.

        Inputs:
            node_type[NodeCore]: The type of node to consider.
            syntax: (optional) The syntax that must be within the object 'fullpath' property.
            group: (optional) The group to limit the search.
            recursive: When True the search will look through all nodes in the entire tree, when
                       False only the children of the node are considered.
        """
        if recursive:
            filter_ = lambda node: (syntax in node.fullpath) and \
                                   isinstance(node, node_type) and \
                                   (group is None or group in node.groups)
            return self.findall(filter_=filter_)

        else:
            return [node for node in self.children if (syntax in node.fullpath) and \
                                                      isinstance(node, node_type) and \
                                                      (group is None or group in node.groups)]

    def console(self):
        """
        Print the node information.
        """
        #pylint: disable=invalid-name
        if self.removed:
            self.COLOR = 'GREY'
        elif self.hidden:
            self.COLOR = self.COLOR.replace('LIGHT_', '')

        msg = '{}: {} hidden={} removed={} groups={} alias={}'.format(self.name,
                                                                      str(self.fullpath),
                                                                      self.hidden,
                                                                      self.removed,
                                                                      self.groups,
                                                                      self.alias)
        return msg

class SyntaxNode(SyntaxNodeBase):
    """
    Defines a class for syntax only (i.e., a node not attached to a C++ class).

    This needs to be a separate class for type checking.
    """
    COLOR = 'LIGHT_GREEN'

    def __init__(self, parent, name, **kwargs):
        SyntaxNodeBase.__init__(self, parent, name, **kwargs)

    def markdown(self):
        """
        Return the expected markdown file name.
        """
        return os.path.join(self.fullpath.lstrip('/'), 'index.md')

    @property
    def parameters(self):
        """
        Return the action parameters for the syntax.
        """
        parameters = dict()
        for action in self.actions():
            if action.parameters is not None:
                parameters.update(action.parameters)
        return parameters

    def console(self):
        """
        Add source to console dump.
        """
        return SyntaxNodeBase.console(self) + ' markdown={}'.format(self.markdown())

class ObjectNode(SyntaxNodeBase): #pylint: disable=abstract-method
    """
    Base class for nodes associated with C++ objects (Action, MooseObjectAction, or MooseObject).
    """

    def __init__(self, parent, name, item, **kwargs):
        SyntaxNodeBase.__init__(self, parent, name, **kwargs)

        if item['description']:
            self.description = item['description']
        if item['parameters']:
            self.parameters = item['parameters']

        self._groups.add(item['label'])
        self._source = item['register_file']
        self._repo = None

        if self._source == '':
            LOG.critical("MooseDocs requires the %s object to use the registerMooseObject or " \
                         "registerMooseAction macro within the source (.C) file, this object " \
                         "is being removed from the available syntax.", self.name)
            self._source = None
            self.removed = True

    def markdown(self):
        """
        The expected markdown file.
        """
        hdr = self.header()
        if hdr is not None:
            idx = hdr.find('/include/') + len('/include/')
            return hdr[idx:-1] + 'md'

    def source(self):
        """
        Return the source file for the object.
        """
        return self._source

    def header(self):
        """
        Return the header file for the object.
        """
        if self._source is not None:
            header = self._source.replace('/src/', '/include/')[:-1] + 'h'
            if not os.path.exists(header):
                LOG.error("%s, no header file found for: %s", self.name, self._source)
                return None
            return header

    def console(self):
        """
        Add source to console dump.
        """
        return SyntaxNodeBase.console(self) + ' markdown={}'.format(self.markdown())

class MooseObjectNode(ObjectNode):
    """
    MooseObject nodes.
    """
    COLOR = 'LIGHT_YELLOW'

    def __init__(self, parent, key, item, **kwargs):
        ObjectNode.__init__(self, parent, key, item, **kwargs)
        self.__class_name = item['class'] if 'class' in item else key

    @property
    def class_name(self):
        """
        Return the name of the C++ class, which can be different than the input file name.
        """
        return self.__class_name

class ActionNode(ObjectNode):
    """
    Action nodes.
    """
    COLOR = 'LIGHT_MAGENTA'

    def __init__(self, parent, key, item, **kwargs):
        ObjectNode.__init__(self, parent, key, item, **kwargs)

        self._tasks = set(item['tasks']) if 'tasks' in item else set()

    @property
    def tasks(self):
        """Return set of associate action tasks."""
        return self._tasks

    @property
    def class_name(self):
        """
        Return the name of the C++ class for the action.
        """
        return self.name

class MooseObjectActionNode(ActionNode):
    """
    MooseObjectAction nodes.
    """
    COLOR = 'LIGHT_CYAN'
