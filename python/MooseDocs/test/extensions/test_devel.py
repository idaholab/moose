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
from MooseDocs import common, base, tree
from MooseDocs.test import MooseDocsTestCase
from MooseDocs.extensions import core, command, floats, devel
logging.basicConfig()

class TestExample(MooseDocsTestCase):
    EXTENSIONS = [core, command, floats, devel]

    def testNoFloatAST(self):
        ast = self.tokenize('!devel example\ntest')
        self.assertToken(ast(0), 'Example', size=2)
        self.assertToken(ast(0,0), 'Code', size=0, content='test')
        self.assertToken(ast(0,1), 'Paragraph', size=1)
        self.assertToken(ast(0,1,0), 'Word', size=0, content='test')

    def testFloatAST(self):
        ast = self.tokenize('!devel example id=foo caption=bar\ntest')
        self.assertToken(ast(0), 'Float', size=2)
        self.assertToken(ast(0,0), 'FloatCaption', size=1, prefix='Example', key='foo')
        self.assertToken(ast(0,0,0), 'Word', size=0, content='bar')
        self.assertToken(ast(0,1), 'Example', size=2)
        self.assertToken(ast(0,1,0), 'Code', size=0, content='test')
        self.assertToken(ast(0,1,1), 'Paragraph', size=1)
        self.assertToken(ast(0,1,1,0), 'Word', size=0, content='test')

class TestSettings(MooseDocsTestCase):
    EXTENSIONS = [core, command, floats, devel]

    def testNoFloatAST(self):
        ast = self.tokenize('!devel settings module=MooseDocs.extensions.devel object=SettingsCommand')
        self.assertToken(ast(0), 'Table', size=2)
        self.assertToken(ast(0,0), 'TableHead', size=1)
        self.assertToken(ast(0,1), 'TableBody', size=7)

    def testFloatAST(self):
        ast = self.tokenize('!devel settings id=foo module=MooseDocs.extensions.devel object=SettingsCommand')
        self.assertToken(ast(0), 'TableFloat', size=2)
        self.assertToken(ast(0,1), 'Table', size=2)
        self.assertToken(ast(0,1,0), 'TableHead', size=1)
        self.assertToken(ast(0,1,1), 'TableBody', size=7)

    def testErrors(self):
        ast = self.tokenize('!devel settings object=SettingsCommand')
        self.assertToken(ast(0), 'ErrorToken', message="The 'module' setting is required.")

        ast = self.tokenize('!devel settings module=MooseDocs.extensions.devel')
        self.assertToken(ast(0), 'ErrorToken', message="The 'object' setting is required.")

        ast = self.tokenize('!devel settings module=wrong object=SettingsCommand')
        self.assertToken(ast(0), 'ErrorToken', message="Unable to load the 'wrong' module.")

        ast = self.tokenize('!devel settings module=MooseDocs.extensions.devel object=wrong')
        self.assertToken(ast(0), 'ErrorToken', message="Unable to load the 'wrong' attribute from the 'MooseDocs.extensions.devel' module.")

class TestRenderExample(MooseDocsTestCase):
    EXTENSIONS = [core, command, floats, devel]

    def testHTML(self):
        ast = self.tokenize('!devel example\ntest')
        res = self.render(ast, renderer=base.HTMLRenderer())
        self.assertHTMLTag(res, 'body', size=2)
        self.assertHTMLTag(res(0), 'pre', size=1, class_='moose-pre')
        self.assertHTMLTag(res(0,0), 'code', size=1, string='test')
        self.assertHTMLTag(res(1), 'p', string='test')

    def testMaterialize(self):
        ast = self.tokenize('!devel example\ntest')
        res = self.render(ast, renderer=base.MaterializeRenderer())

        self.assertHTMLTag(res(0), 'div', size=3, class_='moose-devel-example')
        self.assertHTMLTag(res(0,0), 'ul', size=2, class_='tabs')
        self.assertHTMLTag(res(0,0,0), 'li', size=1, class_='tab')
        self.assertHTMLTag(res(0,0,0,0), 'a', string='Markdown')
        self.assertHTMLTag(res(0,0,1), 'li', size=1, class_='tab')
        self.assertHTMLTag(res(0,0,1,0), 'a', string='HTML')

        self.assertHTMLTag(res(0,1), 'div', size=1, class_='moose-devel-example-code')
        self.assertHTMLTag(res(0,1,0), 'pre', string='test')
        self.assertHTMLTag(res(0,2), 'div', size=1, class_='moose-devel-example-html')
        self.assertHTMLTag(res(0,2,0), 'p', string='test')

    def testLatex(self):
        ast = self.tokenize('!devel example\ntest')
        res = self.render(ast, renderer=base.LatexRenderer())
        self.assertLatex(res(0), 'Environment', 'example')
        self.assertLatex(res(0,0), 'Environment', 'verbatim', string='test')
        self.assertLatex(res(0,1), 'Command', 'tcblower')
        self.assertLatex(res(0,2), 'Command', 'par')
        self.assertLatexString(res(0,3), content='test')

if __name__ == '__main__':
    unittest.main(verbosity=2)
