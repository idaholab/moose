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

import os
import re
import collections
import logging
import anytree
import mooseutils
import MooseDocs

LOG = logging.getLogger(__name__)

class NodeCore(anytree.NodeMixin):
    """
    A general NodeMixin that provides color printing and the full_name property. This serves as
    the base class for all node used throughout MooseDocs.

    Inputs:
        name[str]: The name of the node.
        parent[NodeCore]: The parent node, use None to create a root node.
    """
    COLOR = 'RESET'
    def __init__(self, name, parent=None, display=None, root_directory=None):
        super(NodeCore, self).__init__()
        self.parent = parent
        self.name = name
        self.status = collections.defaultdict(int)
        self._root_directory = root_directory if root_directory else MooseDocs.ROOT_DIR
        self.__cache = dict()

        self._display = display
        if self._display is None:
            self._display = name

    @property
    def root_directory(self):
        """
        Return the root directory of the file (default if MooseDocs.ROOT_DIR)
        """
        return self._root_directory

    @property
    def full_name(self):
        """
        Return the full name of the node, recursively call the name of parents up to the root.
        """
        if self.parent is not None:
            return self.separator.join([self.parent.full_name, self.name]).replace('//', '/')
        return self.name

    def findall(self, name='', filter_=None):
        """
        Locate nodes based on a filter. By

        Args:
            name[str]: (optional) When the 'filter_' options is not supplied, this name is used
                       in the default filter (default: '').
            filter_[function]: A filter function, if it returns true keep the node.  If not supplied
                               the default is to search for all nodes that have a "full_name" that
                               ends with the name provided in the 'name' argument. If the 'name'
                               argument is not set the default will return all nodes. The supplied
                               function should accept the node as an argument.
        """
        if name and (filter_ is not None) and (name in self.__cache):
            return self.__cache[name]

        if filter_ is None:
            filter_ = lambda n: n.full_name.endswith(name)
        nodes = [node for node in anytree.iterators.PreOrderIter(self.root, filter_=filter_)]

        if name and (filter_ is not None):
            self.__cache[name] = nodes

        return nodes

    def replace(self, node):
        """
        Replace the current node with the node provided.

        Args:
            node[anytree.NodeMixin]: The node that is replacing the current node.
        """
        node.parent = self.parent
        for child in self.children:
            child.parent = node
        self.parent = None
        return node

    def reset(self):
        """
        Called before a page is re-created.
        """
        self.status.clear()

    @property
    def display(self):
        """
        Return the display name.
        """
        return self._display

    def __repr__(self):
        """
        Print the node name.
        """
        oname = self.__class__.__name__[:-4]
        msg = '{}: {}'.format(oname, self.full_name)
        return mooseutils.colorText(msg, self.COLOR)

    def __str__(self):
        """
        Calling print on this object will print the tree nice and pretty.
        """
        return str(anytree.RenderTree(self))

class MarkdownNode(NodeCore):
    """
    A simple node that accepts markdown content as raw text.
    """
    def __init__(self, name, content=None, **kwargs):
        super(MarkdownNode, self).__init__(name, **kwargs)
        self._content = content
    @property
    def filename(self):
        """
        Provide a 'filename' for when MooseMarkdown object is used with a raw string.
        """
        return 'supplied string.'

    @property
    def content(self):
        """
        Return the supplied markdown content.
        """
        return self._content

class FileTreeNodeBase(NodeCore):
    """
    Base node type for the markdown file tree.
    """
    COLOR = 'YELLOW'
    def __init__(self, name, base=None, **kwargs):
        super(FileTreeNodeBase, self).__init__(name, **kwargs)
        if base is None:
            if self.parent:
                base = self.parent.base
            else:
                base = ''
        self._base = base

    @property
    def base(self):
        """
        Return the 'base' location.
        """
        return self._base

class DirectoryNode(FileTreeNodeBase):
    """
    This node is used when for directories that do not contain 'index.md' files. This is a separate
    class for color output as to have a distinct class for comparing against.
    """
    @property
    def destination(self):
        """
        Return in invalid location, this is needed for breadcrumbs.
        """
        return None

class FileNodeBase(FileTreeNodeBase):
    """
    Base class for building file tree for converting markdown and copying files (e.g., png, js).
    """

    @property
    def basename(self):
        """
        Return the absolute "base name" of the node.
        """
        return os.path.join(self.root_directory, self._base, self.full_name.strip('/'))

    @property
    def filename(self):
        """
        Return the absolute path to the file.
        """
        raise NotImplementedError("The 'filename' property must be defined.")

    @property
    def destination(self):
        """
        Return the local path to the html file to be created, i.e. the name the full_name.
        """
        return os.path.join(self.full_name.strip('/'), 'index.html')

