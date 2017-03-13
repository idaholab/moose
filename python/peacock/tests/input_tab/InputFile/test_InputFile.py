#!/usr/bin/env python
from peacock.Input.InputFile import InputFile
from peacock.utils import Testing
from peacock import PeacockException

class Tests(Testing.PeacockTester):
    def setUp(self):
        super(Tests, self).setUp()
        self.tmp_file = "tmp_input.i"
        self.basic_input = "[foo]\n[./bar]\ntype = bar\nother = 'bar'[../]\n[./foobar]\nactive = 'bar'\n[../]\n[]\n"

    def tearDown(self):
        Testing.remove_file(self.tmp_file)

    def writeFile(self, s):
        with open(self.tmp_file, "w") as f:
            f.write(s)

    def createBasic(self):
        self.writeFile(self.basic_input)
        input_file = InputFile(self.tmp_file)
        self.assertNotEqual(input_file.root_node, None)
        self.assertEqual(input_file.root_node.children.keys(), ["foo"])
        return input_file

    def testOpenInputFile(self):
        input_file = InputFile()
        with self.assertRaises(PeacockException.PeacockException):
            input_file.openInputFile("/no_exist")
        with self.assertRaises(PeacockException.PeacockException):
            input_file.openInputFile("/")
        with self.assertRaises(Exception):
            # simulate a duplicate section in the input file
            # which should throw an exception
            self.writeFile(self.basic_input*2)
            input_file.openInputFile(self.tmp_file)

    def testIsActive(self):
        input_file = self.createBasic()
        self.assertEqual(input_file.isActive(input_file.root_node), False)
        self.assertEqual(input_file.isActive(input_file.root_node.children["foo"]), True)

    def testHasParams(self):
        input_file = self.createBasic()
        foo = input_file.root_node.children["foo"]
        bar = foo.children["bar"]
        foobar = foo.children["foobar"]
        self.assertEqual(input_file.hasParams(foo), False)
        self.assertEqual(input_file.hasParams(bar), True)
        self.assertEqual(input_file.hasParams(foobar), False)

    def testGetType(self):
        input_file = self.createBasic()
        foo = input_file.root_node.children["foo"]
        bar = foo.children["bar"]
        self.assertEqual(input_file.getType(foo), None)
        self.assertEqual(input_file.getType(bar), "bar")

        foo.name = "Mesh"
        self.assertEqual(input_file.getType(foo), "FileMesh")
        foo.name = "Problem"
        self.assertEqual(input_file.getType(foo), "FEProblem")

if __name__ == '__main__':
    Testing.run_tests()
