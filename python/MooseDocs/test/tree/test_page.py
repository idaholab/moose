#!/usr/bin/env python2
import unittest
import os
import logging

import MooseDocs
from MooseDocs import ROOT_DIR
from MooseDocs.tree import page
from MooseDocs.common import exceptions

class TestPage(unittest.TestCase):
    """
    Tests for latex tree structure.
    """
    def testPageNodeBase(self):
        one = page.PageNodeBase(name='one', content=u'foo')
        two = page.PageNodeBase(one, name='two')
        page.PageNodeBase(two, name='three')

    def testLocationNodeBase(self):
        one = page.LocationNodeBase(source='one')
        two = page.LocationNodeBase(one, source='foo/two')
        three = page.LocationNodeBase(two, source='foo/bar/three')

        self.assertEqual(one.source, 'one')
        self.assertEqual(one.local, 'one')
        self.assertEqual(one.name, 'one')

        self.assertEqual(two.source, 'foo/two')
        self.assertEqual(two.local, 'one/two')
        self.assertEqual(two.name, 'two')

        self.assertEqual(three.source, 'foo/bar/three')
        self.assertEqual(three.local, 'one/two/three')
        self.assertEqual(three.name, 'three')

    def testDirectoryNode(self):
        node = page.DirectoryNode(source='foo')
        self.assertEqual(node.source, 'foo')
        self.assertEqual(node.COLOR, 'CYAN')

    def testFileNode(self):
        source = os.path.join(ROOT_DIR, 'docs', 'content', 'utilities', 'MooseDocs', 'index.md')
        node = page.FileNode(source=source)
        self.assertEqual(node.source, source)

class TestFindall(unittest.TestCase):
    """
    Tests for the LocationNodeBase.findall method.
    """
    def testBasic(self):
        root = page.DirectoryNode(None, source='docs')
        page.FileNode(root, source='docs/index.md')
        page.FileNode(root, source='docs/core.md')
        sub = page.DirectoryNode(root, source='docs/sub')
        page.FileNode(sub, source='docs/sub/core.md')
        page.FileNode(sub, source='docs/sub/full_core.md')

        nodes = root.findall('core.md', maxcount=None)
        self.assertEqual(len(nodes), 3)

        nodes = root.findall('/core.md', maxcount=None)
        self.assertEqual(len(nodes), 2)

        nodes = root.findall('docs/core.md', maxcount=None)
        self.assertEqual(len(nodes), 1)

    def testErrors(self):
        MooseDocs.LOG_LEVEL = logging.DEBUG
        root = page.DirectoryNode(source='docs')
        page.FileNode(root, source='docs/index.md')
        page.FileNode(root, source='docs/core.md')
        sub = page.DirectoryNode(root, source='docs/sub')
        page.FileNode(sub, source='docs/sub/core.md')

        with self.assertRaises(exceptions.MooseDocsException) as e:
            root.findall('foo', maxcount=2.2)
        self.assertIn("The argument 'maxcount' must be", str(e.exception))

        with self.assertRaises(exceptions.MooseDocsException) as e:
            root.findall('foo', mincount=2.2)
        self.assertIn("The argument 'mincount' must be", str(e.exception))

if __name__ == '__main__':
    unittest.main(verbosity=2)
