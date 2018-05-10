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

import MooseDocs
from MooseDocs import common
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

    STUB_HEADER = '<!-- MOOSE Documentation Stub: Remove this when content is added. -->'

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
        if self.alias:
            return self.alias

        out = []
        node = self
        while node is not None:
            out.append(node.name)
            node = node.parent
        return '/'.join(reversed(out))

    @property
    def content(self):
        """Return the markdown content."""
        return common.read(self._filename)

    def markdown(self):
        """Return the 'required' markdown filename."""
        #TODO: Make this a property
        raise NotImplementedError()

    def findfull(self, name, maxlevel=None):
        """
        Search for a node, by full name.
        """
        for node in anytree.PreOrderIter(self, maxlevel=maxlevel):
            if node.fullpath == name:
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

    def check(self, generate=False, group=None, update=None, root=None):
        """
        Check that the expected documentation exists.

        Return:
            True, False, or None, where True indicates that the page exists, False indicates the
            page does not exist or doesn't contain content, and None indicates that the page is
            hidden.
        """
        if self.is_root:
            return

        if self.removed:
            LOG.debug("Skipping documentation check for %s, it is REMOVED.", self.fullpath)
            return

        if self.hidden:
            return

        # Search for nodes within the page tree
        nodes = root.findall(os.path.join('systems', self.markdown()), exc=None)

        # No markdown pages exist
        if not nodes:

            # Desired filename
            filename = os.path.join(os.getcwd(), 'content', 'documentation', 'systems',
                                    self.markdown())

            if generate and (group not in self.groups):
                msg = "The %s object is not associated with the %s, it is associated with %s. "\
                      "It is not possible to generate stub pages for this object."
                LOG.error(msg, self.fullpath, group, ' '.join(self.groups))

            elif generate:
                if not os.path.exists(os.path.dirname(filename)):
                    os.makedirs(os.path.dirname(filename))
                LOG.info('Creating stub page for %s %s', self.fullpath, filename)
                with open(filename, 'w') as fid:
                    content = self._defaultContent()
                    fid.write(content)

            elif group in self.groups:
                msg = "No documentation for %s.\n"
                msg += "The page should be located at %s\n"
                msg += "It is possible to generate stub pages for your documentation " \
                       "using the './moosedocs.py check --generate' command."
                LOG.error(msg, self.fullpath, filename)

            else:
                msg = "No documentation for %s.\n"
                msg += "The page should be created within one of the following apps: %s"
                LOG.error(msg, self.fullpath, ','.join(self.groups))

        # Examine existing page
        else:
            filename = nodes[0].source
            with open(filename, 'r') as fid:
                lines = fid.readlines()

            if lines and self.STUB_HEADER in lines[0]:
                if update:
                    LOG.info("Updating stub page for %s in file %s.", self.fullpath, filename)
                    with open(filename, 'w') as fid:
                        content = self._defaultContent()
                        fid.write(content)
                else:
                    msg = "A MOOSE generated stub page for %s exists, but no content was " \
                          "added. Add documentation content to %s."
                    LOG.error(msg, self.fullpath, filename)

    def _defaultContent(self):
        """
        Markdown stub content.
        """
        raise NotImplementedError("The _defaultContent method must be defined in child classes "
                                  "and return a string.")

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

    def _defaultContent(self):
        """
        Markdown stub content.
        """
        stub = self.STUB_HEADER
        stub += '\n\n\n# {} System\n\n'.format(self.name)
        stub += '!syntax list {} objects=True actions=False subsystems=False\n\n' \
                .format(self.fullpath)
        stub += '!syntax list {} objects=False actions=False subsystems=True\n\n' \
                .format(self.fullpath)
        stub += '!syntax list {} objects=False actions=True subsystems=False\n\n' \
                .format(self.fullpath)
        return stub

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

    def markdown(self):
        """
        The expected markdown file.
        """
        return self.fullpath.lstrip('/') + '.md'

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

    def _defaultContent(self):
        """
        Markdown stub content.
        """
        template_filename = os.path.join(MooseDocs.MOOSE_DIR, 'framework', 'doc', 'templates',
                                         'moose_object.md.template')
        with open(template_filename, 'r') as fid:
            template_content = fid.read()

        template_content = template_content.replace('FullPathCodeClassName',
                                                    '{}'.format(self.fullpath))
        template_content = template_content.replace('CodeClassName', '{}'.format(self.name))
        stub = self.STUB_HEADER + '\n\n'
        stub += template_content
        return stub

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

    def _defaultContent(self):
        """
        Markdown stub content.
        """
        template_filename = os.path.join(MooseDocs.MOOSE_DIR, 'framework', 'doc', 'templates',
                                         'action_object.md.template')
        with open(template_filename, 'r') as fid:
            template_content = fid.read()

        template_content = template_content.replace('FullPathCodeActionName',
                                                    '{}'.format(self.fullpath))
        template_content = template_content.replace('CodeActionName', '{}'.format(self.name))
        stub = self.STUB_HEADER + '\n\n'
        stub += template_content
        return stub

class MooseObjectActionNode(ActionNode):
    """
    MooseObjectAction nodes.
    """
    COLOR = 'LIGHT_CYAN'
