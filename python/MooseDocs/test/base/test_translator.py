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
        page = self.translator.findPage('core.md', key='test')
        self.assertEqual(page.local, 'extensions/core.md')

    def testFindPageKeyedAlternate(self):
        expect_err = 'test: extensions/core.md'
        with self.assertRaisesRegex(exceptions.MooseDocsException, expect_err) as cm:
            self.translator.findPage('core.md', key='framework')

    def testFindPageMissingKey(self):
        expect_err = 'The Content key "foobar" is not registered'
        with self.assertRaisesRegex(exceptions.MooseDocsException, expect_err) as cm:
            self.translator.findPage('core.md', key='foobar')

if __name__ == '__main__':
    unittest.main(verbosity=2)
