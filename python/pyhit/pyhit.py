#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

"""Wrapper for hit parser."""
import os
import subprocess
import moosetree
from mooseutils import message
try:
    from . import hit
except:
    testdir = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..', 'test'))
    subprocess.run(['make', 'hit'], cwd=testdir)
    from . import hit

class Node(moosetree.Node):
    """
    An moosetree.Node object for building a hit tree.

    Inputs:
        parent[Node]: The parent of the node being created
        name[str] or hitnode[hit.Node]: The name of the node or the hit.Node parent
    """
    def __init__(self, parent=None, hitnode=None, offset=0):
        if isinstance(hitnode, str):
            hitnode = hit.NewSection(hitnode)
        elif hitnode is None:
            hitnode = hit.NewSection('')
        super().__init__(parent, hitnode.path())
        self.__hitnode = hitnode         # hit.Node object
        self.__hitblockcomment = None    # hit.Comment object for this block
        self.__hitparamcomments = dict() # hit.Comment objects for the parameters within this block
        self.__hitoffset = offset        # hit index used for inserting new hit nodes
        self.__reinitComments()

    @property
    def fullpath(self):
        """
        Return the node full path.
        """
        out = []
        node = self
        while (node is not None):
            out.append(node.name)
            node = node.parent
        return '/'.join(reversed(out))

    def insert(self, index, name, **kwargs):
        """
        Insert a new input file block before the supplied index

        Inputs:
            index[int]: The index to insert before
            name[str]: The name of the section to add
            kwargs[dict]: Parameters to populate to the new section
        """
        count = 0
        hit_index = 0
        for child in self.__hitnode.children():
            if count == index:
                break
            hit_index += 1
            if child.type() == hit.NodeType.Section:
                count += 1

        new = hit.NewSection(name)
        self.__hitnode.insertChild(hit_index, new)
        node = Node(None, new)
        super().insert(index, node)
        for key, value in kwargs.items():
            node.__addParam(key, value)
        return node

    def append(self, name, **kwargs):
        """
        Append a new input file block.

        Inputs:
            name[str]: The name of the section to add
            kwargs[dict]: Parameters to populate to the new section
        """
        new = hit.NewSection(name)
        self.__hitnode.addChild(new)
        node = Node(self, new)
        for key, value in kwargs.items():
            node.__addParam(key, value)
        return node

    def comment(self, param=None):
        """
        Return a copy of the comment for the block or parameter
        """
        comment = self.__hitparamcomments.get(param, self.__hitblockcomment)
        if comment is not None:
            return str(comment).strip('\n# ')

    def setComment(self, *args):
        """
        Add/Set comment for the block or parameter.

        Usage:
            setComment("comment")
            setComment("param", "comment")
        """
        if len(args) == 1:
            param = None
            text = args[0]
        elif len(args) == 2:
            param, text = args

        comment = self.__hitparamcomments.get(param, self.__hitblockcomment)
        if comment is not None:
            comment.setText('# {}'.format(text))

        elif (comment is None) and (param is None):
            self.parent.__hitnode.insertChild(self.__hitoffset, hit.NewComment('# {}'.format(text)))
            self.__reinitComments()

        elif (comment is None) and (param is not None):
            for child in self.__hitnode.children(hit.NodeType.Field):
                if child.path() == param:
                    child.addChild(hit.NewComment('# {}'.format(text), True))
                    self.__reinitComments()
                    break

    def remove(self):
        """
        Remove this node form the tree.
        """
        self.__hitnode.remove()
        self.__hitnode = None
        self.parent = None

    def removeParam(self, name):
        """
        Remove the supplied parameter.

        Inputs:
            name[str]: The name of the parameter
        """
        for child in self.__hitnode.children(hit.NodeType.Field):
            if child.path() == name:
                child.remove()

    def format(self, **kwargs):
        """
        Render tree with formatting.

        Inputs:
            canonical_section_markers[bool]: Enable/disable use of "./" and "../" section markings,
                                             by default the markings are removed.
        """
        formatter = hit.Formatter()
        formatter.config(**kwargs)
        formatter.formatTree(self.__hitnode)
        return self.render()

    def render(self, **kwargs):
        """
        Render the tree with the hit library.
        """
        return self.__hitnode.render()

    def __contains__(self, name):
        """
        Provides operator in access to the parameters of this node.

            if 'foo' in param:
                ...
        """
        return self.__hitnode.param(name) is not None

    def params(self):
        """
        Return key, value for the parameters of this node.

        for k, v in node.params():
            ...
        """
        for child in self.__hitnode.children(hit.NodeType.Field):
            yield child.path(), child.param()

    def get(self, name, default=None):
        """
        Return a parameter, if it does not exist return the default.
        """
        value = self.__hitnode.param(name)
        if value is None:
            return default
        return value

    def line(self, name=None, default=None):
        """
        Return the line number for node or for the supplied parameter name.

        Inputs:
            name[str]: (Optional) The name of a parameter, if not given the node line is returned.
            default: (Optional) The default value to return if the parameter name is not found.
        """
        if name is None:
            return self.__hitnode.line()

        for child in self.__hitnode.children(hit.NodeType.Field):
            if child.path() == name:
                return child.line()

        return default

    def __getitem__(self, name):
        """
        Provides operator [] access to the parameters of this node.
        """
        return self.__hitnode.param(name)

    def __setitem__(self, name, value):
        """
        Provide operator [] for modifying or adding parameters to this node.

        Inputs:
            name[str]: The name of the parameter
            value[int|float|bool|str]: The parameter value
        """
        if name not in self:
            self.__addParam(name, value)
        else:
            self.__editParam(name, value)

    def __addParam(self, name, value):
        """(private) Add a new parameter to the given node."""
        if isinstance(value, int):
            kind = hit.FieldKind.Int
        elif isinstance(value, float):
            kind = hit.FieldKind.Float
        elif isinstance(value, bool):
            kind = hit.FieldKind.Bool
        elif isinstance(value, str):
            kind = hit.FieldKind.String
        else:
            kind = hit.FieldKind.NotField

        param = hit.NewField(name, kind, str(value))
        self.__hitnode.addChild(param)

    def __editParam(self, name, value):
        """(private) Edit an existing parameter"""
        retcode = self.__hitnode.setParam(name, str(value))
        if retcode != 0:
            raise KeyError("Unknown parameter name '{}'".format(name))

    def __reinitComments(self):
        """(private) Cache comment items for easy access"""

        self.__hitparamcomments.clear()
        for child in self.__hitnode.children(hit.NodeType.Field):
            comment = None
            if child.children() and (child.children()[0].type() == hit.NodeType.Comment):
                comment = child.children()[0]
            self.__hitparamcomments[child.path()] = comment

        self.__hitblockcomment = None
        if (self.parent is not None):
            comment = None
            for child in self.parent.__hitnode.children():
                if child.type() == hit.NodeType.Comment:
                    comment = child
                if (child.path() == self.__hitnode.path()) and (comment is not None):
                    self.__hitblockcomment = comment
                    break

