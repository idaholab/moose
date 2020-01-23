#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html
import logging
import moosetree
import mooseutils

LOG = logging.getLogger(__name__)

class NodeBase(moosetree.Node):
    """
    Base class for tree nodes that accepts arbitrary attributes, see moosetree.Node.
    """

    #The color to print (see mooseutils.colorText).
    COLOR = 'RESET'

    def __init__(self, name, parent, **kwargs):
        super(NodeBase, self).__init__(parent, name, **kwargs)

    def __repr__(self):
        """
        Prints the name of the token, this works in union with __str__ to print
        the tree structure of any given node.
        """
        return mooseutils.colorText(moosetree.Node.__repr__(self), self.COLOR)

    def write(self):
        """
        Method for outputting content of node to a string.
        """
        out = ''
        for child in self.children:
            out += child.write()
        return out

    @moosetree.Node.name.setter
    def name(self, value):
        """TODO: Get rid of the need to rename Nodes...don't know how yet"""
        self._Node__name = value

    @moosetree.Node.children.setter
    def children(self, value):
        """TODO: Get rid the need to set children directly...see navigation.py"""
        for child in self._Node__children:
            child.parent = None
        for child in value:
            child.parent = self
