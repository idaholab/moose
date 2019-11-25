#!/usr/bin/env python
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import os
import unittest
import logging
import moosetree
import mooseutils
import MooseDocs
from MooseDocs.tree import app_syntax

logging.basicConfig()
class TestSyntaxTree(unittest.TestCase):

    def testRemoveDisable(self):
        location = os.path.join(MooseDocs.MOOSE_DIR, 'modules', 'combined')
        exe = mooseutils.find_moose_executable(location)
        root = app_syntax(exe, remove=[])
        node = moosetree.find(root, lambda n: n.fullpath == '/Variables/InitialCondition/BoundingBoxIC')
        self.assertEqual(node.name, 'BoundingBoxIC')

    def testRemove(self):
        location = os.path.join(MooseDocs.MOOSE_DIR, 'modules', 'combined')
        exe = mooseutils.find_moose_executable(location)
        root = app_syntax(exe, remove=['/Variables/InitialCondition'])

        node = moosetree.find(root, lambda n: n.fullpath == '/Variables/InitialCondition/AddICAction')
        self.assertEqual(node.name, 'AddICAction')
        self.assertTrue(node.removed)

        node = moosetree.find(root, lambda n: n.fullpath == '/Variables/InitialCondition/BoundingBoxIC')
        self.assertTrue(node.removed)

    def testRemoveTestApp(self):
        location = os.path.join(MooseDocs.MOOSE_DIR, 'modules', 'combined')
        exe = mooseutils.find_moose_executable(location)
        root = app_syntax(exe)
        node = moosetree.find(root, lambda n: n.fullpath == '/UserObjects/TestDistributionPostprocessor')
        self.assertTrue(node.removed)
        self.assertIn('MiscTestApp', root.groups)

    def testAlias(self):
        location = os.path.join(MooseDocs.MOOSE_DIR, 'test')
        exe = mooseutils.find_moose_executable(location)
        alias = dict()
        alias['/VectorPostprocessors/VolumeHistogram'] = '/VPP/VolumeHistogram'
        root = app_syntax(exe, alias=alias)
        node = moosetree.find(root, lambda n: n.fullpath == '/VectorPostprocessors/VolumeHistogram')

        self.assertEqual(node.fullpath, '/VectorPostprocessors/VolumeHistogram')
        self.assertEqual(node.alias, '/VPP/VolumeHistogram')

    def testADObject(self):
        location = os.path.join(MooseDocs.MOOSE_DIR, 'test')
        exe = mooseutils.find_moose_executable(location)
        root = app_syntax(exe)
        node = moosetree.find(root, lambda n: n.fullpath == '/Kernels/ADDiffusion')
        self.assertEqual(node.fullpath, '/Kernels/ADDiffusion')

if __name__ == '__main__':
    unittest.main(verbosity=2)