def load(filename, root=None):
    """
    Read and parse a hit file (MOOSE input file format).

    Inputs:
        filename[str]: The filename to open and parse.
        root[Node]: (Optional) The root node of the tree

    Returns a Node object, which is the root of the tree. Node objects are custom
    versions of the moosetree.Node objects.
    """
    if os.path.exists(filename):
        with open(filename, 'r') as fid:
            content = fid.read()
    elif isinstance(filename, str):
        content = filename
    else:
        message.mooseError("Unable to load the hit file ", filename)

    return parse(content, root, filename)

def write(filename, root):
    """
    Write the supplied tree to the file.

    Inputs:
        filename[str]: The filename to open and parse
        root[Node]: The root node of the tree to write
    """
    with open(filename, 'w') as fid:
        fid.write(root.render() + "\n")

def parse(content, root=None, filename=''):
    """
    Parse a hit tree from a string.

    Inputs:
       content[str]: string to process
       root[Node]: (Optional) root node of the tree
       filename[str]: (Optional) filename for error reporting

    """
    hit_node = hit.parse(filename, content)
    hit.explode(hit_node)
    root = Node(root, hit_node) if root is not None else Node(None, hit_node)
    _parse_hit(root, hit_node, filename)
    return root

def _parse_hit(root, hit_node, filename):
    """Internal helper for parsing HIT tree"""
    offset = 0
    for hit_child in hit_node.children():
        if hit_child.type() == hit.NodeType.Section:
            new = Node(root, hit_child, offset=offset)
            _parse_hit(new, hit_child, filename)
        offset += 1
    return root
