#!/usr/bin/env python2
#pylint: disable=missing-docstring

import unittest

from MooseDocs.extensions import core, floats
from MooseDocs.tree import tokens, html
from MooseDocs.base import testing, renderers

# TOKEN OBJECTS TESTS
class TestTokens(unittest.TestCase):
    """Test Token object for MooseDocs.extensions.floats MooseDocs extension."""

    def testCaption(self):
        token = floats.Caption(None, prefix=u'caption', key=u'foo')
        self.assertIsInstance(token(0), tokens.Shortcut)
        self.assertEqual(token.key, u'foo')
        self.assertEqual(token.number, 1)

        token2 = floats.Caption(None, prefix=u'caption', key=u'foo')
        self.assertEqual(token2.number, 2)

    def testFloat(self):
        token = floats.Float(None)
        self.assertIsInstance(token, floats.Float)

    def testModalLink(self):
        token = floats.ModalLink(None, url=u'foo', bottom=True,
                                 content=tokens.String(None, content=u''),
                                 title=tokens.String(None, content=u''))
        self.assertIsInstance(token.title, tokens.Token)
        self.assertTrue(token.bottom)

# RENDERER TESTS
class TestRenderCaptionHTML(testing.MooseDocsTestCase):
    """Test renderering of RenderCaption with HTMLRenderer"""

    EXTENSIONS = [core, floats]
    RENDERER = renderers.HTMLRenderer

    def node(self, **kwargs):
        node = html.Tag(None, 'div')
        ast = floats.Caption(None, **kwargs)
        self._translator.renderer.process(node, ast)
        return node

    def testTree(self):
        node = self.node()
        self.assertIsInstance(node(0), html.Tag)
        self.assertEqual(node(0).name, 'p')
        self.assertIn('moose-caption', node(0)['class'])
        self.assertIsInstance(node(0), html.Tag)
        self.assertEqual(node(0)(0).name, 'span')
        self.assertIn('moose-caption-text', node(0)(0)['class'])

        node = self.node(prefix=u'Foo')
        self.assertEqual(node(0)(0).name, 'span')
        self.assertIn('moose-caption-heading', node(0)(0)['class'])
        self.assertEqual(node(0)(1).name, 'span')
        self.assertIn('moose-caption-text', node(0)(1)['class'])

    def testWrite(self):
        node = self.node()
        self.assertEqual(node(0).write(),
                         '<p class="moose-caption"><span class="moose-caption-text"></span></p>')

class TestRenderCaptionMaterialize(TestRenderCaptionHTML):
    """Test renderering of RenderCaption with MaterializeRenderer"""

    RENDERER = renderers.MaterializeRenderer

@unittest.skip('WIP LaTeX')
class TestRenderCaptionLatex(testing.MooseDocsTestCase):
    """Test renderering of RenderCaption with LatexRenderer"""

    EXTENSIONS = [core, floats]
    RENDERER = renderers.LatexRenderer

    def node(self):
        return self.render(self.TEXT).find('document')(0)

    def testTree(self):
        #node = self.node()
        self.assertFalse(True)

    def testWrite(self):
        node = self.node()
        self.assertEqual(node.write(), "GOLD")

class TestRenderFloatHTML(testing.MooseDocsTestCase):
    """Test renderering of RenderFloat with HTMLRenderer"""

    EXTENSIONS = [core, floats]
    RENDERER = renderers.HTMLRenderer

    def node(self, **kwargs):
        node = html.Tag(None, 'div')
        ast = floats.Float(None, **kwargs)
        self._translator.renderer.process(node, ast)
        return node

    def testTree(self):
        node = self.node()
        self.assertIsInstance(node(0), html.Tag)
        self.assertEqual(node(0).name, 'div')
        self.assertIn('moose-float-div', node(0)['class'])

    def testWrite(self):
        node = self.node()
        self.assertEqual(node(0).write(), '<div class="moose-float-div"></div>')