class MarkdownFileNodeBase(FileNodeBase): #pylint: disable=abstract-method
    """
    Base class for node that is part of the markdown file tree and has an associated markdown file.

    As with all paths in MooseDocs it is assumed that all supplied paths are given relative to the
    repository root directory (i.e., ROOT_DIR).

    Inputs:
       name[str]: The name of the node.
       base[str]: The base directory name that is inserted into the filename before the 'full_name'.
       parent[NodeCore]: The parent node, use None to create a root node.
    """
    @property
    def content(self):
        """
        Return the raw markdown content.
        """
        with open(self.filename, 'r') as fid:
            return fid.read().decode('utf-8')

class CopyFileNode(FileNodeBase):
    """
    File tree node for general files that need be copied and linked.
    """
    COLOR = 'BLUE'
    @property
    def filename(self):
        return os.path.join(self.basename)

    @property
    def destination(self):
        return self.full_name.lstrip('/')

class MarkdownFileIndexNode(MarkdownFileNodeBase):
    """
    Node for directories that contain a index.md file within the build file tree.
    """
    COLOR = 'MAGENTA'
    @property
    def filename(self):
        return os.path.join(self.basename, 'index.md')

    @property
    def display(self):
        if self.name == '':
            return 'home'
        else:
            return self._display

class MarkdownFilePageNode(MarkdownFileNodeBase):
    """
    Node for markdown files (e.g., Diffusion.md) within the build file tree.
    """
    COLOR = 'CYAN'
    @property
    def filename(self):
        return self.basename + '.md'

