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

import os
import unittest
import mooseutils
import MooseDocs
from MooseDocs.commands.MooseDocsNode import MooseDocsNode
from MooseDocs.commands.MarkdownNode import MarkdownNode
from MooseDocs.commands.build import make_tree, flat

class TestMooseDocsNode(unittest.TestCase):
    """
    Tests the markdown conversion node objects.
    """
    def testNameOnly(self):
        node = MooseDocsNode(name='only', site_dir=os.getcwd())
        self.assertEqual(node.name(), 'only')

        # "virtual" methods that must return something for iteration over nodes
        self.assertEqual(node.breadcrumbs(), [node])
        self.assertIsNone(node.build())
        self.assertIsNone(node.parent())
        self.assertIsNone(node.source())
        self.assertIsNone(node.url())

        self.assertIs(node.root(), node)

        # The path should be the supplied directory with the name() as a folder
        self.assertEqual(node.path(), os.path.join(os.getcwd(), 'only'))
        self.assertEqual(node.relpath('../misc'), '../../misc')

        self.assertEqual(node.relpath('http:://foo.com'), 'http:://foo.com')

    def testSiteDir(self):
        node = MooseDocsNode(name='only', site_dir='site')
        self.assertEqual(node.name(), 'only')

        # The path should be the supplied directory with the name() as a folder
        self.assertEqual(node.path(), os.path.relpath(os.path.join(os.getcwd(), 'site', 'only')))
        self.assertEqual(node.relpath('foo/misc'), '../foo/misc')

    def testTree(self):
        """
        node0 -- node1 -- node3
              |
              |- node2 -- node4
                       |
                       |- node5
        """

        # Creates some arbitrary tree structure
        node0 = MooseDocsNode(name='', site_dir='site')
        node1 = MooseDocsNode(name='node1', site_dir='site', parent=node0)
        node2 = MooseDocsNode(name='node2', site_dir='site', parent=node0)
        node3 = MooseDocsNode(name='node3', site_dir='site', parent=node1)
        node4 = MooseDocsNode(name='node4', site_dir='site', parent=node2)
        node5 = MooseDocsNode(name='node5', site_dir='site', parent=node2)
        nodes = [node0, node1, node2, node3, node4, node5]

        # path()
        self.assertEqual(node0.path(), 'site')
        self.assertEqual(node1.path(), 'site/node1')
        self.assertEqual(node2.path(), 'site/node2')
        self.assertEqual(node3.path(), 'site/node1/node3')
        self.assertEqual(node4.path(), 'site/node2/node4')
        self.assertEqual(node5.path(), 'site/node2/node5')

        # root() and url()
        for node in nodes:
            self.assertIs(node.root(), node0)
            self.assertIsNone(node.url())

        # __iter__()
        self.assertEqual(list(node0), [node1, node2])
        self.assertEqual(list(node1), [node3])
        self.assertEqual(list(node2), [node4, node5])
        for node in [node3, node4, node5]:
            self.assertEqual(list(node), [])

        # relpath()
        # For these tests how to get from the calling node to the supplied path.
        self.assertEqual(node0.relpath('node2'), 'node2')
        self.assertEqual(node1.relpath('node2'), '../node2')
        self.assertEqual(node2.relpath('node2'), '.')
        self.assertEqual(node3.relpath('node2'), '../../node2')
        self.assertEqual(node4.relpath('node2'), '..')
        self.assertEqual(node5.relpath('node2'), '..')

        # parent()
        self.assertIs(node0.parent(), None)
        self.assertIs(node1.parent(), node0)
        self.assertIs(node2.parent(), node0)
        self.assertIs(node3.parent(), node1)
        self.assertIs(node4.parent(), node2)
        self.assertIs(node5.parent(), node2)


    def testExceptions(self):

        with self.assertRaises(mooseutils.MooseException) as e:
            MooseDocsNode()
        self.assertEqual(str(e.exception),
                         'The "name" string must be supplied to the MooseDocsNode object.')

        with self.assertRaises(mooseutils.MooseException) as e:
            MooseDocsNode(name=42)
        self.assertEqual(str(e.exception),
                         'The "name" string must be supplied to the MooseDocsNode object.')

        for site_dir in [None, 42, 'not/a/valid/directory']:
            with self.assertRaises(mooseutils.MooseException) as e:
                MooseDocsNode(name="foo", site_dir=site_dir)
            self.assertEqual(str(e.exception),
                             'The "site_dir" must be a string and a valid directory.')

class MarkdownNodeTest(unittest.TestCase):

    def setUp(self):
        # clear site directory
        for root, _, files in os.walk('site'):
            for filename in files:
                if filename.endswith('.html'):
                    os.remove(os.path.join(root, filename))

        # Markdown parser
        self._parser = MooseDocs.MooseMarkdown()

    def testSingle(self):
        node = MarkdownNode(name='single', site_dir='site', parser=self._parser,
                            markdown='content/index.md')

        # "virtual" methods that must return something for iteration over nodes
        self.assertEqual(node.breadcrumbs(), [node])
        self.assertIsNone(node.parent())
        self.assertEqual(node.source(), 'content/index.md')
        self.assertEqual(node.url(), 'site/single/index.html')
        self.assertIs(node.root(), node)
        self.assertEqual(node.path(), 'site/single')

        # build()
        node.build()
        self.assertTrue(os.path.exists(node.url()))

    def testBuildTree(self):
        """
        Tests the build.py build_tree function.

        node0 -- node1 -- node3
              |
              |- node2 -- node4
                       |
                       |- node5
                       |
                       |- node6(no markdown) -- node7
        """
        root = MarkdownNode(name='', site_dir='site', parser=self._parser,
                            markdown='content/index.md')
        make_tree('content', root, 'site', self._parser)

        node0 = root
        node1 = list(root)[0]
        node2 = list(root)[1]
        node3 = list(node1)[0]
        node4 = list(node2)[0]
        node5 = list(node2)[1]
        node6 = list(node2)[2]
        node7 = list(node6)[0]

        # MooseDoscNode
        self.assertIsInstance(node6, MooseDocsNode)

        # MarkdownNodes
        for node in [node0, node1, node2, node3, node4, node5, node7]:
            self.assertIsInstance(node, MarkdownNode)

        self.assertEqual(node0.url(), 'site/index.html')
        self.assertEqual(node1.url(), 'site/node1/index.html')
        self.assertEqual(node2.url(), 'site/node2/index.html')
        self.assertEqual(node3.url(), 'site/node1/node3/index.html')
        self.assertEqual(node4.url(), 'site/node2/node4/index.html')
        self.assertEqual(node5.url(), 'site/node2/node5/index.html')
        self.assertEqual(node6.url(), None)
        self.assertEqual(node7.url(), 'site/node2/node6/node7/index.html')

        for node in [root] + list(flat(root)):
            node.build()

        self.assertTrue(os.path.exists('site/index.html'))
        self.assertTrue(os.path.exists('site/node1/index.html'))
        self.assertTrue(os.path.exists('site/node2/index.html'))
        self.assertTrue(os.path.exists('site/node1/node3/index.html'))
        self.assertTrue(os.path.exists('site/node2/node4/index.html'))
        self.assertTrue(os.path.exists('site/node2/node5/index.html'))
        self.assertFalse(os.path.exists('site/node2/node6/index.html'))
        self.assertTrue(os.path.exists('site/node2/node6/node7/index.html'))

if __name__ == '__main__':
    unittest.main(verbosity=2)
