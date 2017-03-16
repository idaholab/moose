#!/usr/bin/env python
from os import path
from FactorySystem.ParseGetPot import readInputFile, GPNode
import mooseutils
from peacock.PeacockException import PeacockException

class InputFile(object):
    """
    Holds the information of an input file.
    Can be empty.
    """
    def __init__(self, filename=None, **kwds):
        """
        Constructor.
        Input:
            filename: file name of the input file
        """
        super(InputFile, self).__init__(**kwds)

        self.root_node = GPNode("/", None)
        self.filename = None
        if filename:
            self.openInputFile(filename)
        self.original_text = ""
        self.changed = False

    def openInputFile(self, filename):
        """
        Opens a file and parses it.
        Input:
            filename: file name of the input file
        Signals:
            input_file_changed: On success
        Raises:
            PeacockException: On invalid input file
        """
        filename = str(filename)
        self.filename = path.abspath(filename)
        self.changed = False
        self.root_node = GPNode("/", None)

        if not path.exists(filename):
            msg = "Input file %s does not exist" % filename
            mooseutils.mooseError(msg)
            raise PeacockException(msg)
        if not path.isfile(filename):
            msg = "Input file %s is not a file" % filename
            mooseutils.mooseError(msg)
            raise PeacockException(msg)

        try:
            self.root_node = readInputFile(filename)
            with open(filename, "r") as f:
                self.original_text = f.read()
            self.changed = False
        except Exception as e:
            msg = "Failed to parse input file %s:\n%s\n" % (filename, e)
            mooseutils.mooseWarning(msg)
            raise e

    def getTopNodes(self):
        """
        Returns an iterable of top level nodes in the input file.
        If the input file hasn't been set then returns an empty list
        Return:
            list of paths of the root nodes. (/Mesh, /Executioner, etc)
        """
        return [ self.root_node.children[c] for c in self.root_node.children_list ]

    def isActive(self, node):
        """
        Whether this node is active, ie if it will actually be used in the input file.
        Input:
            node: ParseGetPot.GPNode()
        Return:
            bool: Whether it is active
        """
        if (node.parent and 'active' not in node.parent.params) or (node.parent and node.name in node.parent.params['active'].split(' ')):
            return True
        return False

    def hasParams(self, node):
        """
        Checks if the node has parameters.
        Input:
            node: ParseGetPot.GPNode()
        Return:
            bool: Whether the node has params
        """
        return bool(node.params) and "active" not in node.params

    def getType(self, node):
        """
        Get the type of the node.
        Input:
            node: ParseGetPot.GPNode()
        Return:
            Type (str) of the node or None if there is no type.
        """
        type_val = node.params.get("type", None)
        if not type_val and node.name == "Mesh":
            return "FileMesh"
        if not type_val and node.name == "Problem":
            return "FEProblem"
        return type_val

if __name__ == '__main__':
    import sys
    if len(sys.argv) < 2:
        print("Need an input file as argument")
        exit(1)
    filename = sys.argv[1]
    input_file = InputFile(filename)
    input_file.dump()
    #for node in input_file.getRootNodes():
    #    print("%s: %s : %s : %s" % (node.name, input_file.isActive(node), input_file.hasParams(node), input_file.getType(node)))
