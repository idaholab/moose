#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import unittest
from MooseDocs.common import build_class_database

class TestClassDatabase(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        """
        Create class database.
        """
        cls.database = build_class_database(['${MOOSE_DIR}/framework/src',
                                             '${MOOSE_DIR}/test/src',
                                             '${MOOSE_DIR}/modules/heat_conduction/src'],
                                            ['${MOOSE_DIR}/framework/include',
                                             '${MOOSE_DIR}/test/include',
                                             '${MOOSE_DIR}/modules/heat_conduction/include'],
                                            ['${MOOSE_DIR}/test/tests'])

    def testBasic(self):
        """
        Test that class with h and C files are located.
        """
        info = self.database['BoxMarker']
        self.assertEqual(info.header, 'framework/include/markers/BoxMarker.h')
        self.assertEqual(info.source, 'framework/src/markers/BoxMarker.C')
        self.assertIn('test/tests/markers/box_marker/box_marker_test.i', info.inputs)

    def testNamed(self):
        """
        Test that named objects with h and C files are located.
        """
        info = self.database['ParsedFunction']
        self.assertEqual(info.header, 'framework/include/functions/MooseParsedFunction.h')
        self.assertEqual(info.source, 'framework/src/functions/MooseParsedFunction.C')

class TestClassDatabaseEmptyInput(unittest.TestCase):
    def testDiffusion(self):
        database = build_class_database()
        info = database['Diffusion']
        self.assertEqual(info.header, 'framework/include/kernels/Diffusion.h')
        self.assertEqual(info.source, 'framework/src/kernels/Diffusion.C')
        self.assertIn('modules/heat_conduction/include/kernels/HeatConduction.h', info.children)
        self.assertIn('test/tests/mesh/named_entities/named_entities_test_xda.i', info.inputs)


if __name__ == '__main__':
    unittest.main(verbosity=2)
