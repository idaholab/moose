#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import mock
import unittest
import logging
import collections
from MooseDocs.test import MooseDocsTestCase
from MooseDocs.extensions import core, floats, heading, command, katex, include, config
from MooseDocs import base, common
from MooseDocs.tree import pages, tokens
logging.basicConfig()

class TestTokenizeEquation(MooseDocsTestCase):
    EXTENSIONS = [core, command, floats, heading, include, katex, config]

    def testBlockNoNumber(self):
        ast = self.tokenize('!equation\ny=2x')
        self.assertSize(ast, 1)
        self.assertToken(ast(0), 'Equation', tex='y=2x', inline=False, label=None, number=None)
        self.assertIn('moose-equation-', ast(0)['bookmark'])

    def testBlockWithNumber(self):
        ast = self.tokenize('!equation id=foo\ny=2x')
        self.assertSize(ast, 2) # shortcut still added, this will go away eventually
        self.assertToken(ast(0), 'Equation', tex='y=2x', inline=False, label='foo', number=1)
        self.assertIn('moose-equation-', ast(0)['bookmark'])

    def testInline(self):
        ast = self.tokenize('[!eq](y=2x)')
        self.assertSize(ast, 1)
        self.assertToken(ast(0), 'Paragraph', size=1)
        self.assertToken(ast(0,0), 'Equation', tex='y=2x', inline=True, label=None, number=None)
        self.assertIn('moose-equation-', ast(0,0)['bookmark'])

    def testExceptions(self):
        with self.assertLogs(level=logging.ERROR) as cm:
            ast = self.tokenize('[!equation](y=2x)')
        self.assertEqual(len(cm.output), 1)
        self.assertIn(r"The '!equation' command is a block level command", cm.output[0])

        with self.assertLogs(level=logging.ERROR) as cm:
            ast = self.tokenize('!eq\ny=2x')
        self.assertEqual(len(cm.output), 1)
        self.assertIn(r"The '!eq' command is an inline level command", cm.output[0])

        with self.assertLogs(level=logging.ERROR) as cm:
            ast = self.tokenize('[!eq id=foo](y=2x)')
        self.assertEqual(len(cm.output), 1)
        self.assertIn(r"The 'id' setting is not allowed", cm.output[0])

class TestTokenizeEquationReference(MooseDocsTestCase):
    EXTENSIONS = TestTokenizeEquation.EXTENSIONS

    def testLocalEqRef(self):
        ast = self.tokenize('[!eqref](second_law)')
        self.assertSize(ast, 1)
        self.assertToken(ast(0), 'Paragraph', size=1)
        self.assertToken(ast(0,0), 'EquationReference', size=0, label='second_law', filename=None)

    def testNonLocalEqRef(self):
        ast = self.tokenize('[!eqref](katex_include.md#second_law)')
        self.assertSize(ast, 1)
        self.assertToken(ast(0), 'Paragraph', size=1)
        self.assertToken(ast(0,0), 'EquationReference', size=0, label='second_law', filename='katex_include.md')

    def testExceptions(self):
        with self.assertLogs(level=logging.ERROR) as cm:
            ast = self.tokenize('!eqref')
        self.assertEqual(len(cm.output), 1)
        self.assertIn(r"The '!eqref' command is an inline level command", cm.output[0])

