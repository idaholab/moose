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
import hit
import moosetree
from . import message


class HitNode(moosetree.Node):
    """
    An moosetree.Node object for building a hit tree.

    Inputs:
        parent[HitNode]: The parent of the node being created
        hitnode[hit.Node|hit.Kind]: The node or kind being created
    """
    def __init__(self, parent, hitnode):
        super(HitNode, self).__init__(parent, hitnode.path())
        self.__hitnode = hitnode     # hit.Node object

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

    def find(self, name, fuzzy=True):
        """
        Locate first occurrence of a node by name starting from this node.

        Inputs:
            name[str]: The name to search for within the tree.
            fuzzy[bool]: When True (the default) a "fuzzy" search is performed, meaning that the
                         provide name must be in the node name. If this is set to False the names
                         must match exact.
        """
        func = lambda n: (fuzzy and name in n.fullpath) or (not fuzzy and name == n.fullpath)
        return moosetree.find(self, func, method=moosetree.IterMethod.PRE_ORDER)

    def findall(self, name, fuzzy=True):
        """
        Locate all nodes withing the tree starting from this node.

        Inputs:
            name[str]: The name to search for within the tree.
            fuzzy[bool]: When True (the default) a "fuzzy" search is performed, meaning that the
                         provide name must be in the node name. If this is set to False the names
                         must match exact.
        """
        func = lambda n: (fuzzy and name in n.fullpath) or (not fuzzy and name == n.fullpath)
        return moosetree.findall(self, func, method=moosetree.IterMethod.PRE_ORDER)

    def append(self, name, **kwargs):
        """
        Append a new input file block.

        Inputs:
            name[str]: The name of the section to add
            kwargs[dict]: Parameters to populate to the new section
        """
        new = hit.NewSection(name)
        self.__hitnode.addChild(new)
        node = HitNode(self, new)
        for key, value in kwargs.items():
            node.addParam(key, value)
        return node

    def remove(self):
        """
        Remove this node form the tree.
        """
        self.__hitnode.remove()
        self.__hitnode = None
        self.parent = None

    def addParam(self, name, value):
        """
        Add a new parameter to the given node.

        Inputs:
            name[str]: The name of the parameter
            value[Int|Float|Bool|String]: The parameter value
        """
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

    def __iter__(self):
        """
        Provides simple looping over children.

            for child in node:
                ...
        """
        for child in self.children:
            yield child

    def iterparams(self):
        """
        Return key, value for the parameters of this node.

        for k, v in node.iterparams():
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


def hit_load(filename):
    """
    Read and parse a hit file (MOOSE input file format).

    Inputs:
        filename[str]: The filename to open and parse.

    Returns a HitNode object, which is the root of the tree. HitNode objects are custom
    versions of the moosetree.Node objects.
    """
    if os.path.exists(filename):
        with open(filename, 'r') as fid:
            content = fid.read()
    elif isinstance(filename, str):
        content = filename
    else:
        message.mooseError("Unable to load the hit file ", filename)

    hit_node = hit.parse(filename, content)
    root = HitNode(None, hit_node)
    hit_parse(root, hit_node, filename)
    return root

def hit_parse(root, hit_node, filename):
    """
    Parse the supplied content into a hit tree.

    Inputs:
        root[HitNode]: The HitNode object that the raw hit content will be inserted
        content[str]: The raw hit content to parse.
        filename[str]: (optional) The filename for error reporting.

    Returns a HitNode object, which is the root of the tree. HitNode objects are custom
    versions of the moosetree.Node objects.
    """
    for hit_child in hit_node.children():
        if hit_child.type() == hit.NodeType.Section:
            new = HitNode(root, hit_child)
            hit_parse(new, hit_child, filename)

    return root
