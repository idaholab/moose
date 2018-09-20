#!/usr/bin/env python2
#pylint: disable=missing-docstring

import unittest
import logging

from MooseDocs.extensions import core, command, floats, media
from MooseDocs.tree import html
from MooseDocs.base import testing, renderers

logging.basicConfig()

# TOKEN OBJECTS TESTS
class TestTokens(unittest.TestCase):
    """Test Token object for MooseDocs.extensions.media MooseDocs extension."""

    def testImage(self):
        tok = media.Image(src=u'foo')
        self.assertEqual(tok.src, u'foo')

    def testVideo(self):
        tok = media.Video(src=u'foo')
        self.assertEqual(tok.src, u'foo')
        self.assertTrue(tok.controls)
        self.assertTrue(tok.autoplay)
        self.assertTrue(tok.loop)

        tok = media.Video(src=u'foo', controls=False, autoplay=False, loop=False)
        self.assertEqual(tok.src, u'foo')
        self.assertFalse(tok.controls)
        self.assertFalse(tok.autoplay)
        self.assertFalse(tok.loop)

# TOKENIZE TESTS
class TestMediaCommandBaseTokenize(testing.MooseDocsTestCase):
    """Test tokenization of MediaCommandBase"""
    # Base class, no test needed

class TestVideoCommandTokenize(testing.MooseDocsTestCase):
    """Test tokenization of VideoCommand"""

    EXTENSIONS = [core, command, floats, media]

    def testToken(self):
        ast = self.ast(u'!media http://link_to_movie.webm id=foo')
        self.assertIsInstance(ast(0), floats.Float)
        self.assertIsInstance(ast(0)(0), media.Video)
        self.assertEqual(ast(0)(0).src, 'http://link_to_movie.webm')

    def testTokenNoFrame(self):
        ast = self.ast(u'!media http://link_to_movie.webm')
        self.assertNotIsInstance(ast(0), floats.Float)
        self.assertIsInstance(ast(0), media.Video)
        self.assertEqual(ast(0).src, 'http://link_to_movie.webm')


class TestImageCommandTokenize(testing.MooseDocsTestCase):
    """Test tokenization of ImageCommand"""

    EXTENSIONS = [core, command, floats, media]

    def testToken(self):
        ast = self.ast(u'!media inl_blue.png id=foo')
        self.assertIsInstance(ast(0), floats.Float)
        self.assertIsInstance(ast(0)(0), media.Image)
        self.assertEqual(ast(0)(0).src, 'inl_blue.png')

# RENDERER TESTS
class TestRenderImageHTML(testing.MooseDocsTestCase):
    """Test renderering of RenderImage with HTMLRenderer"""

    EXTENSIONS = [core, command, floats, media]
    RENDERER = renderers.HTMLRenderer
    TEXT = u'!media inl_blue.png id=foo'

    def node(self):
        return self.render(self.TEXT).find('moose-content', attr='class')(0)

    def testTree(self):
        node = self.node()
        self.assertIsInstance(node, html.Tag)
        self.assertEqual(node.name, 'div')
        self.assertIn('moose-float-div', node['class'])
        self.assertIsInstance(node(0), html.Tag)
        self.assertEqual(node(0).name, 'img')
        self.assertEqual(node(0)['src'], 'inl_blue.png')

    def testWrite(self):
        node = self.node()
        content = node.write()
        self.assertIn('class="moose-float-div">', content)
        self.assertIn('<img src="inl_blue.png"></img>', content)

class TestRenderImageMaterialize(TestRenderImageHTML):
    """Test renderering of RenderImage with MaterializeRenderer"""

    RENDERER = renderers.MaterializeRenderer

    def testTree(self):
        node = self.node()
        self.assertIsInstance(node, html.Tag)
        self.assertEqual(node.name, 'div')
        self.assertIn('card', node['class'])
        self.assertIsInstance(node(0), html.Tag)
        self.assertEqual(node(0).name, 'div')
        self.assertIn('card-image', node(0)['class'])
        self.assertIsInstance(node(0)(0), html.Tag)
        self.assertEqual(node(0)(0).name, 'img')
        self.assertEqual(node(0)(0)['src'], 'inl_blue.png')

    def testWrite(self):
        node = self.node()
        content = node.write()
        self.assertIn('class="card">', content)
        self.assertIn('<div class="card-image">', content)
        self.assertIn('<img src="inl_blue.png"', content)

@unittest.skip('LaTeX WIP')
class TestRenderImageLatex(testing.MooseDocsTestCase):
    """Test renderering of RenderImage with LatexRenderer"""

    EXTENSIONS = [core, command, floats, media]
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

class TestRenderVideoHTML(testing.MooseDocsTestCase):
    """Test renderering of RenderVideo with HTMLRenderer"""

    EXTENSIONS = [core, command, floats, media]
    RENDERER = renderers.HTMLRenderer
    TEXT = u'!media http://foo.webm loop=True autoplay=True id=foo'

    def node(self):
        return self.render(self.TEXT).find('moose-content', attr='class')(0)

    def testTree(self):
        node = self.node()
        self.assertIsInstance(node, html.Tag)
        self.assertEqual(node.name, 'div')
        self.assertIn('moose-float-div', node['class'])
        self.assertIsInstance(node(0), html.Tag)
        self.assertEqual(node(0).name, 'video')
        self.assertEqual(node(0)['controls'], 'controls')
        self.assertEqual(node(0)['loop'], 'loop')
        self.assertEqual(node(0)['autoplay'], 'autoplay')
        self.assertIsInstance(node(0)(0), html.Tag)
        self.assertEqual(node(0)(0).name, 'source')
        self.assertEqual(node(0)(0)['src'], 'http://foo.webm')

    def testWrite(self):
        node = self.node()
        content = node.write()
        self.assertIn('class="moose-float-div"', content)
        self.assertIn('<video width="100%" loop="loop" controls="controls"', content)
        self.assertIn('<source src="http://foo.webm', content)

class TestRenderVideoMaterialize(TestRenderVideoHTML):
    """Test renderering of RenderVideo with MaterializeRenderer"""

    RENDERER = renderers.MaterializeRenderer

    def testTree(self):
        node = self.node()(0)
        self.assertIsInstance(node, html.Tag)
        self.assertEqual(node.name, 'div')
        self.assertIn('card', node['class'])
        self.assertIsInstance(node(0), html.Tag)
        self.assertEqual(node(0).name, 'video')
        self.assertEqual(node(0)['controls'], 'controls')
        self.assertEqual(node(0)['loop'], 'loop')
        self.assertEqual(node(0)['autoplay'], 'autoplay')
        self.assertIsInstance(node(0)(0), html.Tag)
        self.assertEqual(node(0)(0).name, 'source')
        self.assertEqual(node(0)(0)['src'], 'http://foo.webm')

    def testWrite(self):
        node = self.node()
        content = node.write()
        self.assertIn('class="card"', content)
        self.assertIn('<div class="card-image">', content)
        self.assertIn('<source src="http://foo.webm', content)

@unittest.skip('LaTeX WIP')
class TestRenderVideoLatex(testing.MooseDocsTestCase):
    """Test renderering of RenderVideo with LatexRenderer"""

    EXTENSIONS = [core, command, floats, media]
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

if __name__ == '__main__':
    unittest.main(verbosity=2)
