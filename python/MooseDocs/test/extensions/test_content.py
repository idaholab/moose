#!/usr/bin/env python2
import unittest
import logging
from MooseDocs import common, base
from MooseDocs.test import MooseDocsTestCase
from MooseDocs.extensions import core, command, heading, content, acronym, floats, table
logging.basicConfig()

class TestContentList(MooseDocsTestCase):
    EXTENSIONS = [core, command, heading, content]

    def setupContent(self):
        config = [dict(root_dir='python/MooseDocs/test/content',
                       content=['extensions/core.md', 'extensions/content.md'])]
        return common.get_content(config, '.md')

    def testAST(self):
        ast = self.tokenize('!content list')
        self.assertSize(ast, 1)
        self.assertToken(ast(0), 'ContentToken', size=0, level=2, location='')

        ast = self.tokenize('!content list location=foo level=42')
        self.assertSize(ast, 1)
        self.assertToken(ast(0), 'ContentToken', size=0, level=42, location='foo')

    def testHTML(self):
        _, res = self.execute('!content list location=extensions')
        self.assertHTMLTag(res, 'body', size=1)
        self._assertHTML(res)

    def testMaterialize(self):
        _, res = self.execute('!content list location=extensions', renderer=base.MaterializeRenderer())
        self.assertHTMLTag(res, 'div', size=1)
        self._assertHTML(res)

    def _assertHTML(self, res):
        self.assertHTMLTag(res(0), 'div', size=3, class_='row')

        col = res(0)(0)
        self.assertHTMLTag(col, 'div', size=1, class_='col s12 m6 l4')
        self.assertHTMLTag(col(0), 'ul', size=1, class_='moose-a-to-z')
        self.assertHTMLTag(col(0)(0), 'li', size=1)
        self.assertHTMLTag(col(0)(0)(0), 'a', href='extensions/content.html', string='Content Extension')

        col = res(0)(1)
        self.assertHTMLTag(col, 'div', size=1, class_='col s12 m6 l4')
        self.assertHTMLTag(col(0), 'ul', size=1, class_='moose-a-to-z')
        self.assertHTMLTag(col(0)(0), 'li', size=1)
        self.assertHTMLTag(col(0)(0)(0), 'a', href='extensions/core.html', string='Core Extension')

        col = res(0)(2)
        self.assertHTMLTag(col, 'div', size=1, class_='col s12 m6 l4')
        self.assertHTMLTag(col(0), 'ul', size=0, class_='moose-a-to-z')

    def testLatex(self):
        _, res = self.execute('!content list location=extensions', renderer=base.LatexRenderer())
        self.assertSize(res, 4)

        self.assertLatex(res(0), 'Command' ,'par')
        self.assertLatex(res(1), 'Command', 'ContentItem', string='Content Extension')
        self.assertLatexArg(res(1), 0, 'Brace', 'extensions/content.tex')
        self.assertLatexArg(res(1), 1, 'Brace', 'content-extension')

        self.assertLatex(res(2), 'Command', 'ContentItem', string='Core Extension')
        self.assertLatexArg(res(2), 0, 'Brace', 'extensions/core.tex')
        self.assertLatexArg(res(2), 1, 'Brace', 'core-extension')
        self.assertLatex(res(3), 'Command' ,'par')

