#!/usr/bin/env python3
import unittest
import logging
from MooseDocs.test import MooseDocsTestCase
from MooseDocs.extensions import core, comment
from MooseDocs import base
logging.basicConfig()

class TestInlineComment(MooseDocsTestCase):
    EXTENSIONS = [core, comment]
    TEXT = u"Not comment!!this is\nand not"

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
        self.assertLatexString(res(1), u'Not')
        self.assertLatexString(res(2), u' ')
        self.assertLatexString(res(3), u'comment')
        self.assertLatexString(res(4), u' ')
        self.assertLatexString(res(5), u'and')
        self.assertLatexString(res(6), u' ')
        self.assertLatexString(res(7), u'not')

    def testReveal(self):
        _, res = self.execute(self.TEXT, renderer=base.RevealRenderer())
        self._assertHTML(res)

    def _assertHTML(self, res):
        self.assertEqual(len(res), 1)
        self.assertEqual(len(res(0)), 7)
        self.assertHTMLTag(res(0), 'p')
        self.assertHTMLString(res(0)(0), u'Not')
        self.assertHTMLString(res(0)(1), u' ')
        self.assertHTMLString(res(0)(2), u'comment')
        self.assertHTMLString(res(0)(3), u' ')
        self.assertHTMLString(res(0)(4), u'and')
        self.assertHTMLString(res(0)(5), u' ')
        self.assertHTMLString(res(0)(6), u'not')

    def _assertAST(self, ast):
        self.assertEqual(len(ast), 1)
        self.assertEqual(len(ast(0)), 8)

        self.assertToken(ast(0), 'Paragraph')
        self.assertToken(ast(0)(0), 'Word', content=u'Not')
        self.assertToken(ast(0)(1), 'Space')
        self.assertToken(ast(0)(2), 'Word', content=u'comment')
        self.assertToken(ast(0)(3), 'Comment', content=u'this is')
        self.assertToken(ast(0)(4), 'Break', count=1)
        self.assertToken(ast(0)(5), 'Word', content=u'and')
        self.assertToken(ast(0)(6), 'Space')
        self.assertToken(ast(0)(7), 'Word', content=u'not')

class TestHTMLComment(TestInlineComment):
    TEXT = u"Not comment<!--this is-->\nand not"

if __name__ == '__main__':
    unittest.main(verbosity=2)
