#!/usr/bin/env python2
import os
import unittest
import mock

from MooseDocs import ROOT_DIR
from MooseDocs.tree.build_page_tree import build_regex
from MooseDocs.tree.build_page_tree import find_files
from MooseDocs.tree.build_page_tree import doc_import
from MooseDocs.tree.build_page_tree import doc_tree
from MooseDocs.tree import page

class TestBuildRegex(unittest.TestCase):
    def testBasic(self):
        path = '/one/**/four/five'
        self.assertEqual(build_regex(path), r'^/one/(?:.*?)/four/five$')

        path = '/one/two/three/four/**'
        self.assertEqual(build_regex(path), r'^/one/two/three/four/(?:.*?)$')

        path = '**/three/four/five'
        self.assertEqual(build_regex(path), r'^(?:.*?)/three/four/five$')

        path = '**/three/**'
        self.assertEqual(build_regex(path), r'^(?:.*?)/three/(?:.*?)$')

        path = '**/three/**/nine/**'
        self.assertEqual(build_regex(path), r'^(?:.*?)/three/(?:.*?)/nine/(?:.*?)$')

        path = '/one/*/four/five'
        self.assertEqual(build_regex(path), r'^/one/(?:[^/]*?)/four/five$')

        path = '/one/two/three/four/*'
        self.assertEqual(build_regex(path), r'^/one/two/three/four/(?:[^/]*?)$')

        path = '*/three/four/five'
        self.assertEqual(build_regex(path), r'^(?:[^/]*?)/three/four/five$')

        path = '*/three/*'
        self.assertEqual(build_regex(path), r'^(?:[^/]*?)/three/(?:[^/]*?)$')

        path = '*/three/*/nine/*'
        self.assertEqual(build_regex(path), r'^(?:[^/]*?)/three/(?:[^/]*?)/nine/(?:[^/]*?)$')

        path = '*/three/**/nine/*'
        self.assertEqual(build_regex(path), r'^(?:[^/]*?)/three/(?:.*?)/nine/(?:[^/]*?)$')

        path = '**/three/*/nine/**'
        self.assertEqual(build_regex(path), r'^(?:.*?)/three/(?:[^/]*?)/nine/(?:.*?)$')

        path = '**/three/*/nine/**/foo.md'
        self.assertEqual(build_regex(path), r'^(?:.*?)/three/(?:[^/]*?)/nine/(?:.*?)/foo\.md$')

class TestFindFiles(unittest.TestCase):
    def testBasic(self):
        filenames = ['/one/two/three/four/a.md',
                     '/one/two/three/four/b.md',
                     '/one/two/three/four/c.md',
                     '/one/two/three/four/d.md',
                     '/one/two/not-three/four/a.md',
                     '/one/two/not-three/four/b.md',
                     '/one/two/three/four/five/a.md',
                     '/one/two/three/four/five/b.md',
                     '/one/two/three/four/five/c.md',
                     '/one/two/three/four/five/d.md']

        pattern = '/one/two/three/four/*'
        files = find_files(filenames, pattern)
        self.assertEqual(len(files), 4)
        self.assertEqual(files, set(filenames[:4]))

        pattern = '/one/two/three/four/**'
        files = find_files(filenames, pattern)
        self.assertEqual(len(files), 8)
        self.assertEqual(files, set(filenames[:4] + filenames[6:]))

        pattern = '/one/two/*/four/*'
        files = find_files(filenames, pattern)
        self.assertEqual(len(files), 6)
        self.assertEqual(files, set(filenames[:6]))

        pattern = '/one/**/four/*'
        files = find_files(filenames, pattern)
        self.assertEqual(len(files), 6)
        self.assertEqual(files, set(filenames[:6]))

        pattern = '**/four/*'
        files = find_files(filenames, pattern)
        self.assertEqual(len(files), 6)
        self.assertEqual(files, set(filenames[:6]))

        pattern = '**/five/*'
        files = find_files(filenames, pattern)
        self.assertEqual(len(files), 4)
        self.assertEqual(files, set(filenames[6:]))

class TestDocImport(unittest.TestCase):
    def testBasic(self):
        items = doc_import(content=['framework/doc/content/**',
                                    '~framework/doc/content/documentation/**'],
                           root_dir=ROOT_DIR)
        self.assertIsInstance(items, list)

        gold = '{}/framework/doc/content/documentation/systems/Kernels/framework/Diffusion.md'
        self.assertNotIn(gold.format(ROOT_DIR), items)


    def testFilename(self):
        items = doc_import(content=['framework/doc/content/getting_started/*',
                                    '~framework/doc/content/utilities/memory_logger/*',
                                    '~framework/doc/**/MooseDocs/*'],
                            root_dir=ROOT_DIR)

        self.assertIsInstance(items, list)
        gold = '{}/framework/doc/content/utilities/MooseDocs/index.md'
        self.assertNotIn(gold.format(ROOT_DIR), items)

        gold = '{}/framework/doc/content/documentation/systems/Kernels/framework/Diffusion.md'
        self.assertNotIn(gold.format(ROOT_DIR), items)

        gold = '{}/framework/doc/content/utilities/memory_logger/memory_logger.md'
        self.assertNotIn(gold.format(ROOT_DIR), items)

        gold = '{}/framework/doc/content/utilities/moose_docs/moose_markdown/index.md'
        self.assertNotIn(gold.format(ROOT_DIR), items)

    @mock.patch('logging.Logger.error')
    def testErrors(self, mock):

        doc_import(os.getcwd(), content=dict())
        args, _ = mock.call_args
        self.assertIn('The "content" must be a list of str items.', args[-1])

        doc_import(os.getcwd(), content=[1])
        args, _ = mock.call_args
        self.assertIn('The "content" must be a list of str items.', args[-1])

        doc_import('not/valid', content=['foo'])
        args, _ = mock.call_args
        self.assertIn('The "root_dir" must be a valid directory', args[-2])

class TestDocTree(unittest.TestCase):
    def testBasic(self):

        items = [dict(root_dir=os.path.join(ROOT_DIR, 'framework/doc/content'),
                      content=['getting_started/**']),
                 dict(root_dir=os.path.join(ROOT_DIR, 'framework/doc/content'),
                      content=['documentation/systems/Adaptivity/framework/**'])]

        root = doc_tree(items)

        self.assertIsInstance(root(0), page.DirectoryNode)
        self.assertEqual(root(0).name, 'getting_started')
        self.assertEqual(root(0).source,
                         os.path.join(ROOT_DIR, 'framework/doc/content/getting_started'))

        self.assertIsInstance(root(0)(0), page.DirectoryNode)
        self.assertEqual(root(0)(0).name, 'installation')
        self.assertEqual(root(0)(0).source,
                         os.path.join(ROOT_DIR, 'framework/doc/content/getting_started/installation'))

        self.assertIsInstance(root(0)(0)(0), page.MarkdownNode)
        self.assertEqual(root(0)(0)(0).name, 'bash_profile.md')
        self.assertEqual(root(0)(0)(0).source,
                         os.path.join(ROOT_DIR,
                                      'framework/doc/content/getting_started/installation/bash_profile.md'))

if __name__ == '__main__':
    import logging
    logging.basicConfig()
    unittest.main(verbosity=2)
