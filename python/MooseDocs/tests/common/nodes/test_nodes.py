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

import os
import unittest

import MooseDocs
from MooseDocs.common import nodes
from MooseDocs.testing import LogTestCase

class TestNodes(LogTestCase):
    """
    Tests the markdown conversion node objects.
    """
    def testNodeCore(self):
        root = nodes.NodeCore('')
        a = nodes.NodeCore('a', parent=root)
        b = nodes.NodeCore('b', parent=a)

        self.assertEqual(root.name, '')
        self.assertEqual(a.name, 'a')
        self.assertEqual(b.name, 'b')

        self.assertIsNone(root.parent)
        self.assertEqual(a.parent, root)
        self.assertEqual(b.parent, a)

        self.assertEqual(root.full_name, '')
        self.assertEqual(a.full_name, '/a')
        self.assertEqual(b.full_name, '/a/b')

        c = nodes.NodeCore('c', parent=b)
        self.assertEqual(c.findall(), [root, a, b, c])
        self.assertEqual(c.findall('/b'), [b])

        f = lambda n: 'a' in n.full_name
        self.assertEqual(c.findall(filter_=f), [a, b, c])

    def testMarkdownNode(self):
        node = nodes.MarkdownNode('foo', content='bar')
        self.assertEqual(node.name, 'foo')
        self.assertEqual(node.content, 'bar')

    def testDirectoryNode(self):
        node = nodes.DirectoryNode('foo')
        self.assertEqual(node.name, 'foo')

    def testMarkdownFileNodeBase(self):
        node = nodes.MarkdownFileNodeBase('foo', 'the/base/dir')
        self.assertEqual(node.basename, os.path.join(MooseDocs.ROOT_DIR, 'the/base/dir/foo'))
        self.assertEqual(node.destination, 'foo/index.html')

        with self.assertRaises(NotImplementedError) as e:
            node.filename #pylint: disable=pointless-statement
        self.assertEqual(str(e.exception), "The 'filename' property must be defined.")

        with self.assertRaises(NotImplementedError) as e:
            node.content #pylint: disable=pointless-statement
        self.assertEqual(str(e.exception), "The 'filename' property must be defined.")

        node2 = nodes.MarkdownFileNodeBase('bar', 'the/base/dir', parent=node)
        self.assertEqual(node2.destination, 'foo/bar/index.html')

    def testMarkdownFileIndexNode(self):
        node = nodes.MarkdownFileIndexNode('foo', 'the/base/dir')
        self.assertEqual(node.filename,
                         os.path.join(MooseDocs.ROOT_DIR, 'the/base/dir/foo/index.md'))

    def testMarkdownFilePageNode(self):
        node = nodes.MarkdownFilePageNode('foo', 'the/base/dir')
        self.assertEqual(node.filename,
                         os.path.join(MooseDocs.ROOT_DIR, 'the/base/dir/foo.md'))

    def testSyntaxNodeBase(self):
        node = nodes.SyntaxNodeBase('foo')
        self.assertFalse(node.hidden)
        node.hidden = True
        self.assertTrue(node.hidden)

        with self.assertRaises(TypeError) as e:
            node.hidden = 'foo' #pylint: disable=redefined-variable-type
        self.assertEqual(str(e.exception),
                         'The supplied value must be a boolean.')

        with self.assertRaises(NotImplementedError) as e:
            node.markdown('foo')
        self.assertEqual(str(e.exception),
                         "The 'markdown' method must return the expected markdown filename.")

        node.hidden = False
        with self.assertRaises(NotImplementedError) as e:
            node.check('foo')
        self.assertEqual(str(e.exception),
                         "The 'markdown' method must return the expected markdown filename.")

        self.assertEqual(node.groups, dict())
        self.assertEqual(node.syntax(), [])
        self.assertEqual(node.objects(), [])
        self.assertEqual(node.actions(), [])

    def testSyntaxNode(self):
        site = os.path.join('python', 'MooseDocs', 'tests', 'common', 'nodes', 'site')
        tmp = os.path.join(MooseDocs.ROOT_DIR, site, 'foo', 'index.md')
        if os.path.exists(tmp):
            os.remove(tmp)

        root = nodes.SyntaxNode('')
        node = nodes.SyntaxNode('foo', parent=root)
        self.assertEqual(node.markdown(site), tmp)

        self.assertEqual(root.syntax(), [node])
        self.assertEqual(root.objects(), [])
        self.assertEqual(root.actions(), [])

        # Un-documented, no file
        node.check(site)
        self.assertInLogError("No documentation for /foo, documentation")

        # Generated file
        node.check(site, generate=True)
        self.assertInLogInfo("Creating stub page for")
        self.assertTrue(os.path.exists(tmp))

        # Check content
        with open(tmp, 'r') as fid:
            content = fid.read()
        self.assertIn('!syntax objects /foo', content)
        self.assertIn('!syntax actions /foo', content)
        self.assertIn('!syntax subsystems /foo', content)

        # Un-documented, file exists
        node.check(site)
        self.assertInLogError("A MOOSE generated stub page")

    def testObjectNode(self):
        item = dict()
        item['description'] = 'description'
        item['parameters'] = {'param':1}
        item['file_info'] = {'some/path/FrogApp.C': 52}
        item['class'] = 'FrogFoo'

        site = os.path.join('python', 'MooseDocs', 'tests', 'common', 'nodes', 'site')
        tmp = os.path.join(MooseDocs.ROOT_DIR, site, 'frog', 'foo.md')
        if os.path.exists(tmp):
            os.remove(tmp)

        root = nodes.SyntaxNode('')
        node = nodes.MooseObjectNode('foo', item, parent=root)
        action = nodes.ActionNode('action', item, parent=root)

        self.assertEqual(root.syntax(), [])
        self.assertEqual(root.objects(), [node])
        self.assertEqual(root.actions(), [action])

        self.assertEqual(node.class_name, 'FrogFoo')
        self.assertEqual(node.description, 'description')
        self.assertEqual(node.parameters, {'param':1})
        self.assertEqual(node.markdown(site), tmp)
        self.assertEqual(node.groups, {'frog':'Frog'})

        node.check(site, generate=True)
        self.assertInLogInfo("Creating stub page for")
        self.assertTrue(os.path.exists(tmp))

        # Check content
        with open(tmp, 'r') as fid:
            content = fid.read()
        self.assertIn('!syntax description /foo', content)
        self.assertIn('!syntax parameters /foo', content)
        self.assertIn('!syntax inputs /foo', content)
        self.assertIn('!syntax children /foo', content)

if __name__ == '__main__':
    unittest.main(verbosity=2)
