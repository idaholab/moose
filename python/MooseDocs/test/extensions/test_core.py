#!/usr/bin/env python
#pylint: disable=missing-docstring

import unittest
import mock
from MooseDocs.extensions import command, core
from MooseDocs.common import exceptions
from MooseDocs.tree import tokens, html, latex
from MooseDocs.base import testing, renderers

# TOKEN OBJECTS TESTS
class TestTokens(unittest.TestCase):
    """Test Token object for <EXTENSION> MooseDocs extension."""
    # There are no tokens defined in the core, but in the future thier might. Having this empty
    # test will produce a usefull error from the integrety tests.

# TOKENIZE TESTS
class TestBreakTokenize(testing.MooseDocsTestCase):
    """Test tokenization of Break"""
    def testToken(self):
        node = self.ast(u'foo\nbar')(0)
        self.assertIsInstance(node, tokens.Paragraph)
        self.assertIsInstance(node(0), tokens.Word)
        self.assertIsInstance(node(1), tokens.Break)
        self.assertIsInstance(node(2), tokens.Word)
        self.assertEqual(node(1).content, '\n')
        self.assertEqual(node(1).count, 1)

class TestCodeTokenize(testing.MooseDocsTestCase):
    """Test tokenization of Code"""
    def testToken(self):
        code = self.ast(u'```\nint x;\n```')(0)
        self.assertIsInstance(code, tokens.Code)
        self.assertString(code.code, '\nint x;\n')
        self.assertString(code.language, 'text')

    def testLanguage(self):
        code = self.ast(u'```language=cpp\nint x;\n```')(0)
        self.assertIsInstance(code, tokens.Code)
        self.assertString(code.code, '\nint x;\n')
        self.assertString(code.language, 'cpp')

class TestEndOfFileTokenize(testing.MooseDocsTestCase):
    """Test tokenization of EndOfFile"""
    def testToken(self):
        # As far as I can tell the core.EndOfFile component is not reachable, if a way to reach it
        # found then it needs to get added to this test.
        ast = self.ast(u'foo\n     ')(0)
        self.assertIsInstance(ast, tokens.Paragraph)

class TestFormatTokenize(testing.MooseDocsTestCase):
    """Test tokenization of Format"""
    def testStrong(self):
        ast = self.ast(u'+strong+')
        self.assertIsInstance(ast(0), tokens.Paragraph)
        self.assertIsInstance(ast(0)(0), tokens.Strong)
        self.assertIsInstance(ast(0)(0)(0), tokens.String)
        self.assertEqual(ast(0)(0)(0).content, "strong")

        ast = self.ast(u'+strong with space\nand a new line+')
        self.assertIsInstance(ast(0), tokens.Paragraph)
        self.assertIsInstance(ast(0)(0), tokens.Strong)
        self.assertIsInstance(ast(0)(0)(0), tokens.String)
        self.assertEqual(ast(0)(0)(0).content, "strong")
        self.assertEqual(ast(0)(0)(-1).content, "line")

    def testUnderline(self):
        ast = self.ast(u'=underline=')
        self.assertIsInstance(ast(0), tokens.Paragraph)
        self.assertIsInstance(ast(0)(0), tokens.Underline)
        self.assertIsInstance(ast(0)(0)(0), tokens.String)
        self.assertEqual(ast(0)(0)(0).content, "underline")

        ast = self.ast(u'=underline with space\nand a new line=')
        self.assertIsInstance(ast(0), tokens.Paragraph)
        self.assertIsInstance(ast(0)(0), tokens.Underline)
        self.assertIsInstance(ast(0)(0)(0), tokens.String)
        self.assertEqual(ast(0)(0)(0).content, "underline")
        self.assertEqual(ast(0)(0)(-1).content, "line")

    def testEmphasis(self):
        ast = self.ast(u'*emphasis*')
        self.assertIsInstance(ast(0), tokens.Paragraph)
        self.assertIsInstance(ast(0)(0), tokens.Emphasis)
        self.assertIsInstance(ast(0)(0)(0), tokens.String)
        self.assertEqual(ast(0)(0)(0).content, "emphasis")

        ast = self.ast(u'*emphasis with space\nand a new line*')
        self.assertIsInstance(ast(0), tokens.Paragraph)
        self.assertIsInstance(ast(0)(0), tokens.Emphasis)
        self.assertIsInstance(ast(0)(0)(0), tokens.String)
        self.assertEqual(ast(0)(0)(0).content, "emphasis")
        self.assertEqual(ast(0)(0)(-1).content, "line")

    def testStrikethrough(self):
        ast = self.ast(u'~strikethrough~')
        self.assertIsInstance(ast(0), tokens.Paragraph)
        self.assertIsInstance(ast(0)(0), tokens.Strikethrough)
        self.assertIsInstance(ast(0)(0)(0), tokens.String)
        self.assertEqual(ast(0)(0)(0).content, "strikethrough")

        ast = self.ast(u'~strikethrough with space\nand a new line~')
        self.assertIsInstance(ast(0), tokens.Paragraph)
        self.assertIsInstance(ast(0)(0), tokens.Strikethrough)
        self.assertIsInstance(ast(0)(0)(0), tokens.String)
        self.assertEqual(ast(0)(0)(0).content, "strikethrough")
        self.assertEqual(ast(0)(0)(-1).content, "line")

    def testSubscript(self):
        ast = self.ast(u'S@x@')
        self.assertIsInstance(ast(0), tokens.Paragraph)
        self.assertIsInstance(ast(0)(0), tokens.Word)
        self.assertIsInstance(ast(0)(1), tokens.Subscript)
        self.assertIsInstance(ast(0)(1)(0), tokens.Word)
        self.assertEqual(ast(0)(1)(0).content, "x")

    def testSuperscript(self):
        ast = self.ast(u'S^x^')
        self.assertIsInstance(ast(0), tokens.Paragraph)
        self.assertIsInstance(ast(0)(0), tokens.Word)
        self.assertIsInstance(ast(0)(1), tokens.Superscript)
        self.assertIsInstance(ast(0)(1)(0), tokens.Word)
        self.assertEqual(ast(0)(1)(0).content, "x")

    def testMonospace(self):
        ast = self.ast(u'`x`')
        self.assertIsInstance(ast(0), tokens.Paragraph)
        self.assertIsInstance(ast(0)(0), tokens.Monospace)
        self.assertEqual(ast(0)(0).code, "x")

        ast = self.ast(u'`*x*`')
        self.assertIsInstance(ast(0), tokens.Paragraph)
        self.assertIsInstance(ast(0)(0), tokens.Monospace)
        self.assertEqual(ast(0)(0).code, u"*x*")

    def testSuperscriptAndSubscript(self):
        ast = self.ast(u's@y^x^@')
        self.assertIsInstance(ast(0), tokens.Paragraph)
        self.assertIsInstance(ast(0)(0), tokens.Word)
        self.assertIsInstance(ast(0)(1), tokens.Subscript)
        self.assertIsInstance(ast(0)(1)(0), tokens.Word)
        self.assertIsInstance(ast(0)(1)(1), tokens.Superscript)
        self.assertIsInstance(ast(0)(1)(1)(0), tokens.Word)
        self.assertEqual(ast(0)(1)(1)(0).content, "x")

        ast = self.ast(u's^y@x@^')
        self.assertIsInstance(ast(0), tokens.Paragraph)
        self.assertIsInstance(ast(0)(0), tokens.Word)
        self.assertIsInstance(ast(0)(1), tokens.Superscript)
        self.assertIsInstance(ast(0)(1)(0), tokens.Word)
        self.assertIsInstance(ast(0)(1)(1), tokens.Subscript)
        self.assertIsInstance(ast(0)(1)(1)(0), tokens.Word)
        self.assertEqual(ast(0)(1)(1)(0).content, "x")

    def testNesting(self):
        ast = self.ast(u'=*emphasis* underline=')
        self.assertIsInstance(ast(0), tokens.Paragraph)
        self.assertIsInstance(ast(0)(0), tokens.Underline)
        self.assertIsInstance(ast(0)(0)(0), tokens.Emphasis)
        self.assertIsInstance(ast(0)(0)(0)(0), tokens.Word)
        self.assertEqual(ast(0)(0)(0)(0).content, "emphasis")
        self.assertIsInstance(ast(0)(0)(2), tokens.Word)
        self.assertEqual(ast(0)(0)(2).content, "underline")

        ast = self.ast(u'*=underline= emphasis*')
        self.assertIsInstance(ast(0), tokens.Paragraph)
        self.assertIsInstance(ast(0)(0), tokens.Emphasis)
        self.assertIsInstance(ast(0)(0)(0), tokens.Underline)
        self.assertEqual(ast(0)(0)(0)(0).content, "underline")
        self.assertIsInstance(ast(0)(0)(2), tokens.Word)
        self.assertEqual(ast(0)(0)(2).content, "emphasis")

        ast = self.ast(u'*=+~strike~ bold+ under= emphasis*')
        self.assertIsInstance(ast(0), tokens.Paragraph)
        self.assertIsInstance(ast(0)(0), tokens.Emphasis)
        self.assertIsInstance(ast(0)(0)(0), tokens.Underline)
        self.assertIsInstance(ast(0)(0)(0)(0), tokens.Strong)
        self.assertIsInstance(ast(0)(0)(0)(0)(0), tokens.Strikethrough)

        ast = self.ast(u's@*emphasis*@')
        self.assertIsInstance(ast(0), tokens.Paragraph)
        self.assertIsInstance(ast(0)(0), tokens.Word)
        self.assertIsInstance(ast(0)(1), tokens.Subscript)
        self.assertIsInstance(ast(0)(1)(0), tokens.Emphasis)
        self.assertEqual(ast(0)(1)(0)(0).content, "emphasis")

        ast = self.ast(u'*s@x@*')
        self.assertIsInstance(ast(0), tokens.Paragraph)
        self.assertIsInstance(ast(0)(0), tokens.Emphasis)
        self.assertIsInstance(ast(0)(0)(0), tokens.Word)
        self.assertIsInstance(ast(0)(0)(1), tokens.Subscript)
        self.assertEqual(ast(0)(0)(1)(0).content, "x")

