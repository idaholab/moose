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
import anytree
import MooseDocs
from MooseDocs.testing import LogTestCase
from MooseDocs import common
from MooseDocs.MooseMarkdown import MooseMarkdown

class TestFileTree(LogTestCase):
    """
    Tests for MooseDocsFileTree object.
    """
    @staticmethod
    def finder(root, name):
        """Helper for finding tree items"""
        filter_ = lambda n: n.full_name.endswith(name)
        return [node for node in anytree.iterators.PreOrderIter(root, filter_=filter_)]

    def testBasic(self):
        config = dict()
        config['framework'] = dict(base='docs/content',
                                   include=['docs/content/**/Functions/index.md',
                                            'docs/content/**/Functions/framework/**'])
        config['moose_test'] = dict(base='test/docs/content',
                                    include=['test/docs/content/**/Functions/**'])
        root = common.moose_docs_file_tree(config)

        nodes = self.finder(root, 'moose_test/PostprocessorFunction')
        self.assertEqual(len(nodes), 1)
        gold = os.path.join(MooseDocs.ROOT_DIR,
                            "test/docs/content/documentation/systems/Functions/moose_test/"
                            "PostprocessorFunction.md")
        self.assertEqual(nodes[0].filename, gold)

        nodes = self.finder(root, 'framework/ConstantFunction')
        self.assertEqual(len(nodes), 1)
        gold = os.path.join(MooseDocs.ROOT_DIR,
                            "docs/content/documentation/systems/Functions/framework/"
                            "ConstantFunction.md")
        self.assertEqual(nodes[0].filename, gold)

    def testTree(self):
        config = dict()
        config['framework'] = dict(base='docs/content',
                                   include=['docs/content/documentation/systems/Adaptivity/**'])
        root = common.moose_docs_file_tree(config)

        # MarkdownIndexNode
        nodes = self.finder(root, 'systems/Adaptivity')
        self.assertEqual(len(nodes), 1)
        self.assertIsInstance(nodes[0], common.nodes.MarkdownFileIndexNode)
        gold = os.path.join(MooseDocs.ROOT_DIR,
                            "docs/content/documentation/systems/Adaptivity/index.md")
        self.assertEqual(nodes[0].filename, gold)
        gold = "documentation/systems/Adaptivity/index.html"
        self.assertEqual(nodes[0].destination, gold)

        # DirectoryNode
        nodes = self.finder(root, 'systems/Adaptivity/Markers/framework')
        self.assertEqual(len(nodes), 1)
        self.assertIsInstance(nodes[0], common.nodes.DirectoryNode)

        # MarkdownPageNode
        nodes = self.finder(root, 'UniformMarker')
        self.assertEqual(len(nodes), 1)
        self.assertIsInstance(nodes[0], common.nodes.MarkdownFilePageNode)
        gold = "docs/content/documentation/systems/Adaptivity/Markers/framework/UniformMarker.md"
        gold = os.path.join(MooseDocs.ROOT_DIR, gold)
        self.assertEqual(nodes[0].filename, gold)
        gold = "documentation/systems/Adaptivity/Markers/framework/UniformMarker/index.html"
        self.assertEqual(nodes[0].destination, gold)

    def testNodeFinder(self):
        config = dict()
        config['framework'] = dict(base='docs/content',
                                   include=['docs/content/documentation/systems/**'])
        root = common.moose_docs_file_tree(config)
        node0 = MooseMarkdown.find(root, 'systems/index.md')[0]
        node1 = MooseMarkdown.find(root, 'systems/Adaptivity/index.md')[0]
        path = os.path.relpath(node1.destination, os.path.dirname(node0.destination))
        self.assertEqual(path, 'Adaptivity/index.html')

    def testIndex(self):
        config = dict()
        config['framework'] = dict(base='docs/content',
                                   include=['docs/content/index.md'])
        root = common.moose_docs_file_tree(config)
        self.assertEqual(len(root.children), 1)
        self.assertIsInstance(root.children[0], common.nodes.MarkdownFileIndexNode)
        self.assertEqual(root.children[0].filename,
                         os.path.join(MooseDocs.ROOT_DIR, "docs/content/index.md"))

    def testImages(self):
        config = dict()
        config['framework'] = dict(base='docs/content', include=['docs/content/**'])
        root = common.moose_docs_file_tree(config)

        node = self.finder(root, 'media/gitlab-logo.png')[0]
        self.assertEqual(node.filename, os.path.join(MooseDocs.ROOT_DIR, 'docs', 'content', 'media',
                                                     'gitlab-logo.png'))
        self.assertEqual(node.destination, os.path.join('media', 'gitlab-logo.png'))

    def testContentFile(self):
        config = MooseDocs.yaml_load(os.path.join(MooseDocs.ROOT_DIR, 'docs', 'content.yml'))
        root = common.moose_docs_file_tree(config)

        node = self.finder(root, 'media/gitlab-logo.png')[0]
        self.assertEqual(node.filename, os.path.join(MooseDocs.ROOT_DIR, 'docs', 'content', 'media',
                                                     'gitlab-logo.png'))

        node0 = MooseMarkdown.find(root, 'utilities/moose_docs/index.md')[0]
        self.assertIsNotNone(node0)

        node1 = MooseMarkdown.find(root, 'utilities/moose_docs/moose_markdown/index.md')
        self.assertIsNotNone(node1)

        node2 = MooseMarkdown.find(root,
                                   'utilities/moose_docs/moose_markdown/extensions/include.md')
        self.assertIsNotNone(node2)

    def testRootEnv(self):
        config = dict()
        os.environ['MOOSE_DIR'] = MooseDocs.MOOSE_DIR
        config['framework'] = dict(root_dir='$MOOSE_DIR', base='docs/content',
                                   include=['docs/content/**'])
        root = common.moose_docs_file_tree(config)
        node = self.finder(root, 'media/gitlab-logo.png')[0]
        self.assertEqual(node.filename, os.path.join(MooseDocs.ROOT_DIR, 'docs', 'content', 'media',
                                                     'gitlab-logo.png'))

    def testExclude(self):
        """
        Test that apps can include/exclude framework content.
        """
        os.environ['MOOSE_DIR'] = MooseDocs.MOOSE_DIR
        config = dict()
        config['test'] = dict(root_dir='$MOOSE_DIR',
                              base='test/docs/content',
                              include=['test/docs/content/**'])
        config['framework'] = dict(root_dir='$MOOSE_DIR',
                                   base='docs/content',
                                   exclude=['docs/content/**/level_set/**',
                                            'docs/content/**/phase_field/**',
                                            'docs/content/**/navier_stokes/**'
                                           ],
                                   include=['docs/content/**'])
        exclude = common.moose_docs_file_tree(config)

        config['framework']['exclude'] = []
        include = common.moose_docs_file_tree(config)

        name = '/documentation/systems/Kernels/framework/Diffusion'
        node = self.finder(exclude, name)
        self.assertEqual(len(node), 1)
        node = self.finder(include, name)
        self.assertEqual(len(node), 1)

        name = '/documentation/systems/Kernels/level_set/LevelSetAdvection'
        node = self.finder(include, name)
        self.assertEqual(len(node), 1)
        node = self.finder(exclude, name)
        self.assertEqual(len(node), 0)

        name = '/documentation/systems/Kernels/phase_field/CHMath'
        node = self.finder(include, name)
        self.assertEqual(len(node), 1)
        node = self.finder(exclude, name)
        self.assertEqual(len(node), 0)

        name = '/documentation/systems/Kernels/navier_stokes/INSMass'
        node = self.finder(include, name)
        self.assertEqual(len(node), 1)
        node = self.finder(exclude, name)
        self.assertEqual(len(node), 0)


if __name__ == '__main__':
    unittest.main(verbosity=2)
