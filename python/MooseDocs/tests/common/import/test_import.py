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
from MooseDocs.testing import LogTestCase
from MooseDocs.common import moose_docs_import
from MooseDocs.common.moose_docs_import import build_regex
from MooseDocs.common.moose_docs_import import find_files

class TestBuildRegex(unittest.TestCase):
    """
    Tests regex building test function.
    """
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
    """
    Test the file find function.
    """
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

class TestMooseDocsImport(LogTestCase):
    """
    Tests for MooseDocsImport object.
    """
    def testBasic(self):
        items = moose_docs_import(include=['docs/content/**'],
                                  exclude=['docs/content/documentation/**'],
                                  root_dir=MooseDocs.ROOT_DIR,
                                  base='docs/content',
                                  extensions=('.md'))
        self.assertIsInstance(items, list)
        gold = '{}/docs/content/utilities/moose_docs/moose_markdown/index.md'. \
               format(MooseDocs.ROOT_DIR)
        self.assertIn(gold, items)

        gold = '{}/docs/content/documentation/systems/Kernels/framework/Diffusion.md'. \
               format(MooseDocs.ROOT_DIR)
        self.assertNotIn(gold, items)

        self.assertTrue(all(x.endswith('.md') for x in items))

    def testFilename(self):
        items = moose_docs_import(include=['docs/content/utilities/moose_docs/*',
                                           'docs/content/getting_started/**'],
                                  exclude=['docs/content/utilities/memory_logger/*',
                                           'docs/**/moose_markdown/*'],
                                  base='docs/content',
                                  extensions=('.md'))

        self.assertIsInstance(items, list)
        gold = '{}/docs/content/utilities/moose_docs/moose_markdown/index.md'. \
               format(MooseDocs.ROOT_DIR)
        self.assertNotIn(gold, items)

        gold = '{}/docs/content/documentation/systems/Kernels/framework/Diffusion.md'. \
               format(MooseDocs.ROOT_DIR)
        self.assertNotIn(gold, items)

        gold = '{}/docs/content/documentation/utilities/memory_logger/memory_logger.md'. \
               format(MooseDocs.ROOT_DIR)
        self.assertNotIn(gold, items)

        gold = '{}/docs/content/getting_started/installation/create_an_app.md'. \
               format(MooseDocs.ROOT_DIR)
        self.assertIn(gold, items)

        gold = '{}/docs/content/utilities/moose_docs/moose_markdown/index.md'. \
               format(MooseDocs.ROOT_DIR)
        self.assertNotIn(gold, items)

    def testErrors(self):
        moose_docs_import(include=42)
        self.assertInLogError('The "include" must be a list of str items.')

        moose_docs_import(include=[42])
        self.assertInLogError('The "include" must be a list of str items.')

        moose_docs_import(exclude=42)
        self.assertInLogError('The "exclude" must be a list of str items.')

        moose_docs_import(exclude=[42])
        self.assertInLogError('The "exclude" must be a list of str items.')

    def testIndex(self):
        items = moose_docs_import(include=['docs/content/index.md'],
                                  base='docs/content',
                                  extensions=('.md'))
        self.assertEqual(len(items), 1)
        self.assertEqual(items[0], os.path.join(MooseDocs.ROOT_DIR, 'docs', 'content', 'index.md'))

    def testExclude(self):
        items = moose_docs_import(include=['docs/content/**'],
                                  exclude=['docs/content/documentation/**/level_set/**'],
                                  base='docs/content',
                                  extensions=('.md'))

        gold = os.path.join(MooseDocs.ROOT_DIR,
                            'docs/content/documentation/systems/Kernels/framework/Diffusion.md')
        self.assertIn(gold, items)

        gold = os.path.join(MooseDocs.ROOT_DIR,
                            'docs/content/documentation/systems/Kernels/level_set/' \
                            'LevelSetAdvection.md')
        self.assertNotIn(gold, items)



if __name__ == '__main__':
    unittest.main(verbosity=2)
