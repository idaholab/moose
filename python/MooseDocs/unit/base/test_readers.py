#!/usr/bin/env python2
"""
Test for the Reader objects.
"""
import unittest
import re
import logging
logging.basicConfig(level=logging.CRITICAL)

import MooseDocs
from MooseDocs.common import exceptions
from MooseDocs.tree import tokens, page
from MooseDocs.base import readers, lexers, components, Translator, HTMLRenderer

class BlockComponent(components.TokenComponent):
    """Class for testing MarkdownReader"""
    RE = re.compile('(?P<inline>.*)')
    def __call__(self, info, parent):
        return tokens.Token(parent)

class WordComponent(components.TokenComponent):
    """Class for testing lexer."""
    RE = re.compile('(?P<content>\w+) *')
    def __call__(self, info, parent):
        content = info['content']
        if content == 'throw':
            raise exceptions.TokenizeException("testing")
        return tokens.Word(parent, content=content)

class TestReader(unittest.TestCase):
    def testConstruction(self):
        lexer = lexers.RecursiveLexer('foo')
        reader = readers.Reader(lexer)
        self.assertIs(reader.lexer, lexer)

    def testParse(self):
        root = tokens.Token(None)
        content = page.PageNodeBase(None)
        reader = readers.Reader(lexers.RecursiveLexer('foo'))
        translator = Translator(content, reader, HTMLRenderer(), [])
        translator.init()
        reader.add('foo', WordComponent())
        reader.parse(root, u'foo bar')
        self.assertIsInstance(root(0), tokens.Word)
        self.assertEqual(root(0).content, u'foo')
        self.assertIsInstance(root(1), tokens.Word)
        self.assertEqual(root(1).content, u'bar')

    def testParseExceptions(self):
        MooseDocs.LOG_LEVEL = logging.DEBUG
        reader = readers.Reader(lexers.RecursiveLexer('foo'))
        with self.assertRaises(exceptions.MooseDocsException) as e:
            reader.parse([], u'')
        self.assertIn("The argument 'root'", e.exception.message)

        with self.assertRaises(exceptions.MooseDocsException) as e:
            reader.parse(tokens.Token(), [])
        self.assertIn("The argument 'content'", e.exception.message)

    def testAddExceptions(self):
        MooseDocs.LOG_LEVEL = logging.DEBUG
        reader = readers.Reader(lexers.RecursiveLexer('foo'))

        with self.assertRaises(exceptions.MooseDocsException) as e:
            reader.add([], u'', '')
        self.assertIn("The argument 'group'", e.exception.message)

        with self.assertRaises(exceptions.MooseDocsException) as e:
            reader.add('foo', u'', '')
        self.assertIn("The argument 'component'", e.exception.message)

        with self.assertRaises(exceptions.MooseDocsException) as e:
            reader.add('foo', components.TokenComponent(), [])
        self.assertIn("The argument 'location'", e.exception.message)

    def testTokenizeException(self):
        root = tokens.Token(None)
        content = page.PageNodeBase(None)
        reader = readers.Reader(lexers.RecursiveLexer('foo'))
        translator = Translator(content, reader, HTMLRenderer(), [])
        translator.init()
        reader.add('foo', WordComponent())
        reader.parse(root, u'throw bar')
        self.assertIsInstance(root(0), tokens.ExceptionToken)
        self.assertIsInstance(root(1), tokens.Word)
        self.assertEqual(root(1).content, u'bar')

class TestMarkdownReader(unittest.TestCase):
    def testBasic(self):
        root = tokens.Token(None)
        content = page.PageNodeBase(None)
        reader = readers.Reader(lexers.RecursiveLexer('block', 'inline'))
        translator = Translator(content, reader, HTMLRenderer(), [])
        translator.init()
        reader.add('block', BlockComponent())
        reader.add('inline', WordComponent())
        reader.parse(root, u'foo bar')

        self.assertIsInstance(root(0), tokens.Token)
        self.assertIsInstance(root(0)(0), tokens.Word)
        self.assertEqual(root(0)(0).content, u'foo')
        self.assertIsInstance(root(0)(1), tokens.Word)
        self.assertEqual(root(0)(1).content, u'bar')

if __name__ == '__main__':
    unittest.main(verbosity=2)
