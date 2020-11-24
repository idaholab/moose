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
import moosetree
import mooseutils

LOG = logging.getLogger(__name__)

@mooseutils.addProperty('hidden', default=False, ptype=bool)
@mooseutils.addProperty('removed', default=False, ptype=bool)
@mooseutils.addProperty('test', default=False, ptype=bool)
@mooseutils.addProperty('alias', ptype=str)
@mooseutils.addProperty('group', ptype=str)
@mooseutils.addProperty('markdown', ptype=str)
@mooseutils.addProperty('color', ptype=str, default='RED')
class NodeBase(moosetree.Node, mooseutils.AutoPropertyMixinBase):
    """
    Node for MOOSE syntax that serves as the parent for actions/objects.
    """
    def __init__(self, *args, **kwargs):
        mooseutils.AutoPropertyMixinBase.__init__(self, **kwargs)
        moosetree.Node.__init__(self, *args, **{k:v for k, v in kwargs.items() if not hasattr(self, k)})

    def fullpath(self):
        """
        Return the node full path.
        """
        return '/'.join([n.name for n in self.path])

    def groups(self):
        """
        Return groups associated with this node
        """
        out = set([self.group]) if self.group is not None else set()
        return out

    def __repr__(self):
        """
        Print the node information.
        """
        if self.is_root:
            return 'ROOT'

        msg = ''
        color = self.color
        if self.removed:
            color = 'GREY'
        elif self.test:
            color = 'BLUE'
        elif not self.hidden:
            color = 'LIGHT_' + self.color

        msg0 = '{}: {}'.format(self.name, self.fullpath())
        msg1 = 'hidden={} removed={} group={} groups={} test={} alias={}'.format(self.hidden,
                                                                                 self.removed,
                                                                                 self.group,
                                                                                 self.groups(),
                                                                                 self.test,
                                                                                 self.alias)
        msg0 = mooseutils.colorText(msg0, color)
        msg1 = mooseutils.colorText(msg1, 'GREY' if self.removed else 'LIGHT_GREY')
        return '{} {}'.format(msg0, msg1)

class SyntaxNode(NodeBase):
    """
    Defines a class for syntax only (i.e., a node not attached to a C++ class).

    This class extends the 'parameters' property to automatically collect parameters from
    all the child actions objects.
    """
    def __init__(self, *args, **kwargs):
        kwargs.setdefault('group', kwargs.pop('label', None))
        NodeBase.__init__(self, *args, **kwargs)
        self.color = 'GREEN'
        if self.markdown is None:
            self.markdown = os.path.join(self.fullpath().lstrip('/'), 'index.md')

    def groups(self, syntax=True, actions=True, objects=False):
        """
        Return groups associated with this Actions (i.e., where the syntax is defined).
        """
        out = set([self.group]) if self.group is not None else set()
        if syntax:
            for node in self.syntax():
                out.update(node.groups())
        if actions:
            for node in self.actions():
                out.update(node.groups())
        if objects:
            for node in self.objects():
                out.update(node.groups())
        return out

    def parameters(self):
        """
        Return the action parameters for the syntax.
        """
        parameters = dict()
        for action in self.actions():
            if action.parameters is not None:
                parameters.update(action.parameters)
        return parameters

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
            filter_ = lambda node: (syntax in node.fullpath()) and \
                                   isinstance(node, node_type) and \
                                   (group is None or group in node.groups())
            return moosetree.findall(self, filter_)

        else:
            return [node for node in self.children if (syntax in node.fullpath()) and \
                                                      isinstance(node, node_type) and \
                                                      (group is None or group in node.groups())]

@mooseutils.addProperty('classname', ptype=str)
@mooseutils.addProperty('description', ptype=str)
@mooseutils.addProperty('source', ptype=str)
@mooseutils.addProperty('header', ptype=str)
@mooseutils.addProperty('parameters', ptype=dict)
class ObjectNodeBase(NodeBase):
    """
    Base class for nodes associated with C++ objects (Action, MooseObjectAction, or MooseObject).
    """

    def __init__(self, parent, name, **kwargs):
        kwargs.setdefault('classname', kwargs.pop('class', name))
        kwargs.setdefault('group', kwargs.pop('label', None))
        kwargs.setdefault('source', kwargs.pop('register_file', None))
        NodeBase.__init__(self, parent, name, **kwargs)

        if self.source == '':
            LOG.critical("MooseDocs requires the %s object to use the registerMooseObject or " \
                         "registerMooseAction macro within the source (.C) file, this object " \
                         "is being removed from the available syntax.", self.name)
            self.source = None
            self.removed = True

        if (self.source is not None) and (not os.path.isfile(self.source)):
            LOG.critical("The supplied 'source' file does not exist: %s\n This object is being " \
                         "removed from the available syntax.", self.source)
            self.source = None
            self.removed = True

        if (self.source is not None) and (self.header is None):
            self.header = self.source.replace('/src/', '/include/')[:-1] + 'h'

        if (self.header is not None) and (self.markdown is None):
            idx = self.header.find('/include/') + len('/include/')
            self.markdown = self.header[idx:-1] + 'md'

class MooseObjectNode(ObjectNodeBase):
    """
    Node for a registered C++ MooseObject class.
    """
    def __init__(self, *args, **kwargs):
        ObjectNodeBase.__init__(self, *args, **kwargs)
        self.color = 'YELLOW'

@mooseutils.addProperty('tasks', ptype=set)
class ActionNode(ObjectNodeBase):
    """
    Node for a registered C++ Action class.
    """
    def __init__(self, *args, **kwargs):
        ObjectNodeBase.__init__(self, *args, **kwargs)
        self.color = 'MAGENTA'

class MooseObjectActionNode(ActionNode):
    """
    Node for a registered C++ MooseObjectAction class.
    """
    def __init__(self, *args, **kwargs):
        ActionNode.__init__(self, *args, **kwargs)
        self.color = 'CYAN'
