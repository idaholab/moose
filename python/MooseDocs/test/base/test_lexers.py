#!/usr/bin/env python
"""
Tests for Lexer and related objects.
"""
import re
import unittest
import mock

from MooseDocs.tree import tokens
from MooseDocs.base import lexers
from MooseDocs.common import exceptions

class Proxy(object):
    """
    Proxy class for Components.
    """
    def __call__(self, *args):
        pass

class TestGrammer(unittest.TestCase):
    """
    Test Grammer object.
    """
    def testGrammer(self):
        grammer = lexers.Grammer()

        with self.assertRaises(exceptions.MooseDocsException) as e:
            grammer.add(1, [], [], '_end')
        self.assertIn("'name' must be of type", e.exception.message)

        with self.assertRaises(exceptions.MooseDocsException) as e:
            grammer.add('foo', 1, [], '_end')
        self.assertIn("'regex' must be of type", e.exception.message)

        with self.assertRaises(exceptions.MooseDocsException) as e:
            grammer.add('foo', re.compile(''), 1, '_end')
        self.assertIn("'function' must be callable", e.exception.message)

        with self.assertRaises(exceptions.MooseDocsException) as e:
            grammer.add('foo', re.compile(''), Proxy(), [])
        self.assertIn("'location' must be of type", e.exception.message)

    def testPatterns(self):
        """
        Test the multiple patterns can be added.

        NOTE: The underlying Storage object that the Grammer class uses is thoroughly tested
              in the test/common/test_Storage.py.
        """
        grammer = lexers.Grammer()
        grammer.add('foo', re.compile(''), Proxy())
        grammer.add('bar', re.compile(''), Proxy())
        self.assertEqual(grammer[0].name, 'foo')
        self.assertEqual(grammer[1].name, 'bar')
        self.assertEqual(grammer['foo'].name, 'foo')
        self.assertEqual(grammer['bar'].name, 'bar')

class TestLexerInformation(unittest.TestCase):
    """
    Test LexerInformation class that stores parsing data.
    """
    def testInfo(self):
        regex = re.compile(r'(?P<key>foo)')
        match = regex.search('foo bar')

        pattern = lexers.Pattern(name='name', regex=regex, function=Proxy())
        info = lexers.LexerInformation(match=match, pattern=pattern, line=42)
        self.assertEqual(info.line, 42)
        self.assertEqual(info.pattern, 'name')
        self.assertEqual(info.keys(), [0, 1, 'key'])
        self.assertIn('key', info)
        self.assertIn('line:42', str(info))

class FooBar(tokens.Word):
    """Token class for testing lexer."""
    pass

class FooBarComponent(object):
    """Class for testing lexer."""
    def __call__(self, info, parent):
        content = info['content']
        if content in (u'foo', u'bar'):
            return FooBar(parent, content=content)

class WordComponent(object):
    """Class for testing lexer."""
    def __call__(self, info, parent):
        return tokens.Word(parent, content=info['content'])

class TestLexer(unittest.TestCase):
    """
    Test basic operation of Lexer class.
    """
    def testTokenize(self):
        root = tokens.Token(None)
        grammer = lexers.Grammer()
        grammer.add('foo', re.compile('(?P<content>\w+) *'), FooBarComponent())
        grammer.add('word', re.compile('(?P<content>\w+) *'), WordComponent())

        lexer = lexers.Lexer()

        # Basic
        lexer.tokenize(root, grammer, u'foo bar')
        self.assertIsInstance(root(0), FooBar)
        self.assertEqual(root(0).content, u'foo')
        self.assertIsInstance(root(1), FooBar)
        self.assertEqual(root(1).content, u'bar')

        # Fall through
        root = tokens.Token(None)
        lexer.tokenize(root, grammer, u'foo other bar')
        self.assertIsInstance(root(0), FooBar)
        self.assertEqual(root(0).content, u'foo')
        self.assertIsInstance(root(1), tokens.Word)
        self.assertNotIsInstance(root(1), FooBar)
        self.assertEqual(root(1).content, u'other')
        self.assertIsInstance(root(2), FooBar)
        self.assertEqual(root(2).content, u'bar')

    @mock.patch('logging.Logger.error')
    def testTokenizeWithExtraContent(self, mock):
        # Extra
        root = tokens.Token(None)
        grammer = lexers.Grammer()
        grammer.add('foo', re.compile('(?P<content>\w+) *'), FooBarComponent())

        lexer = lexers.Lexer()
        lexer.tokenize(root, grammer, u'foo ???')
        self.assertIsInstance(root(0), FooBar)
        self.assertEqual(root(0).content, u'foo')
        self.assertEqual(mock.call_count, 1)


class EmptyComponent(object):
    """
    Class for testing RecursiveLexer.
    """
    def __call__(self, info, parent):
        return tokens.Token(parent)

class TestRecursiveLexer(unittest.TestCase):
    """
    Test operation of RecursiveLexer class.
    """
    def testTokenize(self):
        lexer = lexers.RecursiveLexer('block', 'inline')
        lexer.add('block', 'foo', re.compile('(?P<inline>\w+) *'), EmptyComponent())
        lexer.add('inline', 'bar', re.compile('(?P<content>\w)'), WordComponent())

        root = tokens.Token(None)
        lexer.tokenize(root, lexer.grammer(), u'foo')
        self.assertIsInstance(root(0), tokens.Token)
        self.assertNotIsInstance(root(0), tokens.Word)
        self.assertIsInstance(root(0)(0), tokens.Word)
        self.assertEqual(root(0)(0).content, u'f')
        self.assertIsInstance(root(0)(1), tokens.Word)
        self.assertEqual(root(0)(1).content, u'o')
        self.assertIsInstance(root(0)(2), tokens.Word)
        self.assertEqual(root(0)(2).content, u'o')

if __name__ == '__main__':
    unittest.main(verbosity=2)
