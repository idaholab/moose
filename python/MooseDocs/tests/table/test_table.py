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

import unittest
from MooseDocs.testing import MarkdownTestCase

class TestMooseTable(MarkdownTestCase):
    EXTENSIONS = ['MooseDocs.extensions.tables', 'MooseDocs.extensions.refs']

    def testTable(self):
        md = '!table caption=This is an example table with a caption.\n' \
             '| 1 | 2 |\n' \
             '|---|---|\n' \
             '| 2 | 4 |\n\n'
        self.assertConvert('testTable.html', md)

    def testTableId(self):
        md = '!table id=table:foo caption=This is an example table with a caption.\n' \
             '| 1 | 2 |\n' \
             '|---|---|\n' \
             '| 2 | 4 |\n\n'
        self.assertConvert('testTableId.html', md)

    def testTableRef(self):
        md = '!table id=foo caption=This is an example table with a caption.\n' \
             '| 1 | 2 |\n' \
             '|---|---|\n' \
             '| 2 | 4 |\n\n' \
             '\\ref{foo}'
        self.assertConvert('testTableRef.html', md)

if __name__ == '__main__':
    unittest.main(verbosity=2)
