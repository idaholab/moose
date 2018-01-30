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

import unittest
from MooseDocs.testing import MarkdownTestCase

class TestAdmonition(MarkdownTestCase):
    EXTENSIONS = ['MooseDocs.extensions.admonition']

    def testBasic(self):
        md = '!admonition error\nThis a message.'
        html = self.convert(md)
        self.assertIn('<div class="admonition error">', html)
        self.assertIn('<p class="admonition-title">Error</p>', html)

    def testTitle(self):
        md = '!admonition error The Title\nThis a message.'
        html = self.convert(md)
        self.assertIn('<div class="admonition error">', html)
        self.assertIn('<p class="admonition-title">Error: The Title</p>', html)

    def testTitleAndSettings(self):
        md = '!admonition error The Title foo=bar\nThis a message.'
        html = self.convert(md)
        self.assertIn('<div class="admonition error">', html)
        self.assertIn('<p class="admonition-title">Error: The Title</p>', html)

    def testBasicWithSettings(self):
        md = '!admonition error foo=bar\nThis a message.'
        html = self.convert(md)
        self.assertIn('<div class="admonition error">', html)
        self.assertIn('<p class="admonition-title">Error</p>', html)

    def testCommands(self):
        for cmd in ['info', 'note', 'important', 'warning', 'danger', 'error']:
            md = '!admonition {}\nThis a message.'.format(cmd)
            html = self.convert(md)
            self.assertIn('<div class="admonition {}">'.format(cmd), html)

if __name__ == '__main__':
    unittest.main(verbosity=2)
