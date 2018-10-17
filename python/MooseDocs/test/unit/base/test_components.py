#!/usr/bin/env python2
"""
Tests for Component objects.
"""
import unittest
import mock

from MooseDocs.tree import tokens, pages
from MooseDocs.common import exceptions
from MooseDocs.base.components import TokenComponent, Extension
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
        content = pages.Page('foo', source='foo')
        t = Translator([content], Reader(RecursiveLexer('foo')), Renderer(), [ext])
        t.init()
        self.assertTrue(ext.called)

class TestTokenComponent(unittest.TestCase):
    """
    Test TokenComponent.
    """
    def testDefault(self):
        """
        Test basic construction and errors.
        """
        comp = TokenComponent()
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

        with self.assertRaises(NotImplementedError):
            comp.createToken(None, None, None)

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
        comp(parent, info, None)
        self.assertEqual(comp.count, 1)

if __name__ == '__main__':
    unittest.main(verbosity=2)
