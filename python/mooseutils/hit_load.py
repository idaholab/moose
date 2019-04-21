#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

"""Wrapper for hit parser."""
from __future__ import print_function
import os
import hit
from . import message

# The 'HitNode' object is used within the TestHarness, which should operate without any
# special python libraries. However, the 'anytree' package is used by various utilities within the
# moose/python tools (e.g., MooseDocs). It is useful to have this hit wrapper use the anytree
# package for consistency. Therefore, the following allows the HitNode to work with or without it.
try:
    import anytree
    from anytree import NodeMixin
    HAVE_ANYTREE = True

except ImportError:
    HAVE_ANYTREE = False

    class NodeMixin(object):
        """Proxy for anytree.NodeMixin"""
        def __init__(self):
            self._parent = None
            self.name = None
            self.children = list()

        @property
        def parent(self):
            return self._parent

        @parent.setter
        def parent(self, value):
            self._parent = value
            if self._parent:
                self._parent.children.append(self)

class HitNode(NodeMixin):
    """
    An anytree.Node object for building a hit tree.
    """
    def __init__(self, parent=None, hitnode=None):
        super(HitNode, self).__init__()
        self.name = hitnode.path()   # anytree.Node property
        self.parent = parent         # anytree.Node property
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
        if HAVE_ANYTREE:
            for node in anytree.PreOrderIter(self):
                if (fuzzy and name in node.fullpath) or (not fuzzy and name == node.fullpath):
                    return node
        else:
            msg = "The 'find' method requires the 'anytree' python package. This can " \
                  "be installed via your python package manager (e.g., pip install anytree --user)."
            message.mooseError(msg)

    def findall(self, name, fuzzy=True):
        """
        Locate all nodes withing the tree starting from this node.

        Inputs:
            name[str]: The name to search for within the tree.
            fuzzy[bool]: When True (the default) a "fuzzy" search is performed, meaning that the
                         provide name must be in the node name. If this is set to False the names
                         must match exact.
        """
        if HAVE_ANYTREE:
            filter_ = lambda n: (fuzzy and name in n.name) or (not fuzzy and n.name == name)
            return [node for node in anytree.PreOrderIter(self, filter_=filter_)]
        else:
            msg = "The 'findall' method requires the 'anytree' python package. This can " \
                  "be installed via your python package manager (e.g., pip install anytree --user)."
            message.mooseError(msg)

    def render(self):
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

    def __repr__(self):
        """
        Display the node name and parameters.
        """
        params = {k:v for k, v in self.iterparams()}
        if params:
            return '{}: {}'.format(self.name, repr(params))
        return self.name

    def __str__(self):
        """
        Print the complete tree beginning at this node.
        """
        if HAVE_ANYTREE:
            return str(anytree.RenderTree(self))
        return self.__class__

def hit_load(filename):
    """
    Read and parse a hit file (MOOSE input file format).

    Inputs:
        filename[str]: The filename to open and parse.

    Returns a HitNode object, which is the root of the tree. HitNode objects are custom
    versions of the anytree.Node objects.
    """
    if os.path.exists(filename):
        with open(filename, 'r') as fid:
            content = fid.read()
    elif isinstance(filename, (str, unicode)):
        content = filename
    else:
        message.mooseError("Unable to load the hit file ", filename)

    hit_node = hit.parse(filename, content)
    root = HitNode(hitnode=hit_node)
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
    versions of the anytree.Node objects.
    """
    for hit_child in hit_node.children():
        if hit_child.type() == hit.NodeType.Section:
            new = HitNode(parent=root, hitnode=hit_child)
            hit_parse(new, hit_child, filename)

    return root

if __name__ == '__main__':
    filename = '../../test/tests/kernels/simple_diffusion/simple_diffusion.i'
    root = hit_load(filename)
    print(root)
    print(root.render())
