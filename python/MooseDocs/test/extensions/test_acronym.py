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
from MooseDocs.test import MooseDocsTestCase
from MooseDocs.extensions import core, command, floats, table, acronym
from MooseDocs import base
logging.basicConfig()

class TestInlineAcronym(MooseDocsTestCase):
    EXTENSIONS = [core, command, floats, table, acronym]
    TEXT = "[!ac](INL) and [!ac](INL)"

    def setupExtension(self, ext):
        if ext == acronym:
            return dict(acronyms=dict(INL="Idaho National Laboratory"))

    def testAST(self):
        ast = self.tokenize(self.TEXT)
        self._assertAST(ast)

    def testHTML(self):
        _, res = self.execute(self.TEXT)
        self._assertHTML(res)

    def testMaterialize(self):
        _, res = self.execute(self.TEXT, renderer=base.MaterializeRenderer())
        self._assertHTML(res)

        self.assertHTMLTag(res(0)(4), 'span', class_='tooltipped')
        tag = res(0)(4)
        self.assertEqual(tag['data-delay'], 50)
        self.assertEqual(tag['data-tooltip'], 'Idaho National Laboratory')
        self.assertEqual(tag['data-position'], 'top')

    def testReveal(self):
        _, res = self.execute(self.TEXT, renderer=base.RevealRenderer())
        self._assertHTML(res)

    def testLatex(self):
        _, res = self.execute(self.TEXT, renderer=base.LatexRenderer())
        self.assertSize(res, 6)
        self.assertLatex(res(0), 'Command', 'par')
        self.assertLatexString(res(1), u'Idaho National Laboratory (INL)')
        self.assertLatexString(res(2), u' ')
        self.assertLatexString(res(3), u'and')
        self.assertLatexString(res(4), u' ')
        self.assertLatexString(res(5), u'INL')

    def _assertHTML(self, res):
        self.assertSize(res, 1)
        self.assertHTMLTag(res(0), 'p', size=5)

        self.assertHTMLTag(res(0)(0), 'span', string=u'Idaho National Laboratory (INL)')

        self.assertHTMLString(res(0)(1), u' ')
        self.assertHTMLString(res(0)(2), u'and')
        self.assertHTMLString(res(0)(3), u' ')

        self.assertHTMLTag(res(0)(4), 'span', string=u'INL')

    def _assertAST(self, ast):
        self.assertSize(ast, 1)
        self.assertSize(ast(0), 5)

        self.assertToken(ast(0), 'Paragraph')
        self.assertToken(ast(0)(0), 'AcronymToken', acronym=u'INL')
        self.assertToken(ast(0)(1), 'Space')
        self.assertToken(ast(0)(2), 'Word', content=u'and')
        self.assertToken(ast(0)(3), 'Space')
        self.assertToken(ast(0)(4), 'AcronymToken', acronym=u'INL')

class TestAcronymList(MooseDocsTestCase):
    EXTENSIONS = [core, command, floats, table, acronym]
    TEXT = "!acronym list complete=True"

    def setupExtension(self, ext):
        if ext == acronym:
            return dict(acronyms=dict(MSU="Montana State University",
                                      WSU="Washington State University",
                                      MTU="Michigan Technological University"))
    def testAST(self):
        ast = self.tokenize(self.TEXT)
        self._assertAST(ast)

    def testHTML(self):
        _, res = self.execute(self.TEXT)
        self._assertHTML(res)

    def testMaterialize(self):
        _, res = self.execute(self.TEXT, renderer=base.MaterializeRenderer())
        self._assertHTML(res)

    def testReveal(self):
        _, res = self.execute(self.TEXT, renderer=base.RevealRenderer())
        self._assertHTML(res)

    def testLatex(self):
        _, res = self.execute(self.TEXT, renderer=base.LatexRenderer())
        self.assertSize(res, 1)
        self.assertLatex(res(0), 'Environment', 'tabulary', size=3, before_end=u'\n',
                         after_begin=u'\n', end=u'\n')
        self.assertLatexArg(res(0), 0, 'Brace', string=u'\\linewidth')
        self.assertLatexArg(res(0), 1, 'Brace', string=u'LL')
        self.assertLatexString(res(0)(0), u'MSU&Montana State University\\\\')
        self.assertLatexString(res(0)(1), u'WSU&Washington State University\\\\')
        self.assertLatexString(res(0)(2), u'MTU&Michigan Technological University\\\\')

    def _assertHTML(self, res):
        self.assertSize(res, 1) # root
        self.assertHTMLTag(res(0), 'div', size=1)
        self.assertHTMLTag(res(0)(0), 'table', size=2)

        thead = res(0)(0)(0)
        self.assertHTMLTag(thead, 'thead', size=1)
        self.assertHTMLTag(thead(0), 'tr', size=2)
        self.assertHTMLTag(thead(0)(0), 'th', style=';text-align:left', string='Acronym')
        self.assertHTMLTag(thead(0)(1), 'th', style=';text-align:left', string='Description')

        tbody = res(0)(0)(1)
        self.assertHTMLTag(tbody, 'tbody', size=3)
        self.assertHTMLTag(tbody(0), 'tr', size=2)
        self.assertHTMLTag(tbody(0)(0), 'td', style=';text-align:left', string='MSU')
        self.assertHTMLTag(tbody(0)(1), 'td', style=';text-align:left', string='Montana State University')

        self.assertHTMLTag(tbody, 'tbody', size=3)
        self.assertHTMLTag(tbody(1), 'tr', size=2)
        self.assertHTMLTag(tbody(1)(0), 'td', style=';text-align:left', string='MTU')
        self.assertHTMLTag(tbody(1)(1), 'td', style=';text-align:left', string='Michigan Technological University')

        self.assertHTMLTag(tbody, 'tbody', size=3)
        self.assertHTMLTag(tbody(2), 'tr', size=2)
        self.assertHTMLTag(tbody(2)(0), 'td', style=';text-align:left', string='WSU')
        self.assertHTMLTag(tbody(2)(1), 'td', style=';text-align:left', string='Washington State University')

    def _assertAST(self, ast):
        self.assertEqual(len(ast), 1)
        self.assertToken(ast(0), 'AcronymListToken')



if __name__ == '__main__':
    unittest.main(verbosity=2)
