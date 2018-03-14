#!/usr/bin/env python
import os
import unittest

import mooseutils

import MooseDocs
from MooseDocs.tree import app_syntax

class TestSyntaxTree(unittest.TestCase):
    def testRemoveDisable(self):
        location = os.path.join(MooseDocs.MOOSE_DIR, 'modules', 'combined')
        exe = mooseutils.find_moose_executable(location)
        root = app_syntax(exe, remove=[])
        node = root.findfull('/Variables/InitialCondition/BoundingBoxIC')
        self.assertEqual(node.name, 'BoundingBoxIC')

    def testRemove(self):
        location = os.path.join(MooseDocs.MOOSE_DIR, 'modules', 'combined')
        exe = mooseutils.find_moose_executable(location)
        root = app_syntax(exe, remove=['/Variables/InitialCondition'])

        node = root.findfull('/Variables/InitialCondition/AddICAction')
        self.assertEqual(node.name, 'AddICAction')
        self.assertTrue(node.removed)

        node = root.findfull('/Variables/InitialCondition/BoundingBoxIC')
        self.assertTrue(node.removed)

    def testRemoveTestApp(self):
        location = os.path.join(MooseDocs.MOOSE_DIR, 'modules', 'combined')
        exe = mooseutils.find_moose_executable(location)
        root = app_syntax(exe)
        node = root.findfull('/UserObjects/TestDistributionPostprocessor')
        self.assertTrue(node.removed)
        self.assertIn('MiscTest', root.groups)


if __name__ == '__main__':
    unittest.main(verbosity=2)
