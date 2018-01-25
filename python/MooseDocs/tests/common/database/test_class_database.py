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
from MooseDocs.common.MooseClassDatabase import MooseClassDatabase

class TestMooseClassDatabase(unittest.TestCase):
    """
    Tests for MooseClassDatabase object.
    """

    @classmethod
    def setUpClass(cls):
        """
        Create class database.
        """
        cls.database = MooseClassDatabase('http::/testing/')

    def testBasic(self):
        """
        Test that class with h and C files are located.
        """
        info = self.database['BoxMarker']
        self.assertEqual(len(info), 2)
        self.assertEqual(info[0].remote, 'http::/testing/framework/include/markers/BoxMarker.h')
        self.assertEqual(info[0].filename, '/framework/include/markers/BoxMarker.h')

        self.assertEqual(info[1].remote, 'http::/testing/framework/src/markers/BoxMarker.C')
        self.assertEqual(info[1].filename, '/framework/src/markers/BoxMarker.C')

    def testNamed(self):
        """
        Test that named objects with h and C files are located.
        """
        info = self.database['MooseParsedFunction']
        self.assertEqual(len(info), 2)
        self.assertEqual(info[0].remote,
                         'http::/testing/framework/include/functions/MooseParsedFunction.h')
        self.assertEqual(info[0].filename, '/framework/include/functions/MooseParsedFunction.h')

        self.assertEqual(info[1].remote,
                         'http::/testing/framework/src/functions/MooseParsedFunction.C')
        self.assertEqual(info[1].filename, '/framework/src/functions/MooseParsedFunction.C')

    def testHeaderOnly(self):
        """
        Test that named objects with h and C files are located.
        """
        info = self.database['MooseObjectWarehouse']
        self.assertEqual(len(info), 1)
        self.assertEqual(info[0].remote,
                         'http::/testing/framework/include/base/MooseObjectWarehouse.h')
        self.assertEqual(info[0].filename, '/framework/include/base/MooseObjectWarehouse.h')



if __name__ == '__main__':
    unittest.main(verbosity=2)
