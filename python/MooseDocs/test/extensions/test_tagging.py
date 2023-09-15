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
import logging
from MooseDocs import common, base
from MooseDocs.common import exceptions
from MooseDocs.test import MooseDocsTestCase
from MooseDocs.extensions import core, command, tagging
logging.basicConfig()

class TestTaggingInit(MooseDocsTestCase):
    EXTENSIONS = [core, command, tagging]

    def setupExtension(self, ext):
        if ext == tagging:
            return dict(active=True)

    def test(self):
        with self.assertLogs(level=logging.WARNING) as cm:
            self._MooseDocsTestCase__setup()
        self.assertEqual(len(cm.output), 2)
        self.assertIn('No javascript file identified.', cm.output[0])
        self.assertIn('The tagging extension is experimental!', cm.output[1])

class TestTaggingCommand(MooseDocsTestCase):
    EXTENSIONS = [core, command, tagging]
    TEXT = '!tag name=test pairs=application:moose foo:bar'
    TEXT_DUPLICATE = '!tag name=test pairs=application:moose foo:bas application:bar'
    TEXT_NOT_ALLOWED = '!tag name=test pairs=application:moose application_wrong:bar'
    TEXT_NO_KEYS = '!tag name=test'
    TEXT_NO_NAME = '!tag pairs=application:moose'

    def setupExtension(self, ext):
        if ext == tagging:
            return dict(active=True, allowed_keys=['application', 'foo'], js_file='tagging.js')

    def setupContent(self):
        config = [dict(root_dir='python/MooseDocs/test/content', content=['js/tagging.js'])]
        return common.get_content(config, '.md')

    def testStandardUsage(self):
        with self.assertLogs(level=logging.INFO) as cm: # For warning suppression. TODO: remove cm when non-experimental
            ast = self.tokenize(self.TEXT)
        self.assertSize(ast, 1)
        self.assertEqual(ast(0)['attr_name'], 'tag_test')
        self.assertEqual(ast(0)['key_vals'], {'application':'moose', 'foo':'bar'})

    def testDuplicateKey(self):
        with self.assertLogs(level=logging.ERROR) as cm:
            ast = self.tokenize(self.TEXT_DUPLICATE)
        self.assertEqual(len(cm.output), 1)
        self.assertIn("Following 'key' provided more than once;", cm.output[0])
        self.assertSize(ast, 1)
        self.assertEqual(ast(0)['key_vals'], {'application':'moose', 'foo':'bas'})

    def testNotAllowed(self):
        with self.assertLogs(level=logging.WARNING) as cm:
            ast = self.tokenize(self.TEXT_NOT_ALLOWED)
        self.assertEqual(len(cm.output), 2)
        self.assertIn("Provided 'key' not in allowed_keys", cm.output[1])
        self.assertSize(ast, 1)
        self.assertEqual(ast(0)['key_vals'], {'application':'moose'})

    def testNoKeys(self):
        with self.assertLogs(level=logging.ERROR) as cm:
            ast = self.tokenize(self.TEXT_NO_KEYS)
        self.assertEqual(len(cm.output), 1)
        self.assertIn("No key:value pairs provided", cm.output[0])
        self.assertSize(ast, 1)
        self.assertEqual(ast(0)['key_vals'], {})

    def testNoName(self):
        with self.assertLogs(level=logging.ERROR) as cm:
            ast = self.tokenize(self.TEXT_NO_NAME)
        self.assertEqual(len(cm.output), 1)
        self.assertIn("No 'name' provided for page and associated tags", cm.output[0])
        self.assertSize(ast, 0) # No token is created if name is not provided

    def testTaggingDuplicateNamesWarning(self):
        with self.assertLogs(level=logging.WARNING) as cm:
            ast1 = self.tokenize(self.TEXT)
            ast2 = self.tokenize(self.TEXT)
        self.assertEqual(len(cm.output), 2)
        self.assertIn('Tag page identifier already exists;', cm.output[1])
        self.assertSize(ast1, 1)
        self.assertEqual(ast1(0)['attr_name'], 'tag_test')
        self.assertSize(ast2, 1)
        self.assertEqual(ast2(0)['attr_name'], '') #Not added to global attributes, so name attribute is empty

if __name__ == '__main__':
    unittest.main(verbosity=2)

