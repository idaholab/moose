#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

"""Test for the Reader objects."""
import unittest
import re
import logging
logging.basicConfig()#level=logging.CRITICAL)

import MooseDocs
from MooseDocs.common import exceptions
from MooseDocs.tree import tokens, pages
from MooseDocs.base import readers, lexers, components

Word = tokens.newToken('Word')
class WordComponent(components.TokenComponent):
    RE = re.compile('(?P<inline>\w+)\s*')
    def createToken(self, parent, info, page):
        if info['inline'] == 'throw':
            raise Exception("testing")
        return Word(parent)

Letter = tokens.newToken('Letter', content=u'')
class LetterComponent(components.TokenComponent):
    RE = re.compile('(?P<content>\w)')
    def createToken(self, parent, info, page):
        return Letter(parent, content=info['content'])

class TestReader(unittest.TestCase):

    def testParse(self):
        root = tokens.Token(None)
        reader = readers.Reader(lexers.RecursiveLexer('block', 'inline'))
        reader.add('block', WordComponent())
        reader.add('inline', LetterComponent())
        reader.tokenize(root, u'foo bar', None)
        self.assertEqual(root(0).name, 'Word')
        self.assertEqual(root(0)(0).name, 'Letter')
        self.assertEqual(root(0)(1).name, 'Letter')
        self.assertEqual(root(0)(2).name, 'Letter')
        self.assertEqual(root(0)(0)['content'], 'f')
        self.assertEqual(root(0)(1)['content'], 'o')
        self.assertEqual(root(0)(2)['content'], 'o')

        self.assertEqual(root(1).name, 'Word')
        self.assertEqual(root(1)(0).name, 'Letter')
        self.assertEqual(root(1)(1).name, 'Letter')
        self.assertEqual(root(1)(2).name, 'Letter')
        self.assertEqual(root(1)(0)['content'], 'b')
        self.assertEqual(root(1)(1)['content'], 'a')
        self.assertEqual(root(1)(2)['content'], 'r')

    def testParseExceptions(self):
        MooseDocs.LOG_LEVEL = logging.DEBUG
        reader = readers.Reader(lexers.RecursiveLexer('foo'))
        with self.assertRaises(exceptions.MooseDocsException) as e:
            reader.tokenize([], u'', None)
        self.assertIn("The argument 'root'", str(e.exception))

        with self.assertRaises(exceptions.MooseDocsException) as e:
            reader.tokenize(tokens.Token(), [], None)
        self.assertIn("The argument 'content'", str(e.exception))
        MooseDocs.LOG_LEVEL = logging.INFO

    def testAddExceptions(self):
        MooseDocs.LOG_LEVEL = logging.DEBUG
        reader = readers.Reader(lexers.RecursiveLexer('foo'))

        with self.assertRaises(exceptions.MooseDocsException) as e:
            reader.add([], u'', '')
        self.assertIn("The argument 'group'", str(e.exception))

        with self.assertRaises(exceptions.MooseDocsException) as e:
            reader.add('foo', u'', '')
        self.assertIn("The argument 'component'", str(e.exception))

        with self.assertRaises(exceptions.MooseDocsException) as e:
            reader.add('foo', components.TokenComponent(), [])
        self.assertIn("The argument 'location'", str(e.exception))
        MooseDocs.LOG_LEVEL = logging.INFO

    def testTokenizeException(self):
        root = tokens.Token(None)
        reader = readers.Reader(lexers.RecursiveLexer('block', 'inline'))
        reader.add('block', WordComponent())
        reader.add('inline', LetterComponent())
        reader.tokenize(root, u'throw bar', pages.Page('foo', source='foo'))
        self.assertEqual(root(0).name, 'ErrorToken')
        self.assertEqual(root(1).name, 'Word')
        self.assertEqual(root(1)(0).name, 'Letter')
        self.assertEqual(root(1)(1).name, 'Letter')
        self.assertEqual(root(1)(2).name, 'Letter')
        self.assertEqual(root(1)(0)['content'], u'b')
        self.assertEqual(root(1)(1)['content'], u'a')
        self.assertEqual(root(1)(2)['content'], u'r')

class TestMarkdownReader(unittest.TestCase):
    def testBasic(self):
        root = tokens.Token(None)
        reader = readers.MarkdownReader()
        reader.add('block', WordComponent())
        reader.add('inline', LetterComponent())
        reader.tokenize(root, u'foo bar', pages.Page('foo', source='foo'))

        self.assertEqual(root(0).name, 'Word')
        self.assertEqual(root(0)(0).name, 'Letter')
        self.assertEqual(root(0)(1).name, 'Letter')
        self.assertEqual(root(0)(2).name, 'Letter')
        self.assertEqual(root(0)(0)['content'], 'f')
        self.assertEqual(root(0)(1)['content'], 'o')
        self.assertEqual(root(0)(2)['content'], 'o')

        self.assertEqual(root(1).name, 'Word')
        self.assertEqual(root(1)(0).name, 'Letter')
        self.assertEqual(root(1)(1).name, 'Letter')
        self.assertEqual(root(1)(2).name, 'Letter')
        self.assertEqual(root(1)(0)['content'], 'b')
        self.assertEqual(root(1)(1)['content'], 'a')
        self.assertEqual(root(1)(2)['content'], 'r')

if __name__ == '__main__':
    unittest.main(verbosity=2)