class TestHeadingHashTokenize(testing.MooseDocsTestCase):
    """Test tokenization of HeadingHash"""
    def testToken(self):
        ast = self.ast(u'# Heading with Spaces')
        h = ast(0)
        self.assertIsInstance(h, tokens.Heading)
        self.assertEqual(h.level, 1)
        self.assertIsInstance(h(0), tokens.Label)
        self.assertIsInstance(h(1), tokens.Word)
        self.assertIsInstance(h(2), tokens.Space)
        self.assertIsInstance(h(3), tokens.Word)
        self.assertIsInstance(h(4), tokens.Space)
        self.assertIsInstance(h(5), tokens.Word)
        self.assertEqual(h(1).content, 'Heading')
        self.assertEqual(h(3).content, 'with')
        self.assertEqual(h(5).content, 'Spaces')

    def testLevels(self):
        for i in range(1,7):
            ast = self.ast(u'{} Heading'.format('#'*i))
            self.assertEqual(ast(0).level, i)

class TestLinkTokenize(testing.MooseDocsTestCase):
    """Test tokenization of Link"""
    def testToken(self):
        link = self.ast(u'[link](url.html)')(0)(0)
        self.assertIsInstance(link.parent, tokens.Paragraph)
        self.assertIsInstance(link, tokens.Link)
        self.assertIsInstance(link(0), tokens.Word)
        self.assertEqual(link(0).content, 'link')
        self.assertEqual(link.url, 'url.html')

    def testSettings(self):
        link = self.ast(u'[link](url.html id=bar)')(0)(0)
        self.assertEqual(link['id'], 'bar')

class TestListTokenize(testing.MooseDocsTestCase):
    """Test tokenization of List"""
    # Base class that is not used directly.

class TestNumberTokenize(testing.MooseDocsTestCase):
    """Test tokenization of Number"""
    def testToken(self):
        node = self.ast(u'foo1bar')(0)
        self.assertIsInstance(node(0), tokens.Word)
        self.assertIsInstance(node(1), tokens.Number)
        self.assertIsInstance(node(2), tokens.Word)

class TestOrderedListTokenize(testing.MooseDocsTestCase):
    """Test tokenization of OrderedList"""
    def testToken(self):
        token = self.ast(u'1. foo\n1. bar')
        self.assertIsInstance(token(0), tokens.OrderedList)
        self.assertIsInstance(token(0)(0), tokens.ListItem)
        self.assertIsInstance(token(0)(0)(0), tokens.Paragraph)
        self.assertIsInstance(token(0)(0)(0)(0), tokens.Word)

        self.assertIsInstance(token(0)(1), tokens.ListItem)
        self.assertIsInstance(token(0)(1)(0), tokens.Paragraph)
        self.assertIsInstance(token(0)(1)(0)(0), tokens.Word)

    def testStart(self):
        token = self.ast(u'42. foo\n1. bar')
        self.assertIsInstance(token(0), tokens.OrderedList)
        self.assertEqual(token(0).start, 42)

    def testSeparate(self):
        token = self.ast(u'1. foo\n\n\n1. bar')
        self.assertIsInstance(token(0), tokens.OrderedList)
        self.assertIsInstance(token(0)(0), tokens.ListItem)
        self.assertIsInstance(token(0)(0)(0), tokens.Paragraph)
        self.assertIsInstance(token(0)(0)(0)(0), tokens.Word)

        self.assertIsInstance(token(1), tokens.OrderedList)
        self.assertIsInstance(token(1)(0), tokens.ListItem)
        self.assertIsInstance(token(1)(0)(0), tokens.Paragraph)
        self.assertIsInstance(token(1)(0)(0)(0), tokens.Word)

    def testNesting(self):
        token = self.ast(u'1. foo\n\n   - nested\n   - list\n1. bar')
        self.assertIsInstance(token(0), tokens.OrderedList)
        self.assertIsInstance(token(0)(0), tokens.ListItem)
        self.assertIsInstance(token(0)(0)(0), tokens.Paragraph)
        self.assertIsInstance(token(0)(0)(0)(0), tokens.Word)

        self.assertIsInstance(token(0)(0)(1), tokens.UnorderedList)
        self.assertIsInstance(token(0)(0)(1)(0), tokens.ListItem)
        self.assertIsInstance(token(0)(0)(1)(1), tokens.ListItem)

        self.assertIsInstance(token(0)(1), tokens.ListItem)
        self.assertIsInstance(token(0)(1)(0), tokens.Paragraph)
        self.assertIsInstance(token(0)(1)(0)(0), tokens.Word)

class TestParagraphTokenize(testing.MooseDocsTestCase):
    """Test tokenization of Paragraph"""
    def testToken(self):
        for i in range(2, 5):
            token = self.ast(u'foo{}bar'.format('\n'*i))
            self.assertIsInstance(token(0), tokens.Paragraph)
            self.assertIsInstance(token(0)(0), tokens.Word)

            self.assertIsInstance(token(1), tokens.Paragraph)
            self.assertIsInstance(token(1)(0), tokens.Word)

class TestPunctuationTokenize(testing.MooseDocsTestCase):
    """Test tokenization of Punctuation"""
    def testToken(self):
        node = self.ast(u'a-z')(0)
        self.assertIsInstance(node(0), tokens.Word)
        self.assertIsInstance(node(1), tokens.Punctuation)
        self.assertIsInstance(node(2), tokens.Word)

        self.assertString(node(1).content, '-')

    def testMultiple(self):
        node = self.ast(u'a-$#%z')(0)
        self.assertIsInstance(node(0), tokens.Word)
        self.assertIsInstance(node(1), tokens.Punctuation)
        self.assertIsInstance(node(2), tokens.Punctuation)
        self.assertIsInstance(node(3), tokens.Punctuation)
        self.assertIsInstance(node(4), tokens.Punctuation)
        self.assertIsInstance(node(5), tokens.Word)

        self.assertString(node(1).content, '-')
        self.assertString(node(2).content, '$')
        self.assertString(node(3).content, '#')
        self.assertString(node(4).content, '%')

    def testCaret(self):
        node = self.ast(u'a^z')(0)
        self.assertIsInstance(node(0), tokens.Word)
        self.assertIsInstance(node(1), tokens.Punctuation)
        self.assertIsInstance(node(2), tokens.Word)
        self.assertString(node(1).content, '^')

    def testAll(self):
        text = u'Word!@#$%^&*()-=_+{}[]|\":;\'?/>.<,~`   Word'
        node = self.ast(text)(0)
        self.assertIsInstance(node(0), tokens.Word)
        for i in range(1, 32):
            self.assertIsInstance(node(i), tokens.Punctuation)
            self.assertString(node(i).content, text[i+3])
        self.assertIsInstance(node(32), tokens.Space)
        self.assertIsInstance(node(33), tokens.Word)

    def testMixedMultiple(self):
        node = self.ast(u'---$')(0)
        self.assertIsInstance(node(0), tokens.Punctuation)
        self.assertIsInstance(node(1), tokens.Punctuation)
        self.assertString(node(0).content, '---') #TODO: use count???
        self.assertString(node(1).content, '$')

