#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from peacock.Input.InputFile import InputFile
from peacock.utils import Testing
from peacock import PeacockException
from PyQt5 import QtWidgets
from pyhit import hit

class Tests(Testing.PeacockTester):
    qapp = QtWidgets.QApplication([])

    def setUp(self):
        super(Tests, self).setUp()
        self.tmp_file = "tmp_input.i"
        self.tmp_bad_file = "tmp_input"
        self.basic_input = "[foo]\n[./bar]\ntype = bar\nother = 'bar'[../]\n[./foobar]\nactive = 'bar'\n[../]\n[]\n"

    def tearDown(self):
        Testing.remove_file(self.tmp_file)
        Testing.remove_file(self.tmp_bad_file)

    def writeFile(self, s, fname):
        with open(fname, "w") as f:
            f.write(s)

    def testFailures(self):
        input_file = InputFile()
        with self.assertRaises(PeacockException.PeacockException):
            input_file.openInputFile("/no_exist")
        with self.assertRaises(PeacockException.PeacockException):
            input_file.openInputFile("/")
        with self.assertRaises(PeacockException.PeacockException):
            self.writeFile(self.basic_input, self.tmp_bad_file)
            input_file.openInputFile(self.tmp_bad_file)
        with self.assertRaises(PeacockException.PeacockException):
            # simulate a duplicate section in the input file
            # which should throw an exception
            self.writeFile(self.basic_input*2, self.tmp_file)
            input_file.openInputFile(self.tmp_file)

    def testSuccess(self):
        self.writeFile(self.basic_input, self.tmp_file)
        input_file = InputFile(self.tmp_file)
        self.assertNotEqual(input_file.root_node, None)
        self.assertEqual(input_file.changed, False)
        children = [ c for c in input_file.root_node.children(node_type=hit.NodeType.Section) ]
        self.assertEqual(len(children), 1)
        top = children[0]
        self.assertEqual(top.path(), "foo")
        children = [ c for c in top.children(node_type=hit.NodeType.Section) ]
        self.assertEqual(len(children), 2)
        c0 = children[0]
        self.assertEqual(c0.path(), "bar")
        params = [ c for c in c0.children(node_type=hit.NodeType.Field) ]
        self.assertEqual(len(params), 2)
        c1 = children[1]
        self.assertEqual(c1.path(), "foobar")
        params = [ c for c in c1.children(node_type=hit.NodeType.Field) ]
        self.assertEqual(len(params), 1)


if __name__ == '__main__':
    Testing.run_tests()
