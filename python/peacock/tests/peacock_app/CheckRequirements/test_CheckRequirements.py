#!/usr/bin/env python
import unittest
import mock
from peacock import CheckRequirements

try:
    import builtins
except ImportError:
    import __builtin__ as builtins

realimport = builtins.__import__

bad_import_name = ""
def myimport(name, globals={}, locals={}, fromlist=[], level=-1):
    if name == bad_import_name:
        raise ImportError
    else:
        return realimport(name, globals, locals, fromlist, level)

class Tests(unittest.TestCase):
    def setUp(self):
        builtins.__import__ = realimport

    def blockImport(self, name):
        global bad_import_name
        bad_import_name = name
        builtins.__import__ = myimport

    def testBadVTK(self):
        self.blockImport("vtk")
        self.assertFalse(CheckRequirements.check_vtk())

    @mock.patch('peacock.CheckRequirements.ErrorObserver')
    def testBadOpenGL(self, mock_err):
        self.assertFalse(CheckRequirements.check_vtk())

    def testGoodVTK(self):
        self.assertTrue(CheckRequirements.check_vtk())

    def testGoodQt(self):
        self.assertTrue(CheckRequirements.check_qt())

    def testBadQt(self):
        self.blockImport("PyQt5")
        self.assertFalse(CheckRequirements.check_qt())

    def testGoodMatplotlib(self):
        self.assertTrue(CheckRequirements.check_matplotlib())

    def testBadMatplotlib(self):
        self.blockImport("matplotlib")
        self.assertFalse(CheckRequirements.check_matplotlib())

    def testBadRequirements(self):
        self.blockImport("matplotlib")
        self.assertFalse(CheckRequirements.has_requirements())

    def testGoodRequirements(self):
        self.assertTrue(CheckRequirements.has_requirements())

if __name__ == '__main__':
    unittest.main(verbosity=2)
