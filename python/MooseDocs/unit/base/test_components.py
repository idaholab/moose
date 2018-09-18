#!/usr/bin/env python2
"""
Tests for Component objects.
"""
import unittest
import mock

from MooseDocs.tree import tokens, page
from MooseDocs.common import exceptions
from MooseDocs.base.components import Component, TokenComponent, RenderComponent, Extension
from MooseDocs.base.lexers import RecursiveLexer, LexerInformation
from MooseDocs.base import Translator, Reader, Renderer

class TestExtension(unittest.TestCase):
    """
    Test the Extension class.
    """
    def testExtend(self):
        """
        Test the extend method.
        """
        class ExtTester(Extension):
            """Dummy extension for testing."""
            def __init__(self, *args, **kwargs):
                Extension.__init__(self, *args, **kwargs)
                self.called = False
            def extend(self, reader, renderer):
                self.called = True

        ext = ExtTester()
        self.assertFalse(ext.called)
        content = page.PageNodeBase(None)
        t = Translator(content, Reader(RecursiveLexer('foo')), Renderer(), [ext])
        t.init()
        self.assertTrue(ext.called)
        self.assertIs(ext.translator, t)

    def testRequires(self):
        pass #TODO: add a test

class TestComponent(unittest.TestCase):
    """
    Test Component base class.
    """
    def test(self):
        """
        Test basic construction and errors.
        """
        comp = Component()

        with self.assertRaises(exceptions.MooseDocsException) as e:
            comp.translator
        self.assertIn("The init() method of", e.exception.message)

        with self.assertRaises(exceptions.MooseDocsException) as e:
            comp.init(42)
        self.assertIn("The argument 'translator' must be of type", e.exception.message)

        content = page.PageNodeBase(None)
        t = Translator(content, Reader(RecursiveLexer('foo')), Renderer(), [])
        comp.init(t)
        with self.assertRaises(exceptions.MooseDocsException) as e:
            comp.init(t)
        self.assertIn("already been", e.exception.message)

        self.assertIs(comp.translator, t)

class TestTokenComponent(unittest.TestCase):
    """
    Test TokenComponent.
    """
    def testDefault(self):
        """
        Test basic construction and errors.
        """
        comp = TokenComponent()
        reader = Reader(RecursiveLexer('foo'))
        content = page.PageNodeBase(None)
        t = Translator(content, reader, Renderer(), [])
        comp.init(t)

        self.assertIsNone(comp.settings)
        self.assertIs(comp.reader, reader)
        self.assertEqual(comp.attributes, dict())

        defaults = comp.defaultSettings()
        for key in ['id', 'class', 'style']:
            self.assertIn(key, defaults)
            self.assertIsInstance(defaults[key], tuple)
            self.assertEqual(len(defaults[key]), 2)
            self.assertEqual(defaults[key][0], u'')
            self.assertIsInstance(defaults[key][1], unicode)

    def testExceptions(self):
        """
        Test that exceptions are raised.
        """
        comp = TokenComponent()
        reader = Reader(RecursiveLexer('foo'))
        content = page.PageNodeBase(None)
        t = Translator(content, reader, Renderer(), [])
        comp.init(t)

        with self.assertRaises(NotImplementedError):
            comp.createToken([], [])

        # Test defaultSettings return type check
        class TestToken(TokenComponent):
            @staticmethod
            def defaultSettings():
                pass

        with self.assertRaises(exceptions.MooseDocsException) as e:
            TestToken()
        self.assertIn("must return a dict", e.exception.message)

    def testCreateToken(self):
        """
        Test the createToken method is called.
        """
        class TestToken(TokenComponent):
            PARSE_SETTINGS = False
            def createToken(self, *args):
                self.count = 1

        info = mock.Mock(spec=LexerInformation)
        parent = tokens.Token()
        comp = TestToken()
        comp(info, parent)
        self.assertEqual(comp.count, 1)

class TestRendererComponent(unittest.TestCase):
    """
    Basic test for RenderComponent.
    """
    def test(self):
        comp = RenderComponent()
        renderer = Renderer()
        content = page.PageNodeBase(None)
        t = Translator(content, Reader(RecursiveLexer('foo')), renderer, [])
        comp.init(t)
        self.assertEqual(comp.renderer, renderer)

if __name__ == '__main__':
    unittest.main(verbosity=2)