class TestQuoteTokenize(testing.MooseDocsTestCase):
    """Test tokenization of Quote"""
    def testToken(self):
        node = self.ast(u'> foo bar')(0)
        self.assertIsInstance(node, tokens.Quote)
        self.assertIsInstance(node(0), tokens.Paragraph)
        self.assertIsInstance(node(0)(0), tokens.Word)
        self.assertIsInstance(node(0)(1), tokens.Space)
        self.assertIsInstance(node(0)(2), tokens.Word)

        self.assertString(node(0)(0).content, 'foo')
        self.assertString(node(0)(1).content, ' ')
        self.assertString(node(0)(2).content, 'bar')

class TestShortcutTokenize(testing.MooseDocsTestCase):
    """Test tokenization of Shortcut"""
    def testToken(self):
        shortcut = self.ast(u'[key]: this is the shortcut content')(0)
        self.assertIsInstance(shortcut, tokens.Shortcut)
        self.assertEqual(shortcut.key, 'key')
        self.assertEqual(shortcut.link, 'this is the shortcut content')

class TestShortcutLinkTokenize(testing.MooseDocsTestCase):
    """Test tokenization of ShortcutLink"""
    def testToken(self):
        link = self.ast(u'[key]\n\n[key]: content')(0)(0)
        self.assertIsInstance(link, tokens.ShortcutLink)
        self.assertEqual(link.key, 'key')

class TestSpaceTokenize(testing.MooseDocsTestCase):
    """Test tokenization of Space"""
    def testToken(self):
        node = self.ast(u'sit      amet')(0)
        self.assertIsInstance(node(0), tokens.Word)
        self.assertIsInstance(node(1), tokens.Space)
        self.assertIsInstance(node(2), tokens.Word)

        self.assertString(node(0).content, 'sit')
        self.assertString(node(1).content, ' ')
        self.assertEqual(node(1).count, 6)
        self.assertString(node(2).content, 'amet')

class TestUnorderedListTokenize(testing.MooseDocsTestCase):
    """Test tokenization of UnorderedList"""
    def testToken(self):
        token = self.ast(u'- foo\n- bar')
        self.assertIsInstance(token(0), tokens.UnorderedList)
        self.assertIsInstance(token(0)(0), tokens.ListItem)
        self.assertIsInstance(token(0)(0)(0), tokens.Paragraph)
        self.assertIsInstance(token(0)(0)(0)(0), tokens.Word)

        self.assertIsInstance(token(0)(1), tokens.ListItem)
        self.assertIsInstance(token(0)(1)(0), tokens.Paragraph)
        self.assertIsInstance(token(0)(1)(0)(0), tokens.Word)

    def testSeparate(self):
        token = self.ast(u'- foo\n\n\n- bar')
        self.assertIsInstance(token(0), tokens.UnorderedList)
        self.assertIsInstance(token(0)(0), tokens.ListItem)
        self.assertIsInstance(token(0)(0)(0), tokens.Paragraph)
        self.assertIsInstance(token(0)(0)(0)(0), tokens.Word)

        self.assertIsInstance(token(1), tokens.UnorderedList)
        self.assertIsInstance(token(1)(0), tokens.ListItem)
        self.assertIsInstance(token(1)(0)(0), tokens.Paragraph)
        self.assertIsInstance(token(1)(0)(0)(0), tokens.Word)

    def testNesting(self):
        token = self.ast(u'- foo\n\n  - nested\n  - list\n- bar')
        self.assertIsInstance(token(0), tokens.UnorderedList)
        self.assertIsInstance(token(0)(0), tokens.ListItem)
        self.assertIsInstance(token(0)(0)(0), tokens.Paragraph)
        self.assertIsInstance(token(0)(0)(0)(0), tokens.Word)

        self.assertIsInstance(token(0)(0)(1), tokens.UnorderedList)
        self.assertIsInstance(token(0)(0)(1)(0), tokens.ListItem)
        self.assertIsInstance(token(0)(0)(1)(1), tokens.ListItem)

        self.assertIsInstance(token(0)(1), tokens.ListItem)
        self.assertIsInstance(token(0)(1)(0), tokens.Paragraph)
        self.assertIsInstance(token(0)(1)(0)(0), tokens.Word)

    def testNestedCode(self):
        token = self.ast(u'- foo\n\n  ```\n  bar\n  ```')
        self.assertIsInstance(token(0), tokens.UnorderedList)
        self.assertIsInstance(token(0)(0), tokens.ListItem)
        self.assertIsInstance(token(0)(0)(0), tokens.Paragraph)
        self.assertIsInstance(token(0)(0)(1), tokens.Code)

class TestWordTokenize(testing.MooseDocsTestCase):
    """Test tokenization of Word"""
    def testToken(self):
        node = self.ast(u'sit amet, consectetur')(0)
        self.assertIsInstance(node(0), tokens.Word)
        self.assertIsInstance(node(1), tokens.Space)
        self.assertIsInstance(node(2), tokens.Word)
        self.assertIsInstance(node(3), tokens.Punctuation)
        self.assertIsInstance(node(4), tokens.Space)
        self.assertIsInstance(node(5), tokens.Word)

        self.assertString(node(0).content, 'sit')
        self.assertString(node(2).content, 'amet')
        self.assertString(node(3).content, ',')
        self.assertString(node(5).content, 'consectetur')

# RENDERER TESTS
class TestRenderBreakHTML(testing.MooseDocsTestCase):
    """Test renderering of RenderBreak with HTMLRenderer"""

    RENDERER = renderers.HTMLRenderer
    TEXT = u'foo\nbar'

    def node(self):
        return self.render(self.TEXT).find('moose-content', attr='class')(0)

    def testTree(self):
        node = self.node()
        self.assertIsInstance(node, html.Tag)
        self.assertIsInstance(node(0), html.String)
        self.assertIsInstance(node(1), html.String)
        self.assertIsInstance(node(2), html.String)

        self.assertString(node(0).content, 'foo')
        self.assertString(node(1).content, u' ')
        self.assertString(node(2).content, 'bar')

    def testWrite(self):
        node = self.node()
        html = node.write()
        self.assertString(html, '<p>foo bar</p>')

class TestRenderBreakMaterialize(TestRenderBreakHTML):
    """Test renderering of RenderBreak with MaterializeRenderer"""

    RENDERER = renderers.MaterializeRenderer

class TestRenderBreakLatex(testing.MooseDocsTestCase):
    """Test renderering of RenderBreak with LatexRenderer"""

    RENDERER = renderers.LatexRenderer

    def testTree(self):
        node = self.render(u'foo\nbar')(-1)
        self.assertIsInstance(node, latex.Environment)
        self.assertIsInstance(node(0), latex.CustomCommand)
        self.assertIsInstance(node(1), latex.String)
        self.assertIsInstance(node(2), latex.String)
        self.assertIsInstance(node(3), latex.String)

        self.assertString(node(0).name, 'par')
        self.assertString(node(1).content, 'foo')
        self.assertString(node(2).content, ' ')
        self.assertString(node(3).content, 'bar')

    def testWrite(self):
        node = self.render(u'foo\nbar')(-1)
        tex = node.write()
        self.assertString(tex, '\n\\begin{document}\n\n\\par\nfoo bar\n\\end{document}\n')

class TestRenderCodeHTML(testing.MooseDocsTestCase):
    """Test renderering of RenderCode with HTMLRenderer"""

    RENDERER = renderers.HTMLRenderer

    def testTree(self):
        node = self.render(u'```\nint x;\n```').find('pre')
        self.assertIsInstance(node, html.Tag)
        self.assertIsInstance(node(0), html.Tag)
        self.assertIsInstance(node(0)(0), html.String)

        self.assertEqual(node.name, 'pre')
        self.assertEqual(node(0).name, 'code')
        self.assertString(node(0)['class'], 'language-text')
        self.assertString(node(0)(0).content, '\nint x;\n')

    def testWrite(self):
        node = self.render(u'```\nint x;\n```').find('pre')
        self.assertString(node.write(), '<pre><code class="language-text">\nint x;\n</code></pre>')

    def testTreeLanguage(self):
        node = self.render(u'```language=cpp\nint x;\n```').find('pre')
        self.assertString(node(0)['class'], 'language-cpp')

    def testWriteLanguage(self):
        node = self.render(u'```language=cpp\nint x;\n```').find('pre')
        self.assertString(node.write(), '<pre><code class="language-cpp">\nint x;\n</code></pre>')

