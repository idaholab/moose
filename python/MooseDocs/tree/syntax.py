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
import re
import logging
import copy

import anytree

import mooseutils

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
                  Property('description', ptype=unicode)]

    # Default documentation destinations for MOOSE and Modules
    #TODO: Build this automatically from source or --registry, not sure how yet.
    DESTINATIONS = dict()
    DESTINATIONS['XFEM'] = '${MOOSE_DIR}/modules/xfem/doc/content/documentation/systems'
    DESTINATIONS['NavierStokes'] = '${MOOSE_DIR}/modules/navier_stokes/doc/content/documentation/' \
                                   'systems'
    DESTINATIONS['TensorMechanics'] = '${MOOSE_DIR}/modules/tensor_mechanics/doc/content/' \
                                      'documentation/systems'
    DESTINATIONS['PhaseField'] = '${MOOSE_DIR}/modules/phase_field/doc/content/documentation/' \
                                 'systems'
    DESTINATIONS['Rdg'] = '${MOOSE_DIR}/modules/rdg/doc/content/documentation/systems'
    DESTINATIONS['Contact'] = '${MOOSE_DIR}/modules/contact/doc/content/documentation/systems'
    DESTINATIONS['SolidMechanics'] = '${MOOSE_DIR}/modules/solid_mechanics/doc/content/' \
                                     'documentation/systems'
    DESTINATIONS['HeatConduction'] = '${MOOSE_DIR}/modules/heat_conduction/doc/content/' \
                                     'documentation/systems'
    DESTINATIONS['MOOSE'] = '${MOOSE_DIR}/framework/doc/content/documentation/systems'
    DESTINATIONS['StochasticTools'] = '${MOOSE_DIR}/modules/stochastic_tools/doc/content/' \
                                      'documentation/systems'
    DESTINATIONS['Misc'] = '${MOOSE_DIR}/modules/misc/doc/content/documentation/systems'
    DESTINATIONS['FluidProperties'] = '${MOOSE_DIR}/modules/fluid_properties/doc/content/' \
                                      'documentation/systems'
    DESTINATIONS['ChemicalReactions'] = '${MOOSE_DIR}/modules/chemical_reactions/doc/content/' \
                                        'documentation/systems'
    DESTINATIONS['LevelSet'] = '${MOOSE_DIR}/modules/level_set/doc/content/documentation/systems'
    DESTINATIONS['PorousFlow'] = '${MOOSE_DIR}/modules/porous_flow/doc/content/documentation/' \
                                 'systems'
    DESTINATIONS['Richards'] = '${MOOSE_DIR}/modules/richards/doc/content/documentation/systems'
    DESTINATIONS['FunctionalExpansionTools'] = '${MOOSE_DIR}/modules/functional_expansion_tools/' \
                                               'doc/content/documentation/systems'

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
        if 'Moose' in out:
            out.remove('Moose')
            out.add('MOOSE')
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

    @property
    def content(self):
        """Return the markdown content."""
        return common.read(self._filename)

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

    def check(self, generate=False, groups=None, update=None):
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

        groups = groups if groups is not None else self.groups
        for group in groups:

            # Locate all the possible locations for the markdown
            filenames = []
            for group in self.groups:
                install = SyntaxNodeBase.DESTINATIONS.get(group,
                                                          'doc/content/documentation/systems')
                install = mooseutils.eval_path(install)
                if not os.path.isabs(install):
                    filename = os.path.join(MooseDocs.ROOT_DIR, install, self.markdown())
                else:
                    filename = os.path.join(install, self.markdown())
                filenames.append((filename, os.path.isfile(filename)))

            # Determine the number of files that exist
            count = sum([x[1] for x in filenames])

            # Class description
            if isinstance(self, ObjectNode) and not self.description and not self.hidden:
                msg = "The class description is missing for %s, it can be added using the " \
                      "'addClassDescription' method from within the objects validParams function."
                LOG.error(msg, self.fullpath)

            # Error if multiple files exist
            if count > 1 and not self.hidden:
                msg = "Multiple markdown files were located for the '{}' syntax:". \
                      format(self.fullpath)
                for filename, exists in filenames:
                    if exists:
                        msg += '\n  {}'.format(filename)
                LOG.error(msg)

            # Error if no files exist
            elif count == 0:
                if not self.hidden:
                    msg = "No documentation for %s, documentation for this object should be " \
                          "created in one of the following locations:"
                    for filename, _ in filenames:
                        msg += '\n  {}'.format(filename)
                    LOG.error(msg, self.fullpath)

                if generate:
                    if not os.path.exists(os.path.dirname(filename)):
                        os.makedirs(os.path.dirname(filename))
                    LOG.info('Creating stub page for %s %s', self.fullpath, filename)
                    with open(filename, 'w') as fid:
                        content = self._defaultContent()
                        if not isinstance(content, str):
                            raise TypeError("The _defaultContent method must return a str.")
                        fid.write(content)
            else:
                for name, exists in filenames:
                    if exists:
                        filename = name

                with open(filename, 'r') as fid:
                    lines = fid.readlines()

                if lines and self.STUB_HEADER in lines[0]:
                    if not self.hidden:
                        msg = "A MOOSE generated stub page for %s exists, but no content was " \
                              "added. Add documentation content to %s."
                        LOG.error(msg, self.fullpath, filename)

                    if update:
                        LOG.info("Updating stub page for %s in file %s.", self.fullpath, filename)
                        with open(filename, 'w') as fid:
                            content = self._defaultContent()
                            if not isinstance(content, str):
                                raise TypeError("The _defaultContent method must return a str.")
                            fid.write(content)


                elif self.hidden and isinstance(self, ObjectNode) and self.description:
                    msg = "The MOOSE syntax %s is listed has hidden; however, a modified page " \
                          "exists for the object, please remove the syntax from the list of " \
                          "hidden list."
                    LOG.error(msg, self.fullpath)


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

        msg = '{}: {} hidden={} removed={} groups={}'.format(self.name,
                                                             str(self.fullpath),
                                                             self.hidden,
                                                             self.removed,
                                                             self.groups)
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

        self._locateGroupNames(item)
        if 'tasks' in item:
            for values in item['tasks'].itervalues():
                self._locateGroupNames(values)

    def markdown(self):
        """
        The expected markdown file.
        """
        return self.fullpath.lstrip('/') + '.md'

    def _locateGroupNames(self, item):
        """
        Creates a list of groups (i.e., Apps).
        """
        if 'file_info' in item:
            for info in item['file_info'].iterkeys():
                match = re.search(r'/(?P<group>\w+)(?:App|Syntax)\.C', info)
                if match:
                    self._groups.add(match.group('group'))

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
