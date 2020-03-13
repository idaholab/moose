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
from MooseDocs.tree import base

class TestNodeBase(unittest.TestCase):
    """
    Tests for NodeBase class.
    """

    def testRoot(self):
        node = base.NodeBase('root', None)
        self.assertEqual(node.parent, None)

    def testTree(self):
        root = base.NodeBase('root', None)
        node = base.NodeBase('child', root)
        self.assertIs(node.parent, root)
        self.assertIs(root.children[0], node)
        self.assertIs(root(0), node)

    def testCallError(self):
        with self.assertRaises(IndexError) as ex:
            node = base.NodeBase('root', None)
            node(0)
        self.assertIn('list index out of range', str(ex.exception))

    def testWrite(self):
        node = base.NodeBase('root', None)
        class TestNode(base.NodeBase):
            def write(self):
                return 'foo'
        TestNode('test0', node)
        TestNode('test1', node)
        self.assertEqual(node.write(), 'foofoo')

    def testIter(self):
        root = base.NodeBase('root', None)
        child0 = base.NodeBase('0', root)
        child1 = base.NodeBase('1', root)
        self.assertEqual(list(root), [child0, child1])

    def testName(self):
        node = base.NodeBase('root', None)
        self.assertEqual(node.name, 'root')

if __name__ == '__main__':
    unittest.main(verbosity=2)
