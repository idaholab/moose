#!/usr/bin/env python3
# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

"""
Tests for Component objects.
"""
import unittest
import os

from MooseDocs import common
from MooseDocs.common import exceptions
from MooseDocs.extensions import command
from MooseDocs.test import TEST_ROOT

class TestTranslator(unittest.TestCase):

    def setUp(self):
        command.CommandExtension.EXTENSION_COMMANDS.clear()
        config = os.path.join(TEST_ROOT, "config.yml")
        self.translator, _ = common.load_config(config)
        self.translator.init()

    def testFindPage(self):
        page = self.translator.findPage('core.md')
        self.assertEqual(page.local, 'extensions/core.md')

    def testFindPageError(self):
        with self.assertRaisesRegex(exceptions.MooseDocsException, "Did you mean"):
            page = self.translator.findPage("wrong.md")

if __name__ == '__main__':
    unittest.main(verbosity=2)
