#!/usr/bin/env python2
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
import anytree
import logging
import mooseutils
import MooseDocs
from MooseDocs.tree import app_syntax

logging.basicConfig()
class TestSyntaxTree(unittest.TestCase):

    def testRemoveDisable(self):
        location = os.path.join(MooseDocs.MOOSE_DIR, 'modules', 'combined')
        exe = mooseutils.find_moose_executable(location)
        root = app_syntax(exe, remove=[])
        node = anytree.search.find_by_attr(root, '/Variables/InitialCondition/BoundingBoxIC',
                                           name='fullpath')
        self.assertEqual(node.name, 'BoundingBoxIC')

    def testRemove(self):
        location = os.path.join(MooseDocs.MOOSE_DIR, 'modules', 'combined')
        exe = mooseutils.find_moose_executable(location)
        root = app_syntax(exe, remove=['/Variables/InitialCondition'])

        node = anytree.search.find_by_attr(root, '/Variables/InitialCondition/AddICAction',
                                           name='fullpath')
        self.assertEqual(node.name, 'AddICAction')
        self.assertTrue(node.removed)

        node = anytree.search.find_by_attr(root, '/Variables/InitialCondition/BoundingBoxIC',
                                           name='fullpath')
        self.assertTrue(node.removed)

    def testRemoveTestApp(self):
        location = os.path.join(MooseDocs.MOOSE_DIR, 'modules', 'combined')
        exe = mooseutils.find_moose_executable(location)
        root = app_syntax(exe)
        node = anytree.search.find_by_attr(root, '/UserObjects/TestDistributionPostprocessor',
                                           name='fullpath')
        self.assertTrue(node.removed)
        self.assertIn('MiscTestApp', root.groups)

    def testAlias(self):
        location = os.path.join(MooseDocs.MOOSE_DIR, 'test')
        exe = mooseutils.find_moose_executable(location)
        alias = dict()
        alias['/VectorPostprocessors/VolumeHistogram'] = '/VPP/VolumeHistogram'
        root = app_syntax(exe, alias=alias)
        node = anytree.search.find_by_attr(root, '/VectorPostprocessors/VolumeHistogram',
                                           name='fullpath')
        self.assertEqual(node.fullpath, '/VectorPostprocessors/VolumeHistogram')
        self.assertEqual(node.alias, '/VPP/VolumeHistogram')

if __name__ == '__main__':
    unittest.main(verbosity=2)