class TestRenderCodeMaterialize(TestRenderCodeHTML):
    """Test renderering of RenderCode with MaterializeRenderer"""

    RENDERER = renderers.MaterializeRenderer

class TestRenderCodeLatex(testing.MooseDocsTestCase):
    """Test renderering of RenderCode with LatexRenderer"""

    RENDERER = renderers.LatexRenderer

    def testTree(self):
        node = self.render(u'```\nint x;\n```').find('document')(0)
        self.assertIsInstance(node, latex.Environment)
        self.assertIsInstance(node(0), latex.String)

    def testWrite(self):
        node = self.render(u'```\nint x;\n```').find('document')(0)
        self.assertEqual(node.write(), u'\n\\begin{verbatim}\n\nint x;\n\n\\end{verbatim}\n')

class TestRenderEmphasisHTML(testing.MooseDocsTestCase):
    """Test renderering of RenderEmphasis with HTMLRenderer"""

    RENDERER = renderers.HTMLRenderer
    TEXT = u'*content*'

    def node(self):
        return self.render(self.TEXT).find('moose-content', attr='class')(0)(0)

    def testTree(self):
        node = self.node()
        self.assertIsInstance(node, html.Tag)
        self.assertIsInstance(node(0), html.String)

        self.assertString(node.name, 'em')
        self.assertString(node(0).content, 'content')

    def testWrite(self):
        node = self.node()
        html = node.write()
        self.assertString(html, '<em>content</em>')

class TestRenderEmphasisMaterialize(TestRenderEmphasisHTML):
    """Test renderering of RenderEmphasis with MaterializeRenderer"""

    RENDERER = renderers.MaterializeRenderer

class TestRenderEmphasisLatex(testing.MooseDocsTestCase):
    """Test renderering of RenderEmphasis with LatexRenderer"""

    RENDERER = renderers.LatexRenderer

    def testTree(self):
        node = self.render(u'*content*')(-1)(1)

        self.assertIsInstance(node, latex.Command)
        self.assertIsInstance(node(0), latex.String)

        self.assertString(node.name, 'emph')
        self.assertString(node(0).content, 'content')

    def testWrite(self):
        node = self.render(u'*content*')(-1)(1)
        tex = node.write()
        self.assertString(tex, '\\emph{content}')

class TestRenderExceptionHTML(testing.MooseDocsTestCase):
    """Test renderering of RenderException with HTMLRenderer"""
    EXTENSIONS = [core, command]
    RENDERER = renderers.HTMLRenderer
    TEXT = u'!unknown command'

    def node(self):
        return self.render(self.TEXT).find('moose-content', attr='class')(0)

    @mock.patch('logging.Logger.error')
    def testTree(self, mock):
        node = self.node()
        self.assertIsInstance(node, html.Tag)
        self.assertIsInstance(node(0), html.String)
        mock.assert_called_once()

    @mock.patch('logging.Logger.error')
    def testWrite(self, mock):
        node = self.node()
        self.assertString(node.write(), '<div class="moose-exception">!unknown command</div>')

class TestRenderExceptionMaterialize(TestRenderExceptionHTML):
    """Test renderering of RenderException with MaterializeRenderer"""

    RENDERER = renderers.MaterializeRenderer

    @mock.patch('logging.Logger.error')
    def testWrite(self, mock):
        node = self.node()
        self.assertIn('class="moose-exception modal-trigger">!unknown command</a>', node.write())

class TestRenderExceptionLatex(testing.MooseDocsTestCase):
    """Test renderering of RenderException with LatexRenderer"""

    RENDERER = renderers.LatexRenderer
    TEXT = u'!unknown command'

    def node(self):
        return self.render(self.TEXT).find('document')

    @mock.patch('logging.Logger.error')
    def testTree(self, mock):
        node = self.render(u'!unknown command').find('document')
        self.assertString(node(1).content, '!')
        self.assertString(node(2).content, 'unknown')
        self.assertString(node(3).content, ' ')
        self.assertString(node(4).content, 'command')

    @mock.patch('logging.Logger.error')
    def testWrite(self, mock):
        node = self.render(u'!unknown command').find('document')
        self.assertString(node.write(), '\n\\begin{document}\n\n\\par\n!unknown command\n\\end{document}\n')

class TestRenderErrorHTML(testing.MooseDocsTestCase):
    pass # same as TestRenderException

class TestRenderErrorMaterialize(TestRenderErrorHTML):
    pass # same as TestRenderException

class TestRenderErrorLatex(testing.MooseDocsTestCase):
    pass # same as TestRenderException


class TestRenderHeadingHTML(testing.MooseDocsTestCase):
    """Test renderering of RenderHeading with HTMLRenderer"""

    RENDERER = renderers.HTMLRenderer

    def node(self, content):
        return self.render(content).find('moose-content', attr='class')(0)

    def testTree(self):
        h = self.node(u'# Heading with Spaces')
        self.assertIsInstance(h, html.Tag)
        self.assertEqual(h.name, 'h1')
        for child in h.children:
            self.assertIsInstance(child, html.String)
        self.assertEqual(h(0).content, 'Heading')
        self.assertEqual(h(1).content, ' ')
        self.assertEqual(h(2).content, 'with')
        self.assertEqual(h(3).content, ' ')
        self.assertEqual(h(4).content, 'Spaces')

        self.assertString(h.write(), "<h1>Heading with Spaces</h1>")

    def testSettings(self):
        h = self.node(u'# Heading with Spaces style=font-size:42pt;')
        self.assertIsInstance(h, html.Tag)
        self.assertEqual(h.name, 'h1')
        for child in h.children:
            self.assertIsInstance(child, html.String)
        self.assertEqual(h(0).content, 'Heading')
        self.assertEqual(h(1).content, ' ')
        self.assertEqual(h(2).content, 'with')
        self.assertEqual(h(3).content, ' ')
        self.assertEqual(h(4).content, 'Spaces')
        self.assertEqual(h['style'], 'font-size:42pt;')

        self.assertString(h.write(), '<h1 style="font-size:42pt;">Heading with Spaces</h1>')

class TestRenderHeadingMaterialize(TestRenderHeadingHTML):
    """Test renderering of RenderHeading with MaterializeRenderer"""

    RENDERER = renderers.MaterializeRenderer

    def node(self, text):
        return self.render(text).find('moose-content', attr='class')(0)(0)


class TestRenderHeadingLatex(testing.MooseDocsTestCase):
    """Test renderering of RenderHeading with LatexRenderer"""

    RENDERER = renderers.LatexRenderer
    TEXT = u'ENTER TEXT HERE'

    def checkLevel(self, lvl, cmd):
        node = self.render(u'{} Heading with Space'.format('#'*lvl))(-1)(0)
        self.assertIsInstance(node, latex.Command)
        self.assertString(node.name, cmd)

        self.assertIsInstance(node(0), latex.Command)
        self.assertString(node(0).name, 'label')
        self.assertString(node(0)(0).content, 'heading-with-space')

        self.assertIsInstance(node(1), latex.String)
        self.assertIsInstance(node(2), latex.String)
        self.assertIsInstance(node(3), latex.String)
        self.assertIsInstance(node(4), latex.String)
        self.assertIsInstance(node(5), latex.String)

       # self.assertString(node(0).name, 'par')
        self.assertString(node(1).content, 'Heading')
        self.assertString(node(2).content, ' ')
        self.assertString(node(3).content, 'with')
        self.assertString(node(4).content, ' ')
        self.assertString(node(5).content, 'Space')

        tex = node.write()
        self.assertString(tex, '\n\\%s{\\label{heading-with-space}Heading with Space}\n' % cmd)

    def testLevels(self):
        self.checkLevel(1, 'chapter')
        self.checkLevel(2, 'section')
        self.checkLevel(3, 'subsection')
        self.checkLevel(4, 'subsubsection')
        self.checkLevel(5, 'paragraph')
        self.checkLevel(6, 'subparagraph')

