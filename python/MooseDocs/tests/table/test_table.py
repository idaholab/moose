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
