#!/usr/bin/env python2
"""Testing for MooseDocs.extensions.alert MooseDocs extension."""
import unittest

from MooseDocs.extensions import core, command, alert
from MooseDocs.tree import tokens, html
from MooseDocs.base import testing, renderers

# TOKEN OBJECTS TESTS
class TestTokens(unittest.TestCase):
    """Test Token object for MooseDocs.extensions.alert MooseDocs extension."""

    def testAlertToken(self):
        token = alert.AlertToken(brand=u'warning')
        self.assertEqual(token.brand, u'warning')

# TOKENIZE TESTS
class TestAlertCommandTokenize(testing.MooseDocsTestCase):
    """Test tokenization of AlertCommand"""

    EXTENSIONS = [core, command, alert]

    def testError(self):
        ast = self.ast(u'!alert error\ncontent')
        self.assertIsInstance(ast(0), alert.AlertToken)
        self.assertEqual(ast(0).brand, u'error')
        self.assertIsInstance(ast(0)(0), tokens.Word)
        self.assertEqual(ast(0)(0).content, u'content')

    def testWarning(self):
        ast = self.ast(u'!alert warning\ncontent')
        self.assertIsInstance(ast(0), alert.AlertToken)
        self.assertEqual(ast(0).brand, u'warning')
        self.assertIsInstance(ast(0)(0), tokens.Word)
        self.assertEqual(ast(0)(0).content, u'content')

    def testNote(self):
        ast = self.ast(u'!alert note\ncontent')
        self.assertIsInstance(ast(0), alert.AlertToken)
        self.assertEqual(ast(0).brand, u'note')
        self.assertIsInstance(ast(0)(0), tokens.Word)
        self.assertEqual(ast(0)(0).content, u'content')

    def testTitle(self):
        ast = self.ast(u'!alert note title=foo\ncontent')
        self.assertIsInstance(ast(0), alert.AlertToken)
        self.assertEqual(ast(0).brand, u'note')
        self.assertIsInstance(ast(0).title, tokens.Token)
        self.assertIsInstance(ast(0).title(0), tokens.Word)
        self.assertEqual(ast(0).title(0).content, u'foo')
        self.assertIsInstance(ast(0)(0), tokens.Word)
        self.assertEqual(ast(0)(0).content, u'content')

# RENDERER TESTS
class TestRenderAlertTokenHTML(testing.MooseDocsTestCase):
    """Test renderering of RenderAlertToken with HTMLRenderer"""

    EXTENSIONS = [core, command, alert]
    RENDERER = renderers.HTMLRenderer
    TEXT = u'!alert warning\ncontent'

    def node(self):
        return self.render(self.TEXT).find('moose-content', attr='class')(0)

    def testTree(self):
        node = self.node()
        self.assertIsInstance(node, html.Tag)
        self.assertEqual(node.name, 'div')
        self.assertIn('moose-alert', node['class'])
        self.assertIn('moose-alert-warning', node['class'])

        self.assertIsInstance(node(0), html.Tag)
        self.assertEqual(node(0).name, 'div')
        self.assertIn('moose-alert-title', node(0)['class'])

        self.assertIsInstance(node(1), html.Tag)
        self.assertEqual(node(1).name, 'div')
        self.assertIn('moose-alert-content', node(1)['class'])

        self.assertIsInstance(node(0)(0), html.Tag)
        self.assertEqual(node(0)(0).name, 'span')
        self.assertIn('moose-alert-title-brand', node(0)(0)['class'])

        self.assertIsInstance(node(0)(0)(0), html.String)
        self.assertEqual(node(0)(0)(0).content, u'warning')

    def testWrite(self):
        node = self.node()
        gold = node.write()
        self.assertIn('<div class="moose-alert moose-alert-warning">', gold)
        self.assertIn('<div class="moose-alert-title">', gold)
        self.assertIn('<span class="moose-alert-title-brand">warning</span>', gold)

class TestRenderAlertTokenMaterialize(TestRenderAlertTokenHTML):
    """Test renderering of RenderAlertToken with MaterializeRenderer"""

    RENDERER = renderers.MaterializeRenderer

    def testTree(self):
        node = self.node()
        self.assertIsInstance(node, html.Tag)
        self.assertEqual(node.name, 'div')
        self.assertIn('card', node['class'])
        self.assertIn('moose-alert', node['class'])
        self.assertIn('moose-alert-warning', node['class'])

        self.assertIsInstance(node(0), html.Tag)
        self.assertEqual(node(0).name, 'div')
        self.assertIn('card-content', node(0)['class'])

        self.assertIsInstance(node(0)(0), html.Tag)
        self.assertEqual(node(0)(0).name, 'div')
        self.assertIn('moose-alert-title', node(0)(0)['class'])
        self.assertIn('card-title', node(0)(0)['class'])

        self.assertIsInstance(node(0)(1), html.Tag)
        self.assertEqual(node(0)(1).name, 'div')
        self.assertIn('moose-alert-content', node(0)(1)['class'])

        self.assertIsInstance(node(0)(0)(0), html.Tag)
        self.assertEqual(node(0)(0)(0).name, 'span')
        self.assertIn('moose-alert-title-brand', node(0)(0)(0)['class'])

        self.assertIsInstance(node(0)(0)(0)(0), html.String)
        self.assertEqual(node(0)(0)(0)(0).content, u'warning')

    def testWrite(self):
        node = self.node()
        gold = node.write()
        self.assertIn('<div class="card moose-alert moose-alert-warning">', gold)
        self.assertIn('<div class="moose-alert-title card-title">', gold)
        self.assertIn('<span class="moose-alert-title-brand">warning</span>', gold)


@unittest.skip('WIP LaTeX')
class TestRenderAlertTokenLatex(testing.MooseDocsTestCase):
    """Test renderering of RenderAlertToken with LatexRenderer"""

    EXTENSIONS = [core, command, alert]
    RENDERER = renderers.LatexRenderer
    TEXT = u'TEST STRING HERE'

    def node(self):
        return self.render(self.TEXT).find('document')(0)

    def testTree(self):
        self.assertFalse(True)

    def testWrite(self):
        node = self.node()
        self.assertEqual(node.write(), "GOLD")

if __name__ == '__main__':
    unittest.main(verbosity=2)
