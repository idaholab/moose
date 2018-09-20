#!/usr/bin/env python2
#pylint: disable=missing-docstring
import os
import unittest
import mock
import tempfile

from MooseDocs.extensions import command, core, listing, floats
from MooseDocs.tree import tokens, page
from MooseDocs.base import testing

# TOKEN OBJECTS TESTS
class TestTokens(unittest.TestCase):
    """Test Token object for MooseDocs.extensions.command MooseDocs extension."""

# TOKENIZE TESTS
class TestInlineCommandTokenize(testing.MooseDocsTestCase):
    """Test tokenization of InlineCommand"""
    EXTENSIONS = [core, command, listing, floats]

    def testToken(self):
        ast = self.ast(u'!listing\nfoo')
        self.assertIsInstance(ast(0), floats.Float)
        self.assertIsInstance(ast(0)(0), tokens.Code)
        self.assertEqual(ast(0)(0).code, u'foo')

    def testTokenWithSettings(self):
        ast = self.ast(u'!listing prefix=bar id=foo-bar\nfoo')
        self.assertIsInstance(ast(0), floats.Float)
        self.assertIsInstance(ast(0)(0), floats.Caption)
        self.assertEqual(ast(0)(0).prefix, u'bar')
        self.assertIsInstance(ast(0)(1), tokens.Code)
        self.assertEqual(ast(0)(1).code, u'foo')

    def testTokenWithSettingsMultiline(self):
        ast = self.ast(u'!listing prefix=bar id=foo-bar\nfoo')
        self.assertIsInstance(ast(0), floats.Float)
        self.assertIsInstance(ast(0)(0), floats.Caption)
        self.assertEqual(ast(0)(0).prefix, u'bar')
        self.assertIsInstance(ast(0)(1), tokens.Code)
        self.assertEqual(ast(0)(1).code, u'foo')


class TestBlockCommandTokenize(testing.MooseDocsTestCase):
    """Test tokenization of BlockCommand"""

    EXTENSIONS = [core, command, listing, floats]

    def testToken(self):
        ast = self.ast(u'!listing!\nfoo\n!listing-end!')
        self.assertIsInstance(ast(0), floats.Float)
        self.assertIsInstance(ast(0)(0), tokens.Code)
        self.assertEqual(ast(0)(0).code, u'foo\n')

    def testTokenWithSettings(self):
        ast = self.ast(u'!listing! prefix=bar id=foo-bar\nfoo\n!listing-end!')
        self.assertIsInstance(ast(0), floats.Float)
        self.assertIsInstance(ast(0)(0), floats.Caption)
        self.assertEqual(ast(0)(0).prefix, u'bar')
        self.assertIsInstance(ast(0)(1), tokens.Code)
        self.assertEqual(ast(0)(1).code, u'foo\n')

    def testTokenWithSettingsMultiline(self):
        ast = self.ast(u'!listing! prefix=bar\n id=foo-bar\nfoo\n!listing-end!')
        self.assertIsInstance(ast(0), floats.Float)
        self.assertIsInstance(ast(0)(0), floats.Caption)
        self.assertEqual(ast(0)(0).prefix, u'bar')
        self.assertIsInstance(ast(0)(1), tokens.Code)
        self.assertEqual(ast(0)(1).code, u'foo\n')

class TestCommandBaseTokenize(testing.MooseDocsTestCase):
    """Test tokenization of CommandBase"""
    pass # base class

class TestCommandComponentTokenize(testing.MooseDocsTestCase):
    """Test tokenization of CommandComponent"""
    pass # base class

class TestErrorHandling(testing.MooseDocsTestCase):
    EXTENSIONS = [core, command]

    def content(self):
        return u'This is some text that contains a command\n\n!unkown command\n, it should error ' \
               u'during tokenize.\n\n!this too\n\n'

    @mock.patch('logging.Logger.error')
    def testFromText(self, mock):
        self.ast(self.content())

        self.assertEqual(mock.call_count, 2)

        calls = mock.call_args_list
        args, kwargs = calls[0]
        msg = '\n'.join(args)
        self.assertIn("The following command combination is unknown: 'unkown command'", msg)

        args, kwargs = calls[1]
        msg = '\n'.join(args)
        self.assertIn("The following command combination is unknown: 'this too'", msg)

    @mock.patch('logging.Logger.error')
    def testFromFile(self, mock):
        filename = tempfile.mkstemp('.md')[-1]
        with open(filename, 'w') as fid:
            fid.write(self.content())

        node = page.MarkdownNode(None, source=filename)
        node.read()
        self.ast(node.content)

        self.assertEqual(mock.call_count, 2)

        calls = mock.call_args_list
        args, kwargs = calls[0]
        msg = '\n'.join(args)
        self.assertIn("The following command combination is unknown: 'unkown command'", msg)

        args, kwargs = calls[1]
        msg = '\n'.join(args)
        self.assertIn("The following command combination is unknown: 'this too'", msg)

        if os.path.exists(filename):
            os.remove(filename)

if __name__ == '__main__':
    unittest.main(verbosity=2)
