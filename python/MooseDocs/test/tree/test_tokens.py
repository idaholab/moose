#!/usr/bin/env python2
import unittest
import re
import inspect
import mock

from MooseDocs.common import exceptions
from MooseDocs.tree import tokens
from MooseDocs.base import lexers

class TestTokens(unittest.TestCase):

    def testCoverage(self):
        status = []
        msg = "The following classes in tokens module do not have a required test.\n"
        for name, obj in inspect.getmembers(tokens):
            if inspect.isclass(obj) and tokens.Token in inspect.getmro(obj):
                status.append(hasattr(self, 'test' + obj.__name__))
                if not status[-1]:
                    msg += '    {}\n'.format(obj.__name__)
        self.assertTrue(all(status), msg)

    @mock.patch('logging.Logger.error')
    def testToken(self, mock):
        token = tokens.Token()
        self.assertEqual(token.name, 'Token')
        self.assertTrue(token.recursive)

        token = tokens.Token(recursive=False)
        self.assertFalse(token.recursive)

    def testSection(self):
        token = tokens.Section()
        self.assertIsInstance(token, tokens.Section)

    def testString(self):
        token = tokens.String(content=u"content")
        self.assertEqual(token.content, "content")

        with self.assertRaises(exceptions.MooseDocsException) as e:
            token = tokens.String(content=1980)
        gold = "The supplied property 'content' must be of type 'unicode', but 'int' was provided."
        self.assertEqual(e.exception.message, gold)

    def testWord(self):
        token = tokens.Word(content=u"content")
        self.assertEqual(token.content, "content")

    def testSpace(self):
        token = tokens.Space()
        self.assertEqual(token.content, ' ')
        self.assertEqual(token.count, 1)
        self.assertEqual(token.recursive, True) # tests token inheritance

        token = tokens.Space(count=42)
        self.assertEqual(token.content, ' ')
        self.assertEqual(token.count, 42)

        with self.assertRaises(exceptions.MooseDocsException) as e:
            token = tokens.Space(count='not int')
        gold = "The supplied property 'count' must be of type 'int', but 'str' was provided."
        self.assertEqual(e.exception.message, gold)

    def testBreak(self):
        token = tokens.Break()
        self.assertEqual(token.content, '\n')
        self.assertEqual(token.count, 1)

        token = tokens.Break(count=42)
        self.assertEqual(token.content, '\n')
        self.assertEqual(token.count, 42)

        with self.assertRaises(exceptions.MooseDocsException) as e:
            token = tokens.Space(count='not int')
        gold = "The supplied property 'count' must be of type 'int', but 'str' was provided."
        self.assertEqual(e.exception.message, gold)

    def testPunctuation(self):
        token = tokens.Punctuation(content=u'---')
        self.assertEqual(token.content, '---')

    def testNumber(self):
        token = tokens.Punctuation(content=u'1980')
        self.assertEqual(token.content, '1980')

    def testCode(self):
        token = tokens.Code(code=u"x+y=2;")
        self.assertEqual(token.code, "x+y=2;")

    def testHeading(self):
        token = tokens.Heading(level=4)
        self.assertEqual(token.level, 4)

        with self.assertRaises(exceptions.MooseDocsException) as e:
            token = tokens.Heading(level='not int')
        gold = "The supplied property 'level' must be of type 'int', but 'str' was provided."
        self.assertEqual(e.exception.message, gold)

    def testParagraph(self):
        tokens.Paragraph()

    def testUnorderedList(self):
        tokens.UnorderedList()

    def testOrderedList(self):
        token = tokens.OrderedList()
        self.assertEqual(token.start, 1)
        token = tokens.OrderedList(start=1980)
        self.assertEqual(token.start, 1980)

        with self.assertRaises(exceptions.MooseDocsException) as e:
            token = tokens.OrderedList(start='not int')
        gold = "The supplied property 'start' must be of type 'int', but 'str' was provided."
        self.assertEqual(e.exception.message, gold)

    def testListItem(self):
        with self.assertRaises(exceptions.MooseDocsException) as e:
            token = tokens.ListItem()
        self.assertIn("A 'ListItem' must have a 'OrderedList' or 'UnorderedList' parent.",
                      e.exception.message)

        root = tokens.OrderedList()
        token = tokens.ListItem(parent=root)
        self.assertIs(token.parent, root)

    def testLink(self):
        with self.assertRaises(exceptions.MooseDocsException) as e:
            token = tokens.Link()
        self.assertIn("The property 'url' is required.", e.exception.message)

        token = tokens.Link(url=u'foo')
        self.assertEqual(token.url, 'foo')
        token.url = u'bar'
        self.assertEqual(token.url, 'bar')

        with self.assertRaises(exceptions.MooseDocsException) as e:
            token.url = 42
        gold = "The supplied property 'url' must be of type 'unicode', but 'int' was provided."
        self.assertEqual(e.exception.message, gold)

    def testShortcut(self):
        with self.assertRaises(exceptions.MooseDocsException) as e:
            token = tokens.Shortcut(key=u'foo')
        self.assertEqual("The property 'link' is required.", e.exception.message)
        with self.assertRaises(exceptions.MooseDocsException) as e:
            token = tokens.Shortcut(content=u'foo', link=u'link')
        self.assertEqual("The property 'key' is required.", e.exception.message)

        token = tokens.Shortcut(key=u'key', link=u'link')
        self.assertEqual(token.key, 'key')
        self.assertEqual(token.link, 'link')

    def testShortcutLink(self):
        with self.assertRaises(exceptions.MooseDocsException) as e:
            token = tokens.ShortcutLink()
        self.assertEqual("The property 'key' is required.", e.exception.message)

        token = tokens.ShortcutLink(key=u'key')
        self.assertEqual(token.key, 'key')

    def testMonospace(self):
        with self.assertRaises(exceptions.MooseDocsException) as e:
            token = tokens.Monospace()
        self.assertIn("The property 'code' is required.", e.exception.message)
        token = tokens.Monospace(code=u'int x;')
        self.assertEqual(token.code, 'int x;')

    def testStrong(self):
        tokens.Strong()

    def testEmphasis(self):
        tokens.Emphasis()

    def testUnderline(self):
        tokens.Underline()

    def testStrikethrough(self):
        tokens.Strikethrough()

    def testQuote(self):
        tokens.Quote()

    def testSuperscript(self):
        tokens.Superscript()

    def testSubscript(self):
        tokens.Subscript()

    def testLabel(self):
        token = tokens.Label(text=u'foo')
        self.assertEqual(token.text, 'foo')

    def testFloat(self):
        token = tokens.Float(label='foo')
        self.assertEqual(token.label, 'foo')
        self.assertEqual(token.caption, None)
        self.assertEqual(token.id, None)

        token = tokens.Float(label='foo', caption=u'bar', id='12345')
        self.assertEqual(token.label, 'foo')
        self.assertEqual(token.caption, 'bar')
        self.assertEqual(token.id, '12345')

        with self.assertRaises(exceptions.MooseDocsException) as e:
            token = tokens.Float()
        self.assertIn("The property 'label' is required.", e.exception.message)

    def testErrorToken(self):
        def func():
            pass
        pattern = lexers.Pattern(name='foo', regex=re.compile('\S'), function=func)
        tokens.ErrorToken(pattern=pattern)

    def testExceptionToken(self):
        def func():
            pass
        pattern = lexers.Pattern(name='foo', regex=re.compile('\S'), function=func)
        tokens.ExceptionToken(pattern=pattern, traceback='foo')

    def testCountToken(self):
        foo1 = tokens.CountToken(None, prefix=u'Foo')
        bar1 = tokens.CountToken(None, prefix=u'Bar')

        foo2 = tokens.CountToken(None, prefix=u'Foo')
        bar2 = tokens.CountToken(None, prefix=u'Bar')

        self.assertEqual(foo1.number, 1)
        self.assertEqual(foo2.number, 2)
        self.assertEqual(bar1.number, 1)
        self.assertEqual(bar2.number, 2)

if __name__ == '__main__':
    unittest.main(verbosity=2)