class TestRenderLabelHTML(testing.MooseDocsTestCase):
    """Test renderering of RenderLabel with HTMLRenderer"""

    RENDERER = renderers.HTMLRenderer

    def testWrite(self):
        node = tokens.Label(None, text=u'foo')
        html = node.write()
        self.assertEqual(html, '')

class TestRenderLabelMaterialize(TestRenderLabelHTML):
    """Test renderering of RenderLabel with MaterializeRenderer"""

    RENDERER = renderers.MaterializeRenderer

class TestRenderLabelLatex(testing.MooseDocsTestCase):
    """Test renderering of RenderLabel with LatexRenderer"""

    RENDERER = renderers.LatexRenderer

    def testWrite(self):
        node = self._renderer.render(tokens.Label(None, text=u'foo')).find('document')(-1)
        tex = node.write()
        self.assertString(tex, '\\label{foo}')

class TestRenderLinkHTML(testing.MooseDocsTestCase):
    """Test renderering of RenderLink with HTMLRenderer"""

    RENDERER = renderers.HTMLRenderer

    def node(self, text):
        return self.render(text).find('moose-content', attr='class')(0)(0)

        node = self.node(u'[link](url.html)')
        self.assertIsInstance(node, html.Tag)
        self.assertEqual(node.name, 'a')
        self.assertIsInstance(node(0), html.String)
        self.assertString(node(0).content, 'link')
        self.assertString(node['href'], 'url.html')

    def testTreeSettings(self):
        node = self.node(u'[link](url.html id=foo)')
        self.assertIsInstance(node, html.Tag)
        self.assertEqual(node.name, 'a')
        self.assertIsInstance(node(0), html.String)
        self.assertString(node(0).content, 'link')
        self.assertString(node['href'], 'url.html')
        self.assertString(node['id'], 'foo')

    def testWrite(self):
        link = self.node(u'[link](url.html)')
        self.assertString(link.write(), '<a href="url.html">link</a>')

    def testWriteSettings(self):
        link = self.node(u'[link](url.html id=bar)')
        self.assertString(link.write(), '<a href="url.html" id="bar">link</a>')

class TestRenderLinkMaterialize(TestRenderLinkHTML):
    """Test renderering of RenderLink with MaterializeRenderer"""

    RENDERER = renderers.MaterializeRenderer

    def testWrite(self):
        link = self.node(u'[link](url.html)')
        self.assertString(link.write(), '<a href="url.html" data-tooltip="url.html" ' \
                                        'data-position="top" class="tooltipped">link</a>')

    def testWriteSettings(self):
        link = self.node(u'[link](url.html id=bar)')
        self.assertString(link.write(), '<a id="bar" href="url.html" data-tooltip="url.html" ' \
                                        'data-position="top" class="tooltipped">link</a>')

class TestRenderLinkLatex(testing.MooseDocsTestCase):
    """Test renderering of RenderLink with LatexRenderer"""

    RENDERER = renderers.LatexRenderer

    def testTree(self):
        node = self.render(u'[link](url.html)')(-1)(1)
        self.assertIsInstance(node, latex.Command)
        self.assertIsInstance(node(0), latex.Brace)
        self.assertIsInstance(node(0)(0), latex.String)
        self.assertIsInstance(node(1), latex.Brace)
        self.assertIsInstance(node(1)(0), latex.String)

        self.assertString(node.name, 'href')
        self.assertString(node(0)(0).content, 'url.html')
        self.assertString(node(1)(0).content, 'link')

    def testWrite(self):
        node = self.render(u'[link](url.html)')(-1)(1)
        self.assertString(node.write(), '\\href{url.html}{link}')

class TestRenderListItemHTML(testing.MooseDocsTestCase):
    """Test renderering of RenderListItem with HTMLRenderer"""

    RENDERER = renderers.HTMLRenderer
    TEXT = u'- item'

    def node(self):
        return self.render(self.TEXT).find('moose-content', attr='class')(0)

    def testWrite(self):
        html = self.node()
        self.assertEqual(html.write(), '<ul><li><p>item</p></li></ul>')

    def testError(self):
        with self.assertRaises(exceptions.MooseDocsException) as e:
            tokens.ListItem(None)
        self.assertIn("A 'ListItem' must have a 'OrderedList' or 'UnorderedList' parent.",
                      str(e.exception))

class TestRenderListItemMaterialize(TestRenderListItemHTML):
    """Test renderering of RenderListItem with MaterializeRenderer"""

    RENDERER = renderers.MaterializeRenderer
    def testWrite(self):
        html = self.node()
        self.assertEqual(html.write(), '<ul class="browser-default"><li><p>item</p></li></ul>')


class TestRenderListItemLatex(testing.MooseDocsTestCase):
    """Test renderering of RenderListItem with LatexRenderer"""

    RENDERER = renderers.LatexRenderer

    def testWrite(self):
        tex = self.render(u'- content').find('document')(0)
        self.assertString(tex.write(),
                          '\n\\begin{itemize}\n\n\\item\n\par\ncontent\n\\end{itemize}\n')

class TestRenderMonospaceHTML(testing.MooseDocsTestCase):
    """Test renderering of RenderMonospace with HTMLRenderer"""

    RENDERER = renderers.HTMLRenderer
    TEXT = u'foo `code` bar'

    def node(self):
        return self.render(self.TEXT).find('moose-content', attr='class')(0)

    def testTree(self):
        node = self.node()
        self.assertIsInstance(node, html.Tag)
        self.assertIsInstance(node(0), html.String)
        self.assertIsInstance(node(1), html.String)
        self.assertIsInstance(node(2), html.Tag)
        self.assertIsInstance(node(2)(0), html.String)
        self.assertIsInstance(node(3), html.String)
        self.assertIsInstance(node(4), html.String)

        self.assertString(node(0).content, 'foo')
        self.assertString(node(1).content, ' ')
        self.assertEqual(node(2).name, 'code')
        self.assertString(node(2)(0).content, 'code')
        self.assertString(node(3).content, ' ')
        self.assertString(node(4).content, 'bar')

    def testWrite(self):
        node = self.node()
        html = node.write()
        self.assertString(html, '<p>foo <code>code</code> bar</p>')

class TestRenderMonospaceMaterialize(TestRenderMonospaceHTML):
    """Test renderering of RenderMonospace with MaterializeRenderer"""

    RENDERER = renderers.MaterializeRenderer

class TestRenderMonospaceLatex(testing.MooseDocsTestCase):
    """Test renderering of RenderMonospace with LatexRenderer"""

    RENDERER = renderers.LatexRenderer

    def testTree(self):
        node = self.render(u'foo `code` bar')(-1)
        self.assertIsInstance(node, latex.Environment)
        self.assertIsInstance(node(0), latex.CustomCommand)
        self.assertIsInstance(node(1), latex.String)
        self.assertIsInstance(node(2), latex.String)
        self.assertIsInstance(node(3), latex.Command)
        self.assertIsInstance(node(4), latex.String)
        self.assertIsInstance(node(5), latex.String)

        self.assertString(node(0).name, 'par')
        self.assertString(node(1).content, 'foo')
        self.assertString(node(2).content, ' ')
        self.assertString(node(3).name, 'texttt')
        self.assertString(node(3)(0).content, 'code')
        self.assertString(node(4).content, ' ')
        self.assertString(node(5).content, 'bar')

    def testWrite(self):
        node = self.render(u'foo `code` bar')(-1)
        tex = node.write()
        self.assertString(tex, '\n\\begin{document}\n\n\\par\nfoo \\texttt{code} bar\n\\end{document}\n')

class TestRenderOrderedListHTML(testing.MooseDocsTestCase):
    """Test renderering of RenderOrderedList with HTMLRenderer"""

    RENDERER = renderers.HTMLRenderer
    TEXT = u'1. foo\n1. bar'

    def node(self):
        return self.render(self.TEXT).find('moose-content', attr='class')(0)

    def testTree(self):
        node = self.node()

        self.assertIsInstance(node, html.Tag)
        self.assertIsInstance(node(0), html.Tag)
        self.assertIsInstance(node(1), html.Tag)

        self.assertIsInstance(node(0)(0), html.Tag)
        self.assertIsInstance(node(1)(0), html.Tag)

        self.assertIsInstance(node(0)(0)(0), html.String)
        self.assertIsInstance(node(1)(0)(0), html.String)

        self.assertString(node.name, 'ol')
        self.assertString(node(0).name, 'li')
        self.assertString(node(1).name, 'li')

        self.assertString(node(0)(0).name, 'p')
        self.assertString(node(1)(0).name, 'p')

        self.assertString(node(0)(0)(0).content, 'foo')
        self.assertString(node(1)(0)(0).content, 'bar')

    def testWrite(self):
        node = self.node()
        self.assertString(node.write(), '<ol><li><p>foo </p></li><li><p>bar</p></li></ol>')

