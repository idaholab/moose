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
from MooseDocs.extensions import core, command, floats, media, gallery
logging.basicConfig()

class TestCard(MooseDocsTestCase):
    EXTENSIONS = [core, command, floats, media, gallery]

    def setupContent(self):
        """Virtual method for populating Content section in configuration."""
        config = [dict(root_dir='large_media', content=['testing/Flag_of_Idaho.svg'])]
        return common.get_content(config, '.md')

    def testAST(self):
        ast = self.tokenize("[!card!Flag_of_Idaho.svg title=Idaho](Details)")(0)
        self.assertSize(ast, 1)
        self.assertToken(ast(0), 'Card', size=3)

        self.assertToken(ast(0,0), 'CardImage', size=1)
        self.assertToken(ast(0,0,0), 'Image', size=0, src='Flag_of_Idaho.svg')

        self.assertToken(ast(0,1), 'CardContent', size=1)
        self.assertToken(ast(0,1,0), 'CardTitle', size=1, activator=True, deactivator=False)
        self.assertToken(ast(0,1,0,0), 'Word', content='Idaho')

        self.assertToken(ast(0,2), 'CardReveal', size=2)
        self.assertToken(ast(0,2,0), 'CardTitle', size=1, activator=False, deactivator=True)
        self.assertToken(ast(0,2,0,0), 'Word', content='Idaho')
        self.assertToken(ast(0,2,1), 'Paragraph', size=1)
        self.assertToken(ast(0,2,1,0), 'Word', content='Details')

    def testMaterialize(self):
        ast = self.tokenize("[!card!Flag_of_Idaho.svg title=Idaho](Details)")(0)
        res = self.render(ast, renderer=base.MaterializeRenderer())(0,0)

        self.assertHTMLTag(res, 'div', size=3, class_='card moose-card')
        self.assertHTMLTag(res(0), 'div', size=1, class_='card-image')
        self.assertHTMLTag(res(0,0), 'picture') # tested in test_media

        self.assertHTMLTag(res(1), 'div', size=1, class_='card-content')
        self.assertHTMLTag(res(1,0), 'span', size=2, class_='card-title activator')
        self.assertHTMLString(res(1,0,0), 'Idaho')
        self.assertHTMLTag(res(1,0,1), 'i', string='more_vert')

        self.assertHTMLTag(res(2), 'div', size=2, class_='card-reveal')
        self.assertHTMLTag(res(2,0), 'span', size=2, class_='card-title')
        self.assertHTMLString(res(2,0,0), 'Idaho')
        self.assertHTMLTag(res(2,0,1), 'i', string='close')
        self.assertHTMLTag(res(2,1), 'p', string='Details')

if __name__ == '__main__':
    unittest.main(verbosity=2)
