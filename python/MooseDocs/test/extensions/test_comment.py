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
from MooseDocs.extensions import core, comment
from MooseDocs import base
logging.basicConfig()

class TestInlineComment(MooseDocsTestCase):
    EXTENSIONS = [core, comment]
    TEXT = "Not comment!!this is\nand not"

    def testAST(self):
        ast = self.tokenize(self.TEXT)
        self._assertAST(ast)

    def testHTML(self):
        _, res = self.execute(self.TEXT)
        self._assertHTML(res)

    def testMaterialize(self):
        _, res = self.execute(self.TEXT, renderer=base.MaterializeRenderer())
        self._assertHTML(res)

    def testLatex(self):
        _, res = self.execute(self.TEXT, renderer=base.LatexRenderer())
        self.assertEqual(len(res), 8)
        self.assertLatex(res(0), 'Command', 'par')
        self.assertLatexString(res(1), 'Not')
        self.assertLatexString(res(2), ' ')
        self.assertLatexString(res(3), 'comment')
        self.assertLatexString(res(4), ' ')
        self.assertLatexString(res(5), 'and')
        self.assertLatexString(res(6), ' ')
        self.assertLatexString(res(7), 'not')

    def testReveal(self):
        _, res = self.execute(self.TEXT, renderer=base.RevealRenderer())
        self._assertHTML(res)

    def _assertHTML(self, res):
        self.assertEqual(len(res), 1)
        self.assertEqual(len(res(0)), 7)
        self.assertHTMLTag(res(0), 'p')
        self.assertHTMLString(res(0)(0), 'Not')
        self.assertHTMLString(res(0)(1), ' ')
        self.assertHTMLString(res(0)(2), 'comment')
        self.assertHTMLString(res(0)(3), ' ')
        self.assertHTMLString(res(0)(4), 'and')
        self.assertHTMLString(res(0)(5), ' ')
        self.assertHTMLString(res(0)(6), 'not')

    def _assertAST(self, ast):
        self.assertEqual(len(ast), 1)
        self.assertEqual(len(ast(0)), 8)

        self.assertToken(ast(0), 'Paragraph')
        self.assertToken(ast(0)(0), 'Word', content='Not')
        self.assertToken(ast(0)(1), 'Space')
        self.assertToken(ast(0)(2), 'Word', content='comment')
        self.assertToken(ast(0)(3), 'Comment', content='this is')
        self.assertToken(ast(0)(4), 'Break', count=1)
        self.assertToken(ast(0)(5), 'Word', content='and')
        self.assertToken(ast(0)(6), 'Space')
        self.assertToken(ast(0)(7), 'Word', content='not')

class TestHTMLComment(TestInlineComment):
    TEXT = "Not comment<!--this is-->\nand not"

if __name__ == '__main__':
    unittest.main(verbosity=2)
