#!/usr/bin/env python
#pylint: disable=missing-docstring
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
        cls.database = build_class_database('${MOOSE_DIR}/framework/include', '${MOOSE_DIR}/test/tests')

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
        info = self.database['MooseParsedFunction']
        self.assertEqual(info.header, 'framework/include/functions/MooseParsedFunction.h')
        self.assertEqual(info.source, 'framework/src/functions/MooseParsedFunction.C')

    def testHeaderOnly(self):
        """
        Test that named objects with h and C files are located.
        """
        info = self.database['MooseObjectWarehouse']
        self.assertEqual(info.header, 'framework/include/base/MooseObjectWarehouse.h')
        self.assertIsNone(info.source)

if __name__ == '__main__':
    unittest.main(verbosity=2)
