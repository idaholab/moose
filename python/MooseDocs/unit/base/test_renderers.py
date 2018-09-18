#!/usr/bin/env python2
"""
Tests for the Renderer objects.
"""
import unittest
import logging
from MooseDocs.common import exceptions
from MooseDocs.tree import tokens, html, page
from MooseDocs.base import renderers, Translator, MarkdownReader
from MooseDocs.base import components

logging.basicConfig(level=logging.CRITICAL)

class ParComponent(components.RenderComponent):
    def __init__(self, *args, **kwargs):
        components.RenderComponent.__init__(self, *args, **kwargs)
        self.count = 0
    def reinit(self):
        self.count += 1
    def createHTML(self, token, parent):
        return html.Tag(parent, 'p')

class StringComponent(components.RenderComponent):
    def createHTML(self, token, parent):
        return html.String(parent, content=token.content)

class BadComponent(components.RenderComponent):
    pass

class TestRenderer(unittest.TestCase):
    def testErrors(self):
        renderer = renderers.Renderer()

        with self.assertRaises(exceptions.MooseDocsException) as e:
            renderer.add(tokens.Paragraph(), ParComponent())
        self.assertIn("The argument 'token'", e.exception.message)

        with self.assertRaises(exceptions.MooseDocsException) as e:
            renderer.add(tokens.Paragraph, 'WRONG')
        self.assertIn("The argument 'component'", e.exception.message)

        with self.assertRaises(exceptions.MooseDocsException) as e:
            renderer.add(tokens.Paragraph, ParComponent())
        self.assertIn("The Reader class of type", e.exception.message)

class TestHTMLRenderer(unittest.TestCase):
    def testMissingCompomentMethod(self):
        renderer = renderers.HTMLRenderer()

        with self.assertRaises(exceptions.MooseDocsException) as e:
            renderer.add(tokens.Paragraph, BadComponent())
        self.assertIn("does not have a createHTML method", e.exception.message)

    def testProcessWithUnlistedToken(self):
        renderer = renderers.HTMLRenderer()
        root = html.Tag(None, 'div')
        renderer.process(root, tokens.Token(None))
        self.assertEqual(len(root), 0)

    def testReinit(self):
        ast = tokens.Paragraph(None)
        comp = ParComponent()
        content = page.PageNodeBase(None)
        renderer = renderers.HTMLRenderer()
        translator = Translator(content, MarkdownReader(), renderer, [])
        translator.init()
        renderer.add(tokens.Paragraph, comp)
        renderer.render(ast)
        renderer.render(ast)
        self.assertEqual(comp.count, 2)

    def testProcess(self):
        ast = tokens.Paragraph(None)
        tokens.String(ast, content=u'foo')

        renderer = renderers.HTMLRenderer()
        renderer.add(tokens.Paragraph, ParComponent())
        renderer.add(tokens.String, StringComponent())

        root = html.Tag(None, 'div')
        renderer.process(root, ast)

        self.assertIsInstance(root(0), html.Tag)
        self.assertEqual(root(0).name, 'p')
        self.assertIsInstance(root(0)(0), html.String)
        self.assertEqual(root(0)(0).content, u'foo')

    def testRender(self):
        ast = tokens.Paragraph(None)
        tokens.String(ast, content=u'foo')
        content = page.PageNodeBase(None)
        renderer = renderers.HTMLRenderer()
        translator = Translator(content, MarkdownReader(), renderer, [])
        translator.init()

        renderer.add(tokens.Paragraph, ParComponent())
        renderer.add(tokens.String, StringComponent())

        root = renderer.render(ast)
        self.assertIsInstance(root, html.Tag)
        self.assertEqual(root.name, 'body')
        self.assertIsInstance(root(0), html.Tag)
        self.assertEqual(root(0).name, 'p')
        self.assertIsInstance(root(0)(0), html.String)
        self.assertEqual(root(0)(0).content, u'foo')

if __name__ == '__main__':
    unittest.main(verbosity=2)
