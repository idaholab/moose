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
class WordComponent(components.ReaderComponent):
    RE = re.compile('(?P<inline>\w+)\s*')
    def createToken(self, parent, info, page, settings):
        if info['inline'] == 'throw':
            raise Exception("testing")
        return Word(parent)

Letter = tokens.newToken('Letter', content='')
class LetterComponent(components.ReaderComponent):
    RE = re.compile('(?P<content>\w)')
    def createToken(self, parent, info, page, settings):
        return Letter(parent, content=info['content'])

class TestReader(unittest.TestCase):

    def testParse(self):
        root = tokens.Token(None)
        reader = readers.Reader(lexers.RecursiveLexer('block', 'inline'))
        reader.add('block', WordComponent())
        reader.add('inline', LetterComponent())
        reader.tokenize(root, 'foo bar', None)
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

    def testTokenizeException(self):
        root = tokens.Token(None)
        reader = readers.Reader(lexers.RecursiveLexer('block', 'inline'))
        reader.add('block', WordComponent())
        reader.add('inline', LetterComponent())
        reader.tokenize(root, 'throw bar', pages.Page('foo', source='foo'))
        self.assertEqual(root(0).name, 'ErrorToken')
        self.assertEqual(root(1).name, 'Word')
        self.assertEqual(root(1)(0).name, 'Letter')
        self.assertEqual(root(1)(1).name, 'Letter')
        self.assertEqual(root(1)(2).name, 'Letter')
        self.assertEqual(root(1)(0)['content'], 'b')
        self.assertEqual(root(1)(1)['content'], 'a')
        self.assertEqual(root(1)(2)['content'], 'r')

class TestMarkdownReader(unittest.TestCase):
    def testBasic(self):
        root = tokens.Token(None)
        reader = readers.MarkdownReader()
        reader.add('block', WordComponent())
        reader.add('inline', LetterComponent())
        reader.tokenize(root, 'foo bar', pages.Page('foo', source='foo'))

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
