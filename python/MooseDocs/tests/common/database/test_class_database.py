#!/usr/bin/env python
#pylint: disable=missing-docstring
####################################################################################################
#                                    DO NOT MODIFY THIS HEADER                                     #
#                   MOOSE - Multiphysics Object Oriented Simulation Environment                    #
#                                                                                                  #
#                              (c) 2010 Battelle Energy Alliance, LLC                              #
#                                       ALL RIGHTS RESERVED                                        #
#                                                                                                  #
#                            Prepared by Battelle Energy Alliance, LLC                             #
#                               Under Contract No. DE-AC07-05ID14517                               #
#                               With the U. S. Department of Energy                                #
#                                                                                                  #
#                               See COPYRIGHT for full restrictions                                #
####################################################################################################
#pylint: enable=missing-docstring

import os
import unittest
import MooseDocs
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