class TestRenderOrderedListMaterialize(TestRenderOrderedListHTML):
    """Test renderering of RenderOrderedList with MaterializeRenderer"""

    RENDERER = renderers.MaterializeRenderer

    def testWrite(self):
        node = self.node()
        self.assertString(node.write(), '<ol start="1" class="browser-default"><li><p>foo </p></li><li><p>bar</p></li></ol>')


class TestRenderOrderedListLatex(testing.MooseDocsTestCase):
    """Test renderering of RenderOrderedList with LatexRenderer"""

    RENDERER = renderers.LatexRenderer

    def testTree(self):
        node = self.render(u'1. foo\n1. bar')(-1)(0)
        self.assertIsInstance(node, latex.Environment)
        self.assertIsInstance(node(0), latex.CustomCommand)
        self.assertIsInstance(node(1), latex.Command)
        self.assertIsInstance(node(2), latex.String)
        self.assertIsInstance(node(3), latex.String)
        self.assertIsInstance(node(4), latex.CustomCommand)
        self.assertIsInstance(node(5), latex.Command)
        self.assertIsInstance(node(6), latex.String)

        self.assertString(node.name, 'enumerate')
        self.assertString(node(0).name, 'item')
        self.assertString(node(1).name, 'par')
        self.assertString(node(2).content, 'foo')
        self.assertString(node(3).content, ' ')
        self.assertString(node(4).name, 'item')
        self.assertString(node(5).name, 'par')
        self.assertString(node(6).content, 'bar')

    def testWrite(self):
        node = self.render(u'1. foo\n1. bar')(-1)(0)
        node.write()
        self.assertString(node.write(),
                          '\n\\begin{enumerate}\n\n\\item\n\\par\nfoo \n\\item\n\\par\nbar\n\\end{enumerate}\n')

class TestRenderParagraphHTML(testing.MooseDocsTestCase):
    """Test renderering of RenderParagraph with HTMLRenderer"""

    RENDERER = renderers.HTMLRenderer
    TEXT = u'foo\n\n\n\nbar'

    def node(self):
        return self.render(self.TEXT).find('moose-content', attr='class')

    def testTree(self):
        node = self.node()
        self.assertIsInstance(node, html.Tag)

        self.assertIsInstance(node(0), html.Tag)
        self.assertEqual(node(0).name, 'p')
        self.assertIsInstance(node(0)(0), html.String)
        self.assertString(node(0)(0).content, 'foo')

        self.assertIsInstance(node(1), html.Tag)
        self.assertEqual(node(1).name, 'p')
        self.assertIsInstance(node(1)(0), html.String)
        self.assertString(node(1)(0).content, 'bar')

    def testWrite(self):
        node = self.node()
        html = node(0).write()
        self.assertString(html, '<p>foo</p>')
        html = node(1).write()
        self.assertString(html, '<p>bar</p>')

class TestRenderParagraphMaterialize(TestRenderParagraphHTML):
    """Test renderering of RenderParagraph with MaterializeRenderer"""

    RENDERER = renderers.MaterializeRenderer

class TestRenderParagraphLatex(testing.MooseDocsTestCase):
    """Test renderering of RenderParagraph with LatexRenderer"""

    RENDERER = renderers.LatexRenderer
    def testTree(self):
        node = self.render(u'[link](url.html)')(-1)(1)
        self.assertIsInstance(node, latex.Command)
        self.assertIsInstance(node(0), latex.Brace)
        self.assertIsInstance(node(0)(0), latex.String)
        self.assertIsInstance(node(1), latex.Brace)
        self.assertIsInstance(node(1)(0), latex.String)

        self.assertString(node.name, 'href')
        self.assertString(node(0)(0).content, 'url.html')
        self.assertString(node(1)(0).content, 'link')

    def testWrite(self):
        node = self.render(u'+content+')(-1)(1)
        tex = node.write()
        self.assertString(tex, '\\textbf{content}')

class TestRenderPunctuationHTML(testing.MooseDocsTestCase):
    """Test renderering of RenderPunctuation with HTMLRenderer"""

    RENDERER = renderers.HTMLRenderer

    def node(self, text):
        return self.render(text).find('moose-content', attr='class')(0)

    def testTree(self):
        node = self.node(u'foo-bar')
        self.assertString(node(0).content, 'foo')
        self.assertString(node(1).content, '-')
        self.assertString(node(2).content, 'bar')

        node = self.node(u'foo--bar')
        self.assertString(node(1).content, '&ndash;')

        node = self.node(u'foo---bar')
        self.assertString(node(1).content, '&mdash;')

    def testWrite(self):
        node = self.node(u'foo-bar')
        self.assertString(node.write(), '<p>foo-bar</p>')

class TestRenderPunctuationMaterialize(TestRenderPunctuationHTML):
    """Test renderering of RenderPunctuation with MaterializeRenderer"""

    RENDERER = renderers.MaterializeRenderer

class TestRenderPunctuationLatex(testing.MooseDocsTestCase):
    """Test renderering of RenderPunctuation with LatexRenderer"""

    RENDERER = renderers.LatexRenderer

    def testTree(self):
        node = self.render(u'foo-bar').find('document')
        self.assertString(node(1).content, 'foo')
        self.assertString(node(2).content, '-')
        self.assertString(node(3).content, 'bar')

    def testWrite(self):
        node = self.render(u'foo-bar').find('document')
        self.assertString(node.write(), '\n\\begin{document}\n\n\\par\nfoo-bar\n\\end{document}\n')

class TestRenderQuoteHTML(testing.MooseDocsTestCase):
    """Test renderering of RenderQuote with HTMLRenderer"""

    RENDERER = renderers.HTMLRenderer
    TEXT = u'> foo bar'

    def node(self):
        return self.render(self.TEXT).find('moose-content', attr='class')(0)

    def testTree(self):
        node = self.node()
        self.assertIsInstance(node, html.Tag)
        self.assertEqual(node.name, 'blockquote')
        self.assertIsInstance(node(0), html.Tag)
        self.assertString(node(0).name, 'p')

        self.assertIsInstance(node(0)(0), html.String)
        self.assertString(node(0)(0).content, 'foo')

        self.assertIsInstance(node(0)(1), html.String)
        self.assertString(node(0)(1).content, ' ')

        self.assertIsInstance(node(0)(2), html.String)
        self.assertString(node(0)(2).content, 'bar')

    def testWrite(self):
        node = self.node()
        html = node.write()
        self.assertString(html, '<blockquote><p>foo bar</p></blockquote>')

class TestRenderQuoteMaterialize(TestRenderQuoteHTML):
    """Test renderering of RenderQuote with MaterializeRenderer"""

    RENDERER = renderers.MaterializeRenderer

class TestRenderQuoteLatex(testing.MooseDocsTestCase):
    """Test renderering of RenderQuote with LatexRenderer"""

    RENDERER = renderers.LatexRenderer
    def testTree(self):
        node = self.render(u'> foo bar').find('document')(-1)
        self.assertIsInstance(node, latex.Environment)
        self.assertString(node.name, 'quote')

        self.assertIsInstance(node(0), latex.Command)
        self.assertString(node(0).name, 'par')

        self.assertIsInstance(node(1), latex.String)
        self.assertIsInstance(node(2), latex.String)
        self.assertIsInstance(node(3), latex.String)

        self.assertString(node(1).content, 'foo')
        self.assertString(node(2).content, ' ')
        self.assertString(node(3).content, 'bar')

    def testWrite(self):
        node = self.render(u'> foo bar').find('document')(-1)
        tex = node.write()
        self.assertString(tex, u'\n\\begin{quote}\n\n\\par\nfoo bar\n\\end{quote}\n')