class TestRenderFloatMaterialize(TestRenderFloatHTML):
    """Test renderering of RenderFloat with MaterializeRenderer"""

    RENDERER = renderers.MaterializeRenderer

    def testTree(self):
        node = self.node()
        self.assertIsInstance(node(0), html.Tag)
        self.assertEqual(node(0).name, 'div')
        self.assertIn('card', node(0)['class'])

        self.assertIsInstance(node(0)(0), html.Tag)
        self.assertEqual(node(0)(0).name, 'div')
        self.assertIn('card-content', node(0)(0)['class'])

    def testWrite(self):
        node = self.node()
        self.assertEqual(node(0).write(), '<div class="card"><div class="card-content"></div></div>')

@unittest.skip('WIP LaTeX')
class TestRenderFloatLatex(testing.MooseDocsTestCase):
    """Test renderering of RenderFloat with LatexRenderer"""

    EXTENSIONS = [core, floats]
    RENDERER = renderers.LatexRenderer
    TEXT = u'TEST STRING HERE'

    def node(self):
        return self.render(self.TEXT).find('document')(0)

    def testTree(self):
        #node = self.node()
        self.assertFalse(True)

    def testWrite(self):
        node = self.node()
        self.assertEqual(node.write(), "GOLD")

class TestRenderModalLinkHTML(testing.MooseDocsTestCase):
    """Test renderering of RenderModal with HTMLRenderer"""

    EXTENSIONS = [core, floats]
    RENDERER = renderers.HTMLRenderer

    def node(self, **kwargs):
        root = html.Tag(None, 'body')
        ast = floats.ModalLink(None, url=u'foo',
                               string=u'link',
                               content=tokens.String(None, content=u'content'),
                               title=tokens.String(None, content=u'title'), **kwargs)
        self._translator.renderer.process(root, ast)
        return root

    def testTree(self):
        node = self.node()
        self.assertEqual(len(node.children), 1)
        self.assertIsInstance(node(0), html.Tag)
        self.assertEqual(node(0).name, 'a')
        self.assertEqual(node(0)(0).content, u'link')

    def testWrite(self):
        node = self.node()
        self.assertEqual(node.write(), '<body><a href="foo">link</a></body>')

class TestRenderModalLinkMaterialize(TestRenderModalLinkHTML):
    """Test renderering of RenderModal with MaterializeRenderer"""

    RENDERER = renderers.MaterializeRenderer

    def testTree(self):
        node = self.node()
        self.assertEqual(len(node.children), 2)
        self.assertIsInstance(node(0), html.Tag)
        self.assertEqual(node(0).name, 'a')
        self.assertTrue(node(0)['href'].startswith('#'))
        self.assertIn('modal-trigger', node(0)['class'])
        self.assertEqual(node(0)(0).content, u'link')

        self.assertIsInstance(node(1), html.Tag)
        self.assertEqual(node(1).name, 'div')
        self.assertIsNotNone(node(1)['id'])
        self.assertIn('modal', node(1)['class'])

        self.assertIsInstance(node(1)(0), html.Tag)
        self.assertEqual(node(1)(0).name, 'div')
        self.assertIn('modal-content', node(1)(0)['class'])

    def testWrite(self):
        node = self.node()
        content = node.write()
        self.assertIn('href="#', content)
        self.assertIn('modal-trigger', content)
        self.assertIn('class="modal">', content)

@unittest.skip('WIP LaTeX')
class TestRenderModalLinkLatex(testing.MooseDocsTestCase):
    """Test renderering of RenderModal with LatexRenderer"""

    EXTENSIONS = [core, floats]
    RENDERER = renderers.LatexRenderer

    def testTree(self):
        #node = self.node()
        self.assertFalse(True)

    def testWrite(self):
        node = self.node()
        self.assertEqual(node.write(), "GOLD")

if __name__ == '__main__':
    unittest.main(verbosity=2)