class TestContentAtoZ(MooseDocsTestCase):
    EXTENSIONS = [core, command, heading, content, table, floats, acronym]

    def setupContent(self):
        config = [dict(root_dir='python/MooseDocs/test/content',
                       content=['extensions/core.md', 'extensions/content.md',
                                'extensions/acronym.md', 'extensions/table.md'])]
        return common.get_content(config, '.md')

    def setupExtension(self, ext):
        if ext == acronym:
            return dict(active=False)

    def testAST(self):
        ast = self.tokenize('!content a-to-z')
        self.assertSize(ast, 1)
        self.assertToken(ast(0), 'AtoZToken', size=0, level=2, location='', buttons=True)

        ast = self.tokenize('!content a-to-z location=foo level=42 buttons=False')
        self.assertSize(ast, 1)
        self.assertToken(ast(0), 'AtoZToken', size=0, level=42, location='foo', buttons=False)

    def testHTML(self):
        _, res = self.execute('!content a-to-z location=extensions')
        self.assertHTMLTag(res, 'body', size=6)
        self._assertHTML(res)

    def testMaterialize(self):
        _, res = self.execute('!content a-to-z location=extensions', renderer=base.MaterializeRenderer())

        b = '0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ'
        self.assertHTMLTag(res(0), 'div', size=len(b), class_='moose-a-to-z-buttons')
        for i in range(0, len(b)):
            if b[i] in ['A', 'C', 'T']:
                self.assertHTMLTag(res(0)(i), 'a', class_='btn moose-a-to-z-button', string=b[i])
            else:
                self.assertHTMLTag(res(0)(i), 'a', class_='btn moose-a-to-z-button disabled', string=b[i])

        res(0).remove()
        self.assertHTMLTag(res, 'div', size=6)
        self._assertHTML(res)

    def testLatex(self):
        _, res = self.execute('!content a-to-z location=extensions', renderer=base.LatexRenderer())
        self.assertSize(res, 8)
        self.assertLatex(res(0), 'Command' ,'par')
        self.assertLatex(res(1), 'Command', 'ContentItem', string='Acronym Extension')
        self.assertLatexArg(res(1), 0, 'Brace', 'extensions/acronym.tex')
        self.assertLatexArg(res(1), 1, 'Brace', 'acronym-extension')

        self.assertLatex(res(2), 'Command' ,'par')
        self.assertLatex(res(3), 'Command', 'ContentItem', string='Content Extension')
        self.assertLatexArg(res(3), 0, 'Brace', 'extensions/content.tex')
        self.assertLatexArg(res(3), 1, 'Brace', 'content-extension')

        self.assertLatex(res(4), 'Command', 'ContentItem', string='Core Extension')
        self.assertLatexArg(res(4), 0, 'Brace', 'extensions/core.tex')
        self.assertLatexArg(res(4), 1, 'Brace', 'core-extension')

        self.assertLatex(res(5), 'Command' ,'par')
        self.assertLatex(res(6), 'Command', 'ContentItem', string='Table Extensions')
        self.assertLatexArg(res(6), 0, 'Brace', 'extensions/table.tex')
        self.assertLatexArg(res(6), 1, 'Brace', 'table-extensions')

    def _assertHTML(self, res):

        # A
        self.assertHTMLTag(res(0), 'h2', string='a', class_='moose-a-to-z')
        row = res(1)
        self._assertCols(row, [1,0,0])

        col = row(0)
        self.assertHTMLTag(col(0)(0), 'li', size=1)
        self.assertHTMLTag(col(0)(0)(0), 'a', href='extensions/acronym.html', string='Acronym Extension')

        # C
        self.assertHTMLTag(res(2), 'h2', string='c', class_='moose-a-to-z')
        row = res(3)
        self._assertCols(row, [1,1,0])

        col = row(0)
        self.assertHTMLTag(col(0)(0), 'li', size=1)
        self.assertHTMLTag(col(0)(0)(0), 'a', href='extensions/content.html', string='Content Extension')

        col = row(1)
        self.assertHTMLTag(col(0)(0), 'li', size=1)
        self.assertHTMLTag(col(0)(0)(0), 'a', href='extensions/core.html', string='Core Extension')

        # T
        self.assertHTMLTag(res(4), 'h2', string='t', class_='moose-a-to-z')
        row = res(5)
        self._assertCols(row, [1,0,0])

        col = row(0)
        self.assertHTMLTag(col(0)(0), 'li', size=1)
        self.assertHTMLTag(col(0)(0)(0), 'a', href='extensions/table.html', string='Table Extensions')

    def _assertCols(self, row, sz):
        self.assertHTMLTag(row, 'div', size=3, class_='row')
        for i in range(0, 2):
            self.assertHTMLTag(row(i), 'div', size=1, class_='col s12 m6 l4')
            self.assertHTMLTag(row(i)(0), 'ul', size=sz[i], class_='moose-a-to-z')

class TestContentToc(MooseDocsTestCase):
    EXTENSIONS = [core, command, heading, content]
    TEXT = '# TOC id=toc\n\n!content toc hide=toc levels=1 2\n\n## One\n\n## Two'

    def testAST(self):
        ast = self.tokenize(self.TEXT)
        self.assertSize(ast, 4)
        self.assertToken(ast(1), 'TableOfContents', size=0, levels=[1, 2], columns=1)

    def testHTML(self):
        _, res = self.execute(self.TEXT)
        self.assertHTMLTag(res, 'body', size=4)
        self._assertHTML(res(1))

    def testMaterialize(self):
        _, res = self.execute(self.TEXT, renderer=base.MaterializeRenderer())
        self.assertHTMLTag(res, 'div', size=4)
        self._assertHTML(res(1))

    def _assertHTML(self, res):

        self.assertHTMLTag(res, 'div', size=2, class_='moose-table-of-contents')
        self.assertHTMLTag(res(0), 'a', size=2, href='#one')
        self.assertHTMLString(res(0)(0), content='One')
        self.assertHTMLTag(res(0)(1), 'br', close=False)

        self.assertHTMLTag(res(1), 'a', size=2, href='#two')
        self.assertHTMLString(res(1)(0), content='Two')
        self.assertHTMLTag(res(1)(1), 'br', close=False)



if __name__ == '__main__':
    unittest.main(verbosity=2)
