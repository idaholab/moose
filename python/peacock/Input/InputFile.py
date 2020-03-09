#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import os
import mooseutils
from peacock.PeacockException import PeacockException
from pyhit import hit


class DupWalker(object):
    def __init__(self, fname):
        self.have = {}
        self.dups = {}
        self.errors = []
        self._fname = fname

    def _duperr(self, node):
        if node.type() == hit.NodeType.Section:
            ntype = 'section'
        elif node.type() == hit.NodeType.Field:
            ntype = 'parameter'
        self.errors.append('{}:{}: duplicate {} "{}"'.format(self._fname, node.line(), ntype, node.fullpath()))

    def walk(self, fullpath, path, node):
        if node.type() != hit.NodeType.Field and node.type() != hit.NodeType.Section:
            return

        if fullpath in self.have:
            if fullpath not in self.dups:
                self._duperr(self.have[fullpath])
                self.dups[fullpath] = True
            self._duperr(node)
        else:
            self.have[fullpath] = node


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

        self.root_node = None
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
        self.filename = os.path.abspath(filename)
        self.changed = False
        self.root_node = None

        # Do some basic checks on the filename to make sure
        # it is probably a real input file since the GetPot
        # parser doesn't do any checks.
        if not os.path.exists(filename):
            msg = "Input file %s does not exist" % filename
            raise PeacockException(msg)

        if not os.path.isfile(filename):
            msg = "Input file %s is not a file" % filename
            raise PeacockException(msg)

        if not filename.endswith(".i"):
            msg = "Input file %s does not have the proper extension" % filename
            raise PeacockException(msg)

        with open(filename, 'r') as f:
            data = f.read()
        self.readInputData(data, filename)

    def readInputData(self, data, filename):
        try:
            self.filename = os.path.abspath(filename)
            root = hit.parse(os.path.abspath(filename), data)
            hit.explode(root)
            w = DupWalker(os.path.abspath(filename))
            root.walk(w, hit.NodeType.Field)
            if w.errors:
                for err in w.errors:
                    mooseutils.mooseWarning(err)
                raise PeacockException("Parser errors")
            self.original_text = data
            self.root_node = root
            self.changed = False
        except PeacockException as e:
            msg = "Failed to parse input file %s:\n%s\n" % (filename, e)
            mooseutils.mooseWarning(msg)
            raise e

if __name__ == '__main__':
    import sys
    if len(sys.argv) < 2:
        print("Need an input file as argument")
        exit(1)
    filename = sys.argv[1]
    input_file = InputFile(filename)
    print(input_file.root_node.render())
    for root_node in input_file.root_node.children(node_type=hit.NodeType.Section):
        print(root_node.fullpath())