class SyntaxNodeBase(NodeCore):
    """
    Node for MOOSE syntax that serves as the parent for actions/objects.
    """
    STUB_HEADER = '<!-- MOOSE Documentation Stub: Remove this when content is added. -->\n'

    def __init__(self, name, **kwargs):
        super(SyntaxNodeBase, self).__init__(name, **kwargs)
        self.__hidden = False
        self.__check_status = None

    @property
    def hidden(self):
        """
        Return the hidden status of the node.
        """
        return self.__hidden

    @hidden.setter
    def hidden(self, value):
        """
        Set the hidden status for the node.
        """
        if isinstance(value, bool):
            self.__hidden = value
        else:
            raise TypeError("The supplied value must be a boolean.")

    @property
    def groups(self):
        """
        Return groups associated with this node or entire tree (i.e., where the syntax is defined).
        """
        out = dict()
        for node in self.descendants:
            if isinstance(node, ActionNode):
                out.update(node.groups)
        return out

    def hasGroups(self, groups):
        """
        Return True if ANY of the supplied groups exist in this object or children of this object.
        """
        all_groups = set()
        for node in self.descendants:
            all_groups.update(node.groups.keys())
        return len(all_groups.intersection(groups)) > 0

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

    def markdown(self, install, absolute=True):
        """
        Return the expected markdown file name.
        """
        raise NotImplementedError("The 'markdown' method must return the expected markdown "
                                  "filename.")

    def check(self, install, generate=False, groups=None, update=None):
        """
        Check that the expected documentation exists.

        Return:
            True, False, or None, where True indicates that the page exists, False indicates the
            page does not exist or doesn't contain content, and None indicates that the page is
            hidden.
        """
        out = None # not checked because it was hidden
        if self.hidden:
            LOG.debug("Skipping documentation check for %s, it is hidden.", self.full_name)

        elif groups and not set(self.groups).intersection(groups):
            LOG.debug("Skipping documentation check for %s (%s), it is not listed in the provided "
                      "groups: %s.", self.full_name, self.groups.keys(), groups)

        else:
            filename = self.markdown(install)
            if not os.path.isfile(filename):
                out = False
                LOG.error("No documentation for %s, documentation for this object should be "
                          "created in: %s", self.full_name, filename)
                if generate:
                    if not os.path.exists(os.path.dirname(filename)):
                        os.makedirs(os.path.dirname(filename))
                    LOG.info('Creating stub page for %s %s', self.full_name, filename)
                    with open(filename, 'w') as fid:
                        content = self._defaultContent()
                        if not isinstance(content, str):
                            raise TypeError("The _defaultContent method must return a str.")
                        fid.write(content)
            else:
                with open(filename, 'r') as fid:
                    lines = fid.readlines()
                if lines and self.STUB_HEADER in lines[0]:
                    out = False
                    LOG.error("A MOOSE generated stub page for %s exists, but no content was "
                              "added. Add documentation content to %s.", self.name, filename)
                    if update:
                        LOG.info("Updating stub page for %s in file %s.", self.name, filename)
                        with open(filename, 'w') as fid:
                            content = self._defaultContent()
                            if not isinstance(content, str):
                                raise TypeError("The _defaultContent method must return a str.")
                            fid.write(content)
        return out

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
            syntax: (optional) The syntax that must be within the object 'full_name' property.
            group: (optional) The group to limit the search.
            recursive: When True the search will look through all nodes in the entire tree, when
                       False only the children of the node are considered.
        """
        if recursive:
            filter_ = lambda node: (syntax in node.full_name) and \
                                   isinstance(node, node_type) and \
                                   (group is None or group in node.groups)
            return self.findall(filter_=filter_)

        else:
            return [node for node in self.children if (syntax in node.full_name) and \
                                                      isinstance(node, node_type) and \
                                                      (group is None or group in node.groups)]

    def __repr__(self):
        """
        Print the node name.
        """
        oname = self.__class__.__name__[:-4]
        msg = '{}: {} hidden={} groups={}'.format(oname,
                                                  str(self.full_name),
                                                  self.hidden,
                                                  self.groups.keys())
        return mooseutils.colorText(msg, self.COLOR)

class SyntaxNode(SyntaxNodeBase):
    """
    Defines a class for syntax only (i.e., a node not attached to a C++ class).

    This needs to be a separate class for type checking.
    """
    COLOR = 'GREEN'

    def markdown(self, install, absolute=True):
        """
        Return the expected markdown file name.
        """
        path = os.path.join(install, self.full_name.strip('/')).split('/')
        path += ['index.md']
        if absolute:
            return os.path.join(self.root_directory, *path)
        else:
            return os.path.join(*path)

    def _defaultContent(self):
        """
        Markdown stub content.
        """
        stub = self.STUB_HEADER
        stub += '\n# {} System\n'.format(self.name)
        stub += '!syntax objects {}\n\n'.format(self.full_name)
        stub += '!syntax subsystems {}\n\n'.format(self.full_name)
        stub += '!syntax actions {}\n'.format(self.full_name)
        return stub

class ObjectNode(SyntaxNodeBase): #pylint: disable=abstract-method
    """
    Base class for nodes associated with C++ objects (Action, MooseObjectAction, or MooseObject).
    """
    def __init__(self, name, item, **kwargs):
        super(ObjectNode, self).__init__(name, **kwargs)
        self.__description = item['description']
        self.__parameters = item['parameters']
        self.__groups = dict()

        self._locateGroupNames(item)
        if 'tasks' in item:
            for values in item['tasks'].itervalues():
                self._locateGroupNames(values)

    @property
    def description(self):
        """
        Return the object description.
        """
        return self.__description

    @property
    def parameters(self):
        """
        Return the object parameters.
        """
        return self.__parameters

    def markdown(self, install, absolute=True):
        """
        The expected markdown file.
        """
        folder = self.__groups.keys()[0]
        path = os.path.join(install, self.full_name.strip('/')).split('/')
        path.insert(-1, folder)
        if absolute:
            return os.path.join(self.root_directory, '/'.join(path) + '.md')
        else:
            return os.path.join(*path) + '.md'

    @property
    def groups(self):
        """
        Return groups associated with this node or entire tree (i.e., where the syntax is defined).
        """
        return self.__groups

    def _locateGroupNames(self, item):
        """
        Creates a list of groups (i.e., Apps).
        """
        if 'file_info' in item:
            for info in item['file_info'].iterkeys():
                match = re.search(r'/(?P<group>\w+)(?:App|Syntax)\.C', info)
                if match:
                    heading = re.sub(r'(?<=[a-z])([A-Z])', r' \1', match.group('group'))
                    folder = heading.replace(' ', '_').lower()
                    self.__groups[folder] = heading
                else:
                    self.__groups['framework'] = 'Framework'


class MooseObjectNode(ObjectNode):
    """
    MooseObject nodes.
    """
    COLOR = 'YELLOW'

    def __init__(self, key, item, **kwargs):
        super(MooseObjectNode, self).__init__(key, item, **kwargs)
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
        template_filename = os.path.join(MooseDocs.MOOSE_DIR,
                                         'docs/templates/standards/moose_object.md.template')
        with open(template_filename, 'r') as fid:
            template_content = fid.read()

        template_content = template_content.replace('FullPathCodeClassName',
                                                    '{}'.format(self.full_name))
        template_content = template_content.replace('CodeClassName', '{}'.format(self.name))
        stub = self.STUB_HEADER
        stub += template_content
        return stub

class ActionNode(ObjectNode):
    """
    Action nodes.
    """
    COLOR = 'MAGENTA'

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
        template_filename = os.path.join(MooseDocs.MOOSE_DIR,
                                         'docs/templates/standards/'
                                         'action_object.md.template')
        with open(template_filename, 'r') as fid:
            template_content = fid.read()

        template_content = template_content.replace('FullPathCodeActionName',
                                                    '{}'.format(self.full_name))
        template_content = template_content.replace('CodeActionName', '{}'.format(self.name))
        stub = self.STUB_HEADER
        stub += template_content
        return stub

class MooseObjectActionNode(ActionNode):
    """
    MooseObjectAction nodes.
    """
    COLOR = 'CYAN'
