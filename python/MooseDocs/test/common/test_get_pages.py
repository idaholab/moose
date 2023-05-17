#!/usr/bin/env python3
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
import mock

from MooseDocs import ROOT_DIR
from MooseDocs.common.get_content import _build_regex, _find_files, _doc_import, get_content
from MooseDocs.tree import pages

class TestBuildRegex(unittest.TestCase):
    def testBasic(self):
        path = '/one/**/four/five'
        self.assertEqual(_build_regex(path), r'^/one/(?:.*?)/four/five$')

        path = '/one/two/three/four/**'
        self.assertEqual(_build_regex(path), r'^/one/two/three/four/(?:.*?)$')

        path = '**/three/four/five'
        self.assertEqual(_build_regex(path), r'^(?:.*?)/three/four/five$')

        path = '**/three/**'
        self.assertEqual(_build_regex(path), r'^(?:.*?)/three/(?:.*?)$')

        path = '**/three/**/nine/**'
        self.assertEqual(_build_regex(path), r'^(?:.*?)/three/(?:.*?)/nine/(?:.*?)$')

        path = '/one/*/four/five'
        self.assertEqual(_build_regex(path), r'^/one/(?:[^/]*?)/four/five$')

        path = '/one/two/three/four/*'
        self.assertEqual(_build_regex(path), r'^/one/two/three/four/(?:[^/]*?)$')

        path = '*/three/four/five'
        self.assertEqual(_build_regex(path), r'^(?:[^/]*?)/three/four/five$')

        path = '*/three/*'
        self.assertEqual(_build_regex(path), r'^(?:[^/]*?)/three/(?:[^/]*?)$')

        path = '*/three/*/nine/*'
        self.assertEqual(_build_regex(path), r'^(?:[^/]*?)/three/(?:[^/]*?)/nine/(?:[^/]*?)$')

        path = '*/three/**/nine/*'
        self.assertEqual(_build_regex(path), r'^(?:[^/]*?)/three/(?:.*?)/nine/(?:[^/]*?)$')

        path = '**/three/*/nine/**'
        self.assertEqual(_build_regex(path), r'^(?:.*?)/three/(?:[^/]*?)/nine/(?:.*?)$')

        path = '**/three/*/nine/**/foo.md'
        self.assertEqual(_build_regex(path), r'^(?:.*?)/three/(?:[^/]*?)/nine/(?:.*?)/foo\.md$')

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
        files = _find_files(filenames, pattern)
        self.assertEqual(len(files), 4)
        self.assertEqual(files, set(filenames[:4]))

        pattern = '/one/two/three/four/**'
        files = _find_files(filenames, pattern)
        self.assertEqual(len(files), 8)
        self.assertEqual(files, set(filenames[:4] + filenames[6:]))

        pattern = '/one/two/*/four/*'
        files = _find_files(filenames, pattern)
        self.assertEqual(len(files), 6)
        self.assertEqual(files, set(filenames[:6]))

        pattern = '/one/**/four/*'
        files = _find_files(filenames, pattern)
        self.assertEqual(len(files), 6)
        self.assertEqual(files, set(filenames[:6]))

        pattern = '**/four/*'
        files = _find_files(filenames, pattern)
        self.assertEqual(len(files), 6)
        self.assertEqual(files, set(filenames[:6]))

        pattern = '**/five/*'
        files = _find_files(filenames, pattern)
        self.assertEqual(len(files), 4)
        self.assertEqual(files, set(filenames[6:]))

class TestDocImport(unittest.TestCase):
    def testBasic(self):
        items = _doc_import(content=['framework/doc/content/**',
                                    '~framework/doc/content/documentation/**'],
                           root_dir=ROOT_DIR)
        self.assertIsInstance(items, list)

        gold = '{}/framework/doc/content/documentation/systems/Kernels/framework/Diffusion.md'
        self.assertNotIn(gold.format(ROOT_DIR), items)


    def testFilename(self):
        items = _doc_import(content=['framework/doc/content/getting_started/*',
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

        _doc_import(os.getcwd(), content=dict())
        args, _ = mock.call_args
        self.assertIn('The "content" must be a list of str items.', args[-1])

        _doc_import(os.getcwd(), content=[1])
        args, _ = mock.call_args
        self.assertIn('The "content" must be a list of str items.', args[-1])

        _doc_import('not/valid', content=['foo'])
        args, _ = mock.call_args
        self.assertIn('The "root_dir" must be a valid directory', args[-2])

class TestDocTree(unittest.TestCase):
    def testBasic(self):
        from_dir = 'framework/doc/content'

        # If you come across this in the future with this failing... you need to
        # specify a few directories in from_dir/* and
        # list all of the files in said folder(s)
        framework_dirs = {'framework': ['contributing.md', 'documenting.md', 'patch_to_code.md', 'reviewing.md'],
                          'finite_volumes': ['fv_design.md', 'index.md']}

        items = []
        for dir in framework_dirs:
            items.append(dict(root_dir=os.path.join(ROOT_DIR, from_dir),
                         content=['{}/**'.format(dir)], key=dir))

        nodes = get_content(items, '.md')

        num_nodes = len(framework_dirs)
        for files in framework_dirs.values():
            num_nodes = num_nodes + len(files)
        self.assertEqual(num_nodes, len(nodes))

        for p in nodes:
            self.assertIn(p.key, framework_dirs)
            if p.local in framework_dirs:
                self.assertIsInstance(p, pages.Directory)
            else:
                if p.local.endswith('.md'):
                    self.assertIsInstance(p, pages.Source)
                else:
                    self.assertIsInstance(p, pages.File)
            self.assertEqual(p.source,
                             os.path.join(ROOT_DIR, from_dir, p.local))

if __name__ == '__main__':
    import logging
    logging.basicConfig()
    unittest.main(verbosity=2)
