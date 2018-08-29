#!/usr/bin/env python2
"""Testing for MooseDocs.extensions.katex MooseDocs extension."""
import unittest

import MooseDocs
from MooseDocs.extensions import katex
from MooseDocs.tree import tokens, html, latex
from MooseDocs.base import testing, renderers

# TOKEN OBJECTS TESTS
class TestTokens(unittest.TestCase):
    """Test Token object for MooseDocs.extensions.katex MooseDocs extension."""

    EXTENSIONS = ['MooseDocs.extensions.core', 'MooseDocs.extensions.katex']

    def testLatexBlockEquation(self):
        token = katex.LatexBlockEquation(tex='y')
        self.assertEqual(token.tex, 'y')

    def testLatexInlineEquation(self):
        token = katex.LatexInlineEquation(tex='y')
        self.assertEqual(token.tex, 'y')


# TOKENIZE TESTS
class TestKatexBlockEquationComponentTokenize(testing.MooseDocsTestCase):
    """Test tokenization of KatexBlockEquationComponent"""

    EXTENSIONS = [MooseDocs.extensions.core, MooseDocs.extensions.katex]

    def testToken(self):
        ast = self.ast(u'\\begin{equation}\nfoo\n\\end{equation}')
        self.assertIsInstance(ast(0), tokens.Paragraph)
        self.assertIsInstance(ast(0)(0), katex.LatexBlockEquation)

class TestKatexInlineEquationComponentTokenize(testing.MooseDocsTestCase):
    """Test tokenization of KatexInlineEquationComponent"""

    EXTENSIONS = [MooseDocs.extensions.core, MooseDocs.extensions.katex]

    def testToken(self):
        ast = self.ast(u'$foo$')
        self.assertIsInstance(ast(0)(0), katex.LatexInlineEquation)

# RENDERER TESTS
class TestRenderLatexEquationHTML(testing.MooseDocsTestCase):
    """Test renderering of RenderLatexEquation with HTMLRenderer"""

    EXTENSIONS = ['MooseDocs.extensions.core', 'MooseDocs.extensions.katex']
    RENDERER = renderers.HTMLRenderer
    TEXT = u'$ax+b$\n\n\\begin{equation}\nE=mc^2\n\\end{equation}'

    def node(self):
        return self.render(self.TEXT).find('moose-content', attr='class')

    def testTree(self):
        node = self.node()

        # Inline
        self.assertIsInstance(node(0), html.Tag)
        self.assertEqual(node(0).name, 'p')
        self.assertIsInstance(node(0)(0), html.Tag)
        self.assertEqual(node(0)(0).name, 'span')
        self.assertIsInstance(node(0)(0)(0), html.Tag)
        self.assertEqual(node(0)(0)(0).name, 'script')
        self.assertIsInstance(node(0)(0)(0)(0), html.String)
        self.assertIn('moose-equation-', node(0)(0)(0)(0).content)
        self.assertIn('katex.render("ax+b', node(0)(0)(0)(0).content)
        self.assertIn('displayMode:false', node(0)(0)(0)(0).content)

        # Block
        self.assertIsInstance(node(1), html.Tag)
        self.assertEqual(node(1).name, 'p')
        self.assertIsInstance(node(1)(0), html.Tag)
        self.assertEqual(node(1)(0).name, 'span')
        self.assertIn('moose-katex-block-equation', node(1)(0)['class'])

        self.assertIsInstance(node(1)(0)(0), html.Tag)
        self.assertEqual(node(1)(0)(0).name, 'span')
        self.assertIn('moose-katex-equation', node(1)(0)(0)['class'])

        self.assertIsInstance(node(1)(0)(1), html.Tag)
        self.assertEqual(node(1)(0)(1).name, 'span')
        self.assertIn('moose-katex-equation-number', node(1)(0)(1)['class'])
        self.assertIsInstance(node(1)(0)(1)(0), html.String)
        self.assertIn('(', node(1)(0)(1)(0).content)

        self.assertIsInstance(node(1)(0)(2), html.Tag)
        self.assertEqual(node(1)(0)(2).name, 'script')
        self.assertIsInstance(node(1)(0)(2)(0), html.String)
        self.assertIn('moose-equation-', node(1)(0)(2)(0).content)
        self.assertIn('katex.render("E=mc^2', node(1)(0)(2)(0).content)
        self.assertIn('displayMode:true', node(1)(0)(2)(0).content)

    def testWrite(self):
        content = self.node().write()

        self.assertIn('moose-equation-', content)
        self.assertIn('moose-katex-inline-equation', content)

        self.assertIn('katex.render("ax+b', content)
        self.assertIn('displayMode:false', content)

        self.assertIn('moose-katex-block-equation', content)
        self.assertIn('katex.render("E=mc^2', content)
        self.assertIn('displayMode:true', content)

class TestRenderLatexEquationMaterialize(TestRenderLatexEquationHTML):
    """Test renderering of RenderLatexEquation with MaterializeRenderer"""

    RENDERER = renderers.MaterializeRenderer

@unittest.skip("LaTeX WIP")
class TestRenderLatexEquationLatex(testing.MooseDocsTestCase):
    """Test renderering of RenderLatexEquation with LatexRenderer"""

    EXTENSIONS = [MooseDocs.extensions.core, MooseDocs.extensions.katex]
    RENDERER = renderers.LatexRenderer
    TEXT = u'$ax+b$\n\n\\begin{equation}\nE=mc^2\n\\end{equation}'

    def node(self):
        return self.render(self.TEXT).find('document')

    def testTree(self):
        node = self.node()
        self.assertIsInstance(node(1), latex.String)
        self.assertEqual(node(1).content, u'$ax+b$')
        self.assertIsInstance(node(2), latex.Environment)
        self.assertIsInstance(node(2)(0), latex.String)
        self.assertEqual(node(2)(0).content, u'E=mc^2')

    def testWrite(self):
        node = self.node()
        self.assertIn('\\$ax+b\\$', node.write())
        self.assertIn('\\begin{equation}\nE=mc\\^2\n\\end{equation}', node.write())

if __name__ == '__main__':
    unittest.main(verbosity=2)
