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
import hit
from mooseutils import message

class Node(moosetree.Node):
    """
    An [moosetree/Node.md#moosetree.node] object for building a [!ac](HIT) tree.

    Add a new node to the input file tree that is a child of *parent*. The supplied *parent* must
    be `pyhit.Node` object. The *hitnode* is name of the node, as a string.

    The *hitnode* input can also be an object from the hit C bindings. However, this second form is
    not intended for general use; it is used when creating a tree from a file when loading from a
    file.
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
        Return the node full path as a string.
        """
        out = []
        node = self
        while (node is not None):
            out.append(node.name)
            node = node.parent
        return '/'.join(reversed(out))

    def insert(self, index, name, **kwargs):
        """
        Insert a child input block, with the given *name*, to the current block +before+ the *index*.

        The keyword arguments supplied are added as parameters to the inserted block.
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
        Append a child input block, with the given *name*, to the end of current block.

        The keyword arguments supplied are added as parameters to the added block.
        """
        new = hit.NewSection(name)
        self.__hitnode.addChild(new)
        node = Node(self, new)
        for key, value in kwargs.items():
            node.__addParam(key, value)
        return node

    def comment(self, param=None):
        """
        Return a +copy+ of the comment for the block or parameter given by *param*.

        When this method is called without arguments it returns the comments for the block itself.
        When called with the *param* name, the comment for that parameter is returned.

        !alert note title=The "comment" method returns a copy.
        This method returns a copy of the comment text. To modify the comment, the "setComment"
        method must be used.
        """
        comment = self.__hitparamcomments.get(param, self.__hitblockcomment)
        if comment is not None:
            return str(comment).strip('\n# ')

    def setComment(self, *args):
        """
        Add/Set comment for the block or parameter.

        There are two modes of operation. The first (`setComment(text)`) sets the comment of the
        block itself to the supplied *text*. The second (`setComment(param, text)` sets the comment
        of the supplied *param* with the value of *text*.
        """
        if len(args) == 1:
            param = None
            text = args[0]
        elif len(args) == 2:
            param, text = args

        comment = self.__hitparamcomments.get(param, self.__hitblockcomment)
        if (comment is not None) and (text is not None):
            comment.setText('# {}'.format(text))

        if (comment is not None) and (text is None):
            if comment is self.__hitblockcomment:
                self.__hitblockcomment.remove()
                self.__hitblockcomment = None
            else:
                comment.remove()

        elif (comment is None) and (param is None) and (text is not None):
            self.parent.__hitnode.insertChild(self.__hitoffset, hit.NewComment('# {}'.format(text)))
            self.__reinitComments()

        elif (comment is None) and (param is not None) and (text is not None):
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
        Remove the supplied parameter with *name* from the node.
        """
        for child in self.__hitnode.children(hit.NodeType.Field):
            if child.path() == name:
                child.remove()

    def format(self, **kwargs):
        """
        Return a string of the node that is rendered with C hit library with formatting.

        An optional keyword argument *canonical_section_markers* can be supplied with `True` or
        `False` to enable/disable the use of "./" and "../" section markings. By default these
        markings this are removed.
        """
        formatter = hit.Formatter()
        formatter.config(**kwargs)
        formatter.formatTree(self.__hitnode)
        return self.render()

    def render(self, **kwargs):
        """
        Return a string of the node this rendered with the C hit library without formatting.
        """
        return self.__hitnode.render()

    def __contains__(self, name):
        """
        Provides operator in access to the parameters of this node.

        ```python
        if 'foo' in param:
                ...
        ```
        """
        return self.__hitnode.param(name) is not None

    def params(self):
        """
        Return key, value for the parameters of this node.

        ```python
        for k, v in node.params():
            ...
        ```
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
        Return the line number for node itself or for the supplied *name*.

        When the *name* is not included the line number for the beginning of the block is returned;
        and when it is included the line number is returned.

        When *name* is used and the parameter is not located, the *default* is returned.
        """
        if name is None:
            return self.__hitnode.line()

        for child in self.__hitnode.children(hit.NodeType.Field):
            if child.path() == name:
                return child.line()

        return default

    def filename(self, name=None, default=None):
        """
        Return the file name for node itself or for the supplied *name*.
        When the *name* is not included the file name for the beginning of the block is returned;
        and when it is included the file name is returned.
        When *name* is used and the parameter is not located, the *default* is returned.
        """
        if name is None:
            return self.__hitnode.filename()

        for child in self.__hitnode.children(hit.NodeType.Field):
            if child.path() == name:
                return child.filename()

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
    Read and parse a HIT file given in *filename*.

    The function return a `pyhit.Node` object which is the root node of the loaded tree. The
    specific node object that should be populated can be supplied with the *root* input. If it is
    provided this same node will be returned.
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
    Write the supplied tree in *root* to a text file *filename*.
    """
    with open(filename, 'w') as fid:
        fid.write(root.render() + "\n")

def parse(content, root=None, filename=''):
    """
    Parse a hit tree from a *content* string and return a `pyhit.Node` object.

    The returned object is the root of the loaded tree. The *root* input can provide a node object
    for the tree to populate; if it is given this same node is returned. The *filename*, if provided,
    will be used for error reporting when manipulating the tree.
    """
    hit_node = hit.parse(filename, content)
    hit.explode(hit_node)
    root = Node(root, hit_node) if root is not None else Node(None, hit_node)
    _parse_hit(root, hit_node, filename)
    return root

def tokenize(content, filename=''):
    """
    Tokenize a hit tree from a string.

    Returns a token tree for the supplied *content* tree. The tokens returned are defined by the core
    python bindings to the C library. The *filename*, if provided, will be used for error reporting
    when manipulating the tree.
    """
    return hit.tokenize(filename, content)

def _parse_hit(root, hit_node, filename):
    """Internal helper for parsing HIT tree"""
    offset = 0
    for hit_child in hit_node.children():
        if hit_child.type() == hit.NodeType.Section:
            new = Node(root, hit_child, offset=offset)
            _parse_hit(new, hit_child, filename)
        offset += 1
    return root
