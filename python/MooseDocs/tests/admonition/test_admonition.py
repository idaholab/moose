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