class TestRenderShortcutLinkHTML(testing.MooseDocsTestCase):
    """Test renderering of RenderShortcutLink with HTMLRenderer"""

    RENDERER = renderers.HTMLRenderer

    def node(self, text):
        return self.render(text).find('moose-content', attr='class')(0)(0)

    def testTree(self):
        node = self.node(u'[key]\n\n[key]: content')
        self.assertIsInstance(node, html.Tag)
        self.assertIsInstance(node(0), html.String)

        self.assertEqual(node.name, 'a')
        self.assertString(node(0).content, 'key')
        self.assertEqual(node['href'], 'content')

    @mock.patch('logging.Logger.error')
    def testShortcutLinkError(self, mock):
        self.node(u'Some\ntext\nwith a [item] that\nis bad')
        mock.assert_called_once()
        args, _ = mock.call_args
        self.assertIn("The shortcut link key 'item' was not located", args[0])

    @mock.patch('logging.Logger.error')
    def testShortcutLinkError2(self, mock):
        self.node(u'[item] with some text\n[item]: foo')
        args, _ = mock.call_args
        self.assertIn("The shortcut link key 'item' was not located", args[0])

    def testWrite(self):
        node = self.node(u'[key]\n\n[key]: content')
        self.assertString(node.write(), '<a href="content">key</a>')

    def testWriteSettings(self):
        link = self.node(u'[test id=bar]\n\n[test]: foo')
        self.assertString(link.write(), '<a href="foo" id="bar">test</a>')

class TestRenderShortcutLinkMaterialize(TestRenderShortcutLinkHTML):
    """Test renderering of RenderShortcutLink with MaterializeRenderer"""

    RENDERER = renderers.MaterializeRenderer

    def testWrite(self):
        node = self.node(u'[key]\n\n[key]: content')
        self.assertString(node.write(), '<a class="tooltipped" href="content" ' \
                                        'data-tooltip="content" data-position="top">key</a>')

    def testWriteSettings(self):
        link = self.node(u'[test id=bar]\n\n[test]: foo')
        self.assertString(link.write(), '<a class="tooltipped" href="foo" data-tooltip="foo" ' \
                                        'data-position="top" id="bar">test</a>')

class TestRenderShortcutLinkLatex(testing.MooseDocsTestCase):
    """Test renderering of RenderShortcutLink with LatexRenderer"""

    RENDERER = renderers.LatexRenderer

    def testTree(self):
        node = self.render(u'[key]\n\n[key]: content').find('document')(1)
        self.assertIsInstance(node, latex.Command)
        self.assertIsInstance(node(0), latex.Brace)
        self.assertIsInstance(node(0)(0), latex.String)
        self.assertIsInstance(node(1), latex.Brace)
        self.assertIsInstance(node(1)(0), latex.String)

        self.assertString(node.name, 'href')
        self.assertString(node(0)(0).content, 'content')
        self.assertString(node(1)(0).content, 'key')

    def testWrite(self):
        node = self.render(u'[key]\n\n[key]: content').find('document')(1)
        self.assertString(node.write(), '\\href{content}{key}')

class TestRenderStrikethroughHTML(testing.MooseDocsTestCase):
    """Test renderering of RenderStrikethrough with HTMLRenderer"""

    RENDERER = renderers.HTMLRenderer
    TEXT = u'~content~'

    def node(self):
        return self.render(self.TEXT).find('moose-content', attr='class')(0)(0)

    def testTree(self):
        node = self.node()
        self.assertIsInstance(node, html.Tag)
        self.assertIsInstance(node(0), html.String)

        self.assertString(node.name, 'strike')
        self.assertString(node(0).content, 'content')

    def testWrite(self):
        node = self.node()
        html = node.write()
        self.assertString(html, '<strike>content</strike>')

class TestRenderStrikethroughMaterialize(TestRenderStrikethroughHTML):
    """Test renderering of RenderStrikethrough with MaterializeRenderer"""

    RENDERER = renderers.MaterializeRenderer

class TestRenderStrikethroughLatex(testing.MooseDocsTestCase):
    """Test renderering of RenderStrikethrough with LatexRenderer"""

    RENDERER = renderers.LatexRenderer

    def testTree(self):
        node = self.render(u'~content~')(-1)(1)

        self.assertIsInstance(node, latex.Command)
        self.assertIsInstance(node(0), latex.String)

        self.assertString(node.name, 'sout')
        self.assertString(node(0).content, 'content')

    def testWrite(self):
        node = self.render(u'~content~')(-1)(1)
        self.assertString(node.write(), '\\sout{content}')

class TestRenderStringHTML(testing.MooseDocsTestCase):
    """Test renderering of RenderString with HTMLRenderer"""

    RENDERER = renderers.HTMLRenderer
    TEXT = u'sit amet, consectetur'

    def node(self):
        return self.render(self.TEXT).find('moose-content', attr='class')(0)

    def testTree(self):
        node = self.node()
        self.assertIsInstance(node, html.Tag)
        for i in range(6):
            self.assertIsInstance(node(i), html.String)

        self.assertString(node(0).content, 'sit')
        self.assertString(node(1).content, ' ')
        self.assertString(node(2).content, 'amet')
        self.assertString(node(3).content, ',')
        self.assertString(node(4).content, ' ')
        self.assertString(node(5).content, 'consectetur')

    def testWrite(self):
        node = self.node()
        self.assertString(node.write(), '<p>sit amet, consectetur</p>')

class TestRenderStringMaterialize(TestRenderStringHTML):
    """Test renderering of RenderString with MaterializeRenderer"""

    RENDERER = renderers.MaterializeRenderer

class TestRenderStringLatex(testing.MooseDocsTestCase):
    """Test renderering of RenderString with LatexRenderer"""

    RENDERER = renderers.LatexRenderer

    def testTree(self):
        node = self.render(u'sit amet, consectetur').find('document')
        for i in range(1,7):
            self.assertIsInstance(node(i), latex.String)

        self.assertString(node(1).content, 'sit')
        self.assertString(node(2).content, ' ')
        self.assertString(node(3).content, 'amet')
        self.assertString(node(4).content, ',')
        self.assertString(node(5).content, ' ')
        self.assertString(node(6).content, 'consectetur')

    def testWrite(self):
        node = self.render(u'sit amet, consectetur').find('document')
        self.assertString(node.write(),
                          '\n\\begin{document}\n\n\\par\nsit amet, consectetur\n\\end{document}\n')

class TestRenderStrongHTML(testing.MooseDocsTestCase):
    """Test renderering of RenderStrong with HTMLRenderer"""

    RENDERER = renderers.HTMLRenderer
    TEXT = u'+content+'

    def node(self):
        return self.render(self.TEXT).find('moose-content', attr='class')(0)(0)

    def testTree(self):
        node = self.node()
        self.assertIsInstance(node, html.Tag)
        self.assertIsInstance(node(0), html.String)

        self.assertString(node.name, 'strong')
        self.assertString(node(0).content, 'content')

    def testWrite(self):
        node = self.node()
        self.assertString(node.write(), '<strong>content</strong>')

class TestRenderStrongMaterialize(TestRenderStrongHTML):
    """Test renderering of RenderStrong with MaterializeRenderer"""

    RENDERER = renderers.MaterializeRenderer

class TestRenderStrongLatex(testing.MooseDocsTestCase):
    """Test renderering of RenderStrong with LatexRenderer"""

    RENDERER = renderers.LatexRenderer

    def testTree(self):
        node = self.render(u'+content+')(-1)(1)

        self.assertIsInstance(node, latex.Command)
        self.assertIsInstance(node(0), latex.String)

        self.assertString(node.name, 'textbf')
        self.assertString(node(0).content, 'content')

    def testWrite(self):
        node = self.render(u'+content+')(-1)(1)
        self.assertString(node.write(), '\\textbf{content}')

class TestRenderSubscriptHTML(testing.MooseDocsTestCase):
    """Test renderering of RenderSubscript with HTMLRenderer"""

    RENDERER = renderers.HTMLRenderer
    TEXT = u'foo@content@'

    def node(self):
        return self.render(self.TEXT).find('moose-content', attr='class')(0)

    def testTree(self):
        node = self.node()
        self.assertIsInstance(node(0), html.String)
        self.assertString(node(1).name, 'sub')
        self.assertString(node(1)(0).content, 'content')

    def testWrite(self):
        node = self.node()
        self.assertString(node.write(), '<p>foo<sub>content</sub></p>')