class TestRenderEquation(MooseDocsTestCase):
    EXTENSIONS = TestTokenizeEquation.EXTENSIONS

    def testRenderBlockEquation(self):
        ast = katex.Equation(None, tex=r'y=x')
        res = self.render(ast, renderer=base.HTMLRenderer())
        self.assertHTMLTag(res, 'body', size=1)
        self.assertHTMLTag(res(0), 'span', size=2, class_='moose-katex-block-equation')
        self.assertHTMLTag(res(0,0), 'span', size=0, class_='moose-katex-equation table-cell')
        self.assertHTMLTag(res(0,1), 'script', size=1, string='var element = document.getElementById("None");katex.render("y=x", element, {displayMode:true,throwOnError:false});')

    def testRenderBlockEquationWithLabel(self):
        ast = katex.Equation(None, tex=r'y=x', label='foo')
        res = self.render(ast, renderer=base.HTMLRenderer())
        self.assertHTMLTag(res, 'body', size=1)
        self.assertHTMLTag(res(0), 'span', size=3, class_='moose-katex-block-equation')
        self.assertHTMLTag(res(0,0), 'span', size=0, class_='moose-katex-equation table-cell')
        self.assertHTMLTag(res(0,1), 'span', size=1, class_='moose-katex-equation-number', string='(None)')
        self.assertHTMLTag(res(0,2), 'script', size=1, string='var element = document.getElementById("None");katex.render("y=x", element, {displayMode:true,throwOnError:false});')

    def testRenderBlockEquationWithLabelAndNumber(self):
        ast = katex.Equation(None, tex=r'y=x', label='foo', number=1980)
        res = self.render(ast, renderer=base.HTMLRenderer())
        self.assertHTMLTag(res, 'body', size=1)
        self.assertHTMLTag(res(0), 'span', size=3, class_='moose-katex-block-equation')
        self.assertHTMLTag(res(0,0), 'span', size=0, class_='moose-katex-equation table-cell')
        self.assertHTMLTag(res(0,1), 'span', size=1, class_='moose-katex-equation-number', string='(1980)')
        self.assertHTMLTag(res(0,2), 'script', size=1, string='var element = document.getElementById("None");katex.render("y=x", element, {displayMode:true,throwOnError:false});')

    def testRenderInlineEquation(self):
        ast = katex.Equation(None, tex=r'y=x', inline=True)
        res = self.render(ast, renderer=base.HTMLRenderer())
        self.assertHTMLTag(res, 'body', size=1)
        self.assertHTMLTag(res(0), 'span', size=1, class_='moose-katex-inline-equation')
        self.assertHTMLTag(res(0,0), 'script', size=1, string='var element = document.getElementById("None");katex.render("y=x", element, {displayMode:false,throwOnError:false});')

class TestRenderEquationReference(MooseDocsTestCase):
    EXTENSIONS = TestTokenizeEquation.EXTENSIONS

    def setupContent(self):
        config = [dict(root_dir='python/MooseDocs/test/content',
                       content=['extensions/katex.md',
                                'extensions/katex_include.md',
                                'extensions/katex_include2.md'])]
        return common.get_content(config, '.md')

    def testLocalEqRef(self):
        ast = katex.EquationReference(None, label='second_law')
        res = self.render(ast, renderer=base.HTMLRenderer())
        self.assertHTMLTag(res, 'body', size=1)
        self.assertHTMLTag(res(0), 'a', size=1, class_='moose-equation-reference', href='#None', string='Eq. (None)')

    def testNonLocalEqRefNoHeading(self):
        ast = katex.EquationReference(None, label='second_law', filename='katex_include.md')
        res = self.render(ast, renderer=base.HTMLRenderer())
        self.assertHTMLTag(res, 'body', size=1)
        self.assertHTMLTag(res(0), 'a', size=2, class_='moose-equation-reference')
        self.assertIn('extensions/katex_include.html#moose-equation-', res(0)['href'])
        self.assertIn('katex_include.md, ', res(0,0)['content'])
        self.assertIn('Eq. (2)', res(0,1)['content'])

    def testNonLocalEqRefWithHeading(self):
        ast = katex.EquationReference(None, label='second_law', filename='katex_include2.md')
        res = self.render(ast, renderer=base.HTMLRenderer())
        self.assertHTMLTag(res, 'body', size=1)
        self.assertHTMLTag(res(0), 'a', size=9, class_='moose-equation-reference')
        self.assertIn('extensions/katex_include2.html#moose-equation-', res(0)['href'])
        self.assertIn('Equations', res(0,0)['content'])
        self.assertIn('Eq. (2)', res(0,8)['content'])

    def testLocalEqRef(self):
        ast = katex.EquationReference(None, label='second_law')
        res = self.render(ast, renderer=base.LatexRenderer())
        self.assertLatexString(res(0), content='Eq.~')
        self.assertLatexCommand(res(1), 'eqref', string='second_law')

    def testExceptions(self):
        ast = katex.EquationReference(None, label='first_law', filename='katex_include2.md')
        with self.assertLogs(level=logging.ERROR) as cm:
            res = self.render(ast, renderer=base.HTMLRenderer())
        self.assertHTMLTag(res, 'body', size=1)
        self.assertHTMLTag(res(0), 'a', size=9, class_='moose-error')
        self.assertIn('extensions/katex_include2.html#None', res(0)['href'])
        self.assertIn('Equations', res(0,0)['content'])
        self.assertIn('extensions/katex_include2.md#first_law', res(0,8)['content'])
        self.assertEqual(len(cm.output), 2)
        self.assertIn('Could not find equation with key first_law on page extensions/katex_include2.md', cm.output[1])


if __name__ == '__main__':
    unittest.main(verbosity=2, buffer=False)
