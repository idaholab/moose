#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

"""
Tests for Lexer and related objects.
"""
import re
import unittest

from MooseDocs.tree import tokens
from MooseDocs.base import lexers
from MooseDocs.common import exceptions
from MooseDocs.extensions import core

class Proxy(object):
    """
    Proxy class for Components.
    """
    def __call__(self, *args):
        pass

class TestGrammar(unittest.TestCase):
    """
    Test Grammar object.
    """
    def testPatterns(self):
        """
        Test the multiple patterns can be added.

        NOTE: The underlying Storage object that the Grammar class uses is thoroughly tested
              in the test/common/test_Storage.py.
        """
        grammar = lexers.Grammar()
        grammar.add('foo', re.compile(''), Proxy())
        grammar.add('bar', re.compile(''), Proxy())
        self.assertEqual(grammar[0].name, 'foo')
        self.assertEqual(grammar[1].name, 'bar')
        self.assertEqual(grammar['foo'].name, 'foo')
        self.assertEqual(grammar['bar'].name, 'bar')

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
        self.assertEqual(list(info.keys()), [0, 1, 'key'])
        self.assertIn('key', info)
        self.assertIn('line:42', str(info))

FooBar = tokens.newToken('FooBar', content='')
class FooBarComponent(object):
    """Class for testing lexer."""
    def __call__(self, parent, info, page):
        content = info['content']
        if content in ('foo', 'bar'):
            return FooBar(parent, content=content)

class WordComponent(object):
    """Class for testing lexer."""
    def __call__(self, parent, info, page):
        return core.Word(parent, content=info['content'])

class TestLexer(unittest.TestCase):
    """
    Test basic operation of Lexer class.
    """
    def testTokenize(self):
        root = tokens.Token(None)
        grammar = lexers.Grammar()
        grammar.add('foo', re.compile('(?P<content>\w+) *'), FooBarComponent())
        grammar.add('word', re.compile('(?P<content>\w+) *'), WordComponent())

        lexer = lexers.Lexer()

        # Basic
        lexer.tokenize(root, 'foo bar', None, grammar)
        self.assertEqual(root(0).name, 'FooBar')
        self.assertEqual(root(0)['content'], 'foo')
        self.assertEqual(root(1).name, 'FooBar')
        self.assertEqual(root(1)['content'], 'bar')

        # Fall through
        root = tokens.Token(None)
        lexer.tokenize(root, 'foo other bar', None, grammar)
        self.assertEqual(root(0).name, 'FooBar')
        self.assertEqual(root(0)['content'], 'foo')
        self.assertEqual(root(1).name, 'Word')
        self.assertEqual(root(1)['content'], 'other')
        self.assertEqual(root(2).name, 'FooBar')
        self.assertEqual(root(2)['content'], 'bar')

    def testTokenizeWithExtraContent(self):
        # Extra
        root = tokens.Token(None)
        grammar = lexers.Grammar()
        grammar.add('foo', re.compile('(?P<content>\w+) *'), FooBarComponent())

        lexer = lexers.Lexer()
        lexer.tokenize(root, 'foo ???', None, grammar)
        self.assertEqual(root(0).name, 'FooBar')
        self.assertEqual(root(0)['content'], 'foo')
        self.assertEqual(root(1).name, 'ErrorToken')
        self.assertIn('Unprocessed', root(1)['message'])

Letters = tokens.newToken('Letters')
def letters_func(parent, info, page):
        return Letters(parent)

Letter = tokens.newToken('Letter', content='')
def letter_func(parent, info, page):
        return Letter(parent, content=info['content'])

class TestRecursiveLexer(unittest.TestCase):
    """
    Test operation of RecursiveLexer class.
    """
    def testTokenize(self):
        lexer = lexers.RecursiveLexer('block', 'inline')
        lexer.add('block', 'foo', re.compile('(?P<inline>\w+) *'), letters_func)
        lexer.add('inline', 'bar', re.compile('(?P<content>\w)'), letter_func)

        root = tokens.Token(None)
        lexer.tokenize(root, 'foo', None, lexer.grammar('block'))

        self.assertIsInstance(root(0), tokens.Token)
        self.assertEqual(root(0).name, 'Letters')
        self.assertEqual(root(0)(0).name, 'Letter')
        self.assertEqual(root(0)(0)['content'], 'f')
        self.assertEqual(root(0)(1).name, 'Letter')
        self.assertEqual(root(0)(1)['content'], 'o')
        self.assertEqual(root(0)(2).name, 'Letter')
        self.assertEqual(root(0)(2)['content'], 'o')

if __name__ == '__main__':
    unittest.main(verbosity=2)
