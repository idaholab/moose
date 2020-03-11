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
import os
import multiprocessing

from MooseDocs import ROOT_DIR
from MooseDocs.tree import pages

class TestPage(unittest.TestCase):
    """
    Tests for latex tree structure.
    """
    def testPage(self):
        one = pages.Page('one', source='foo')
        self.assertEqual(one.name, 'one')
        self.assertEqual(one.source, 'foo')

    def testDirectory(self):
        node = pages.Directory('name', source='foo')
        self.assertEqual(node.source, 'foo')
        self.assertEqual(node.COLOR, 'CYAN')

    def testFile(self):
        source = os.path.join(ROOT_DIR, 'docs', 'content', 'utilities', 'MooseDocs', 'index.md')
        node = pages.File('foo', source=source)
        self.assertEqual(node.source, source)





if __name__ == '__main__':
    unittest.main(verbosity=2)
