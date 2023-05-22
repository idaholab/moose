#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

"""
Tests for Component objects.
"""
import unittest
import os

from MooseDocs import common
from MooseDocs.common import exceptions
from MooseDocs.extensions import command

class TestTranslator(unittest.TestCase):
    def setUp(self):
        command.CommandExtension.EXTENSION_COMMANDS.clear()
        config = os.path.join('..', 'config.yml')
        self.translator, _ = common.load_config(config)
        self.translator.init()

    def testFindPage(self):
        page = self.translator.findPage('core.md')
        self.assertEqual(page.local, 'extensions/core.md')

    def testFindPageError(self):
        expect_err = 'Did you mean'
        with self.assertRaisesRegex(exceptions.MooseDocsException, expect_err) as cm:
            self.translator.findPage('wrong.md')

    def testFindPageKeyed(self):
        page = self.translator.findPage('core.md', key='python_test')
        self.assertEqual(page.local, 'extensions/core.md')
        self.assertEqual(page.key, 'python_test')

    def testFindPageKeyedAlternate(self):
        expect_err = 'test: extensions/core.md'
        with self.assertRaisesRegex(exceptions.MooseDocsException, expect_err) as cm:
            self.translator.findPage('core.md', key='framework')

    def testFindPageMissingKey(self):
        expect_err = 'The Content key "foobar" is not registered'
        with self.assertRaisesRegex(exceptions.MooseDocsException, expect_err) as cm:
            self.translator.findPage('core.md', key='foobar')

class TestTranslatorWithDuplicate(unittest.TestCase):
    def setUp(self):
        command.CommandExtension.EXTENSION_COMMANDS.clear()
        config = os.path.join('..', 'config.yml')
        duplicate_entry = {'duplicate': {'root_dir': 'python/MooseDocs/test/content_other', 'content': ['extensions_duplicate/autolink.md']}}
        self.translator, _ = common.load_config(config, Content=duplicate_entry)
        self.translator.init()

    def testFindPageImplicit(self):
        page = self.translator.findPage('core.md')
        self.assertEqual(page.key, 'python_test')
        self.assertEqual(page.local, 'extensions/core.md')

    def testFindPageImplicitError(self):
        with self.assertRaises(exceptions.MooseDocsException) as cm:
            self.translator.findPage('autolink.md')
        self.assertIn("Multiple pages with a name that ends with 'autolink.md' were found", cm.exception.message)
        self.assertIn("test: extensions/autolink.md", cm.exception.message)
        self.assertIn("duplicate: extensions_duplicate/autolink.md", cm.exception.message)

    def testFindPageExplicit(self):
        page = self.translator.findPage('autolink.md', key='duplicate')
        self.assertEqual(page.key, 'duplicate')
        self.assertEqual(page.local, 'extensions_duplicate/autolink.md')

class TestTranslatorWithOther(unittest.TestCase):
    def setUp(self):
        command.CommandExtension.EXTENSION_COMMANDS.clear()
        config = os.path.join('..', 'config.yml')
        duplicate_entry = {'other': {'root_dir': 'python/MooseDocs/test/content_other', 'content': ['wrong_key.md']}}
        self.translator, _ = common.load_config(config, Content=duplicate_entry)
        self.translator.init()

    def testFindPageOtherKeysException(self):
        with self.assertRaises(self.translator.FindPageOtherKeysException) as cm:
            self.translator.findPage('wrong_key.md', key='python_test')
        self.assertEqual(cm.exception.pages[0].key, 'other')
        page = self.translator.findPage('wrong_key.md')
        self.assertEqual(page, cm.exception.pages[0])

if __name__ == '__main__':
    unittest.main(verbosity=2)