class TestRenderSubscriptMaterialize(TestRenderSubscriptHTML):
    """Test renderering of RenderSubscript with MaterializeRenderer"""

    RENDERER = renderers.MaterializeRenderer

class TestRenderSubscriptLatex(testing.MooseDocsTestCase):
    """Test renderering of RenderSubscript with LatexRenderer"""

    RENDERER = renderers.LatexRenderer

    def testTree(self):
        node = self.render(u'foo@content@')(-1)

        self.assertIsInstance(node(1), latex.String)
        self.assertString(node(1).content, 'foo')

        self.assertIsInstance(node(2), latex.InlineMath)
        self.assertString(node(2)(0).content, '_{')

        self.assertIsInstance(node(2)(1), latex.Command)
        self.assertString(node(2)(1).name, 'text')
        self.assertString(node(2)(1)(0).content, 'content')

        self.assertString(node(2)(2).content, '}')

    def testWrite(self):
        node = self.render(u'foo@content@').find('document')(-1)
        self.assertString(node.write(), '$\_\{\\text{content}\}$')

class TestRenderSuperscriptHTML(testing.MooseDocsTestCase):
    """Test renderering of RenderSuperscript with HTMLRenderer"""

    RENDERER = renderers.HTMLRenderer
    TEXT = u'foo^content^'

    def node(self):
        return self.render(self.TEXT).find('moose-content', attr='class')(0)

    def testTree(self):
        node = self.node()
        self.assertIsInstance(node(0), html.String)
        self.assertString(node(1).name, 'sup')
        self.assertString(node(1)(0).content, 'content')

    def testWrite(self):
        node = self.node()
        self.assertString(node.write(), '<p>foo<sup>content</sup></p>')

class TestRenderSuperscriptMaterialize(TestRenderSuperscriptHTML):
    """Test renderering of RenderSuperscript with MaterializeRenderer"""

    RENDERER = renderers.MaterializeRenderer

class TestRenderSuperscriptLatex(testing.MooseDocsTestCase):
    """Test renderering of RenderSuperscript with LatexRenderer"""

    RENDERER = renderers.LatexRenderer
    def testTree(self):
        node = self.render(u'foo^content^').find('document')

        self.assertIsInstance(node(1), latex.String)
        self.assertString(node(1).content, 'foo')

        self.assertIsInstance(node(2), latex.InlineMath)
        self.assertString(node(2)(0).content, '^{')

        self.assertIsInstance(node(2)(1), latex.Command)
        self.assertString(node(2)(1).name, 'text')
        self.assertString(node(2)(1)(0).content, 'content')

        self.assertString(node(2)(2).content, '}')

    def testWrite(self):
        node = self.render(u'foo^content^').find('document')(-1)
        self.assertString(node.write(), '$\^\{\\text{content}\}$')

class TestRenderUnderlineHTML(testing.MooseDocsTestCase):
    """Test renderering of RenderUnderline with HTMLRenderer"""

    RENDERER = renderers.HTMLRenderer
    TEXT = u'=content='

    def node(self):
        return self.render(self.TEXT).find('moose-content', attr='class')(0)(0)

    def testTree(self):
        node = self.node()
        self.assertIsInstance(node, html.Tag)
        self.assertIsInstance(node(0), html.String)

        self.assertString(node.name, 'u')
        self.assertString(node(0).content, 'content')

    def testWrite(self):
        node = self.node()
        self.assertString(node.write(), '<u>content</u>')

class TestRenderUnderlineMaterialize(TestRenderUnderlineHTML):
    """Test renderering of RenderUnderline with MaterializeRenderer"""

    RENDERER = renderers.MaterializeRenderer

class TestRenderUnderlineLatex(testing.MooseDocsTestCase):
    """Test renderering of RenderUnderline with LatexRenderer"""

    RENDERER = renderers.LatexRenderer

    def testTree(self):
        node = self.render(u'=content=').find('document')(1)

        self.assertIsInstance(node, latex.Command)
        self.assertIsInstance(node(0), latex.String)

        self.assertString(node.name, 'underline')
        self.assertString(node(0).content, 'content')

    def testWrite(self):
        node = self.render(u'=content=').find('document')(1)
        self.assertString(node.write(), '\\underline{content}')

class TestRenderUnorderedListHTML(testing.MooseDocsTestCase):
    """Test renderering of RenderUnorderedList with HTMLRenderer"""

    RENDERER = renderers.HTMLRenderer
    TEXT = u'ENTER TEXT HERE'

    def node(self, text):
        return self.render(text).find('moose-content', attr='class')(0)

    def testTree(self):
        node = self.node(u'- foo\n- bar')
        self.assertIsInstance(node, html.Tag)
        self.assertIsInstance(node(0), html.Tag)
        self.assertIsInstance(node(1), html.Tag)

        self.assertIsInstance(node(0)(0), html.Tag)
        self.assertIsInstance(node(1)(0), html.Tag)

        self.assertIsInstance(node(0)(0)(0), html.String)
        self.assertIsInstance(node(1)(0)(0), html.String)

        self.assertString(node.name, 'ul')
        self.assertString(node(0).name, 'li')
        self.assertString(node(1).name, 'li')

        self.assertString(node(0)(0).name, 'p')
        self.assertString(node(1)(0).name, 'p')

        self.assertString(node(0)(0)(0).content, 'foo')
        self.assertString(node(1)(0)(0).content, 'bar')

    def testWrite(self):
        node = self.node(u'- foo\n- bar')
        self.assertString(node.write(), '<ul><li><p>foo </p></li><li><p>bar</p></li></ul>')

    def testNestedCode(self):
        node = self.node(u'- foo\n\n  ```language=text\n  code\n  ```')
        self.assertString(node.write(), '<ul><li><p>foo</p><pre><code ' \
                                        'class="language-text">\ncode\n</code></pre></li></ul>')


class TestRenderUnorderedListMaterialize(TestRenderUnorderedListHTML):
    """Test renderering of RenderUnorderedList with MaterializeRenderer"""

    RENDERER = renderers.MaterializeRenderer

    def testWrite(self):
        node = self.node(u'- foo\n- bar')
        self.assertString(node.write(),
                          '<ul class="browser-default"><li><p>foo </p></li><li><p>bar</p></li></ul>')

    def testNestedCode(self):
        node = self.node(u'- foo\n\n  ```language=text\n  code\n  ```')
        self.assertString(node.write(), '<ul class="browser-default"><li><p>foo</p><pre><code ' \
                                        'class="language-text">\ncode\n</code></pre></li></ul>')

class TestRenderUnorderedListLatex(testing.MooseDocsTestCase):
    """Test renderering of RenderUnorderedList with LatexRenderer"""

    RENDERER = renderers.LatexRenderer

    def testTree(self):
        node = self.render(u'- foo\n- bar').find('document')(0)

        self.assertIsInstance(node, latex.Environment)
        self.assertIsInstance(node(0), latex.CustomCommand)
        self.assertIsInstance(node(1), latex.Command)
        self.assertIsInstance(node(2), latex.String)
        self.assertIsInstance(node(3), latex.String)
        self.assertIsInstance(node(4), latex.CustomCommand)
        self.assertIsInstance(node(5), latex.Command)
        self.assertIsInstance(node(6), latex.String)

        self.assertString(node.name, 'itemize')
        self.assertString(node(0).name, 'item')
        self.assertString(node(1).name, 'par')
        self.assertString(node(2).content, 'foo')
        self.assertString(node(3).content, ' ')
        self.assertString(node(4).name, 'item')
        self.assertString(node(5).name, 'par')
        self.assertString(node(6).content, 'bar')

    def testWrite(self):
        node = self.render(u'- foo\n- bar').find('document')(0)
        self.assertString(node.write(),
                          '\n\\begin{itemize}\n\n\\item\n\\par\nfoo \n\\item\n\\par\nbar\n\\end{itemize}\n')

class TestErrors(testing.MooseDocsTestCase):

    @mock.patch('logging.Logger.error')
    def testUnknownSettings(self, mock):
        h = self.render(u'# Heading with Spaces foo=bar')(0)
        mock.assert_called_once()
        self.assertIsInstance(h, html.Tag)
        self.assertEqual(h.name, 'div')
        self.assertEqual(h(0).content, '# Heading with Spaces foo=bar')
        self.assertString(h.write(), '<div class="moose-exception"># Heading with Spaces foo=bar</div>')

if __name__ == '__main__':
    unittest.main(verbosity=2)
