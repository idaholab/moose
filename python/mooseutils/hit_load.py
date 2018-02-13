"""Wrapper for hit parser."""
import os
import anytree
import hit
import message

class HitNode(anytree.NodeMixin):
    """
    An anytree.Node object for building a hit tree.
    """
    def __init__(self, parent=None, hitnode=None):
        super(HitNode, self).__init__()
        self.name = hitnode.path() # anytree.Node property
        self.parent = parent       # anytree.Node property
        self.__line = hitnode.line()
        self.__hitnode = hitnode   # hit.Node object

    @property
    def line(self):
        """
        Access to the line number.
        """
        return self.__line

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
        return '/'.join(out)

    def find(self, name, fuzzy=True):
        """
        Locate first occurance of a node by name starting from this node.

        Inputs:
            name[str]: The name to search for within the tree.
            fuzzy[bool]: When True (the default) a "fuzzy" search is performed, meaning that the
                         provide name must be in the node name. If this is set to False the names
                         must match exact.
        """
        for node in anytree.PreOrderIter(self):
            if (fuzzy and name in node.name) or (not fuzzy and name == node.name):
                return node

    def findall(self, name, fuzzy=True):
        """
        Locate all nodes withing the tree starting from this node.

        Inputs:
            name[str]: The name to search for within the tree.
            fuzzy[bool]: When True (the default) a "fuzzy" search is performed, meaning that the
                         provide name must be in the node name. If this is set to False the names
                         must match exact.
        """
        filter_ = lambda n: (fuzzy and name in n.name) or (not fuzzy and n.name == name)
        return [node for node in anytree.PreOrderIter(self, filter_=filter_)]

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

    def get(self, name, default):
        """
        Return a parameter, if it does not exist return the default.
        """
        value = self.__hitnode.param(name)
        if value is None:
            return default
        return value

    def __getitem__(self, name):
        """
        Provides operator [] access to the parameters of this node.
        """
        return self.__hitnode.param(name)

    def __repr__(self):
        """
        Dislpay the node name and parameters.
        """
        params = {k:v for k, v in self.iterparams()}
        if params:
            return '{}: {}'.format(self.name, repr(params))
        return self.name

    def __str__(self):
        """
        Print the complete tree beginning at this node.
        """
        return str(anytree.RenderTree(self))

def hit_load(filename):
    """
    Read and parse a hit file (MOOSE input file format).

    Inputs:
        filenae[str]: The filename to open and parse.

    Returns a HitNode object, which is the root of the tree. HitNode objects are custom
    versions of the anytree.Node objects.
    """
    if os.path.exists(filename):
        with open(filename, 'r') as fid:
            content = fid.read()
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
    print root
    print root.render()
