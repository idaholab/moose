#!/usr/bin/env python2
#pylint: disable=missing-docstring
import os
import unittest
import tempfile
import shutil
import logging

from MooseDocs.extensions import core, autolink, floats
from MooseDocs.tree import tokens, page, html
from MooseDocs.base import testing, renderers

logging.basicConfig()

class TestAutoLinkBase(testing.MooseDocsTestCase):
    EXTENSIONS = [core, autolink, floats]

    def setUpContent(self):
        self.loc = tempfile.mkdtemp()
        self.files = [os.path.join(self.loc, 'file0.md'),
                      os.path.join(self.loc, 'file1.md')]

        with open(self.files[0], 'w') as fid:
            fid.write('# Page 0\n\n[Foo](file1.md)\n\n[Bar](file1.md#sub)\n\n[file1.md]\n\n[file1.md#sub]')
        with open(self.files[1], 'w') as fid:
            fid.write('# Page 1\n\nparagraph\n\n## Sub id=sub')

        self.root = page.DirectoryNode(None, source=self.loc)
        page.MarkdownNode(self.root, base=os.path.dirname(self.loc), source=self.files[0])
        page.MarkdownNode(self.root, base=os.path.dirname(self.loc), source=self.files[1])
        return self.root

    def build(self):
        self._translator.execute(num_threads=1)

    def tearDown(self):
        shutil.rmtree(self.loc)

# TOKEN OBJECTS TESTS
class TestTokens(unittest.TestCase):
    """Test Token object for MooseDocs.extensions.autolink MooseDocs extension."""

    def testAutoLink(self):
        token = autolink.AutoLink(url=u'http://test', key=u'foo', bookmark=u'sub')
        self.assertEqual(token.bookmark, u'sub')

    def testAutoShortcutLink(self):
        token = autolink.AutoShortcutLink(key=u'foo', header=True, bookmark=u'sub')
        self.assertEqual(token.bookmark, u'sub')
        self.assertTrue(token.header)

# TOKENIZE TESTS
class TestAutoLinkComponentTokenize(TestAutoLinkBase):
    """Test tokenization of AutoLinkComponent"""
    def testToken(self):
        self.build()
        auto0 = self.root(0).ast(1)(0)
        self.assertIsInstance(auto0, autolink.AutoLink)
        self.assertEqual(auto0.url, u'file1.md')
        self.assertIsNone(auto0.bookmark)
        self.assertIsInstance(auto0(0), tokens.Word)
        self.assertEqual(auto0(0).content, u'Foo')

        auto1 = self.root(0).ast(2)(0)
        self.assertIsInstance(auto1, autolink.AutoLink)
        self.assertEqual(auto1.url, u'file1.md')
        self.assertEqual(auto1.bookmark, u'#sub')
        self.assertIsInstance(auto1(0), tokens.Word)
        self.assertEqual(auto1(0).content, u'Bar')

class TestAutoShortcutLinkComponentTokenize(TestAutoLinkBase):
    """Test tokenization of AutoShortcutLinkComponent"""

    def testToken(self):
        self.build()
        auto0 = self.root(0).ast(3)(0)
        self.assertIsInstance(auto0, autolink.AutoShortcutLink)
        self.assertEqual(auto0.key, u'file1.md')
        self.assertIsNone(auto0.bookmark)
        self.assertEqual(auto0.children, ())

        auto1 = self.root(0).ast(4)(0)
        self.assertIsInstance(auto1, autolink.AutoShortcutLink)
        self.assertEqual(auto1.key, u'file1.md')
        self.assertEqual(auto1.bookmark, u'#sub')
        self.assertEqual(auto1.children, ())

# RENDERER TESTS
class TestRenderAutoLinkHTML(TestAutoLinkBase):
    """Test renderering of RenderAutoLink with HTMLRenderer"""

    RENDERER = renderers.HTMLRenderer

    def node(self):
        self.build()
        return self._translator.root(0).result

    def testTree(self):
        node = self.node()
        a0 = node(1)(0)
        self.assertIsInstance(a0, html.Tag)
        self.assertEqual(a0.name, 'a')
        self.assertIn(u'file1.html', a0['href'])
        self.assertIsInstance(a0(0), html.String)
        self.assertEqual(a0(0).content, u'Foo')

        a1 = node(2)(0)
        self.assertIsInstance(a1, html.Tag)
        self.assertEqual(a1.name, 'a')
        self.assertIn(u'file1.html#sub', a1['href'])
        self.assertIsInstance(a1(0), html.String)
        self.assertEqual(a1(0).content, u'Bar')


    def testWrite(self):
        node = self.node()
        gold = node(1)(0).write()
        self.assertIn(u'<a', gold)
        self.assertIn(u'file1.html', gold)
        self.assertIn(u'>Foo</a>', gold)

        gold = node(2)(0).write()
        self.assertIn(u'<a', gold)
        self.assertIn(u'file1.html#sub"', gold)
        self.assertIn(u'>Bar</a>', gold)


class TestRenderAutoLinkMaterialize(TestRenderAutoLinkHTML):
    """Test renderering of RenderAutoLink with MaterializeRenderer"""
    RENDERER = renderers.MaterializeRenderer

    def node(self):
        self.build()
        return self.root(0).result.find('moose-content', attr='class')(0)

@unittest.skip('WIP LaTeX')
class TestRenderAutoLinkLatex(testing.MooseDocsTestCase):
    """Test renderering of RenderAutoLink with LatexRenderer"""

    EXTENSIONS = [core, autolink]
    RENDERER = renderers.LatexRenderer
    TEXT = u'TEST STRING HERE'

    def node(self):
        return self.render(self.TEXT).find('document')(0)

    def testTree(self):
        #node = self.node()
        self.assertFalse(True)

    def testWrite(self):
        node = self.node()
        self.assertEqual(node.write(), "GOLD")

class TestRenderAutoShortcutLinkHTML(TestAutoLinkBase):
    """Test renderering of RenderAutoShortcutLink with HTMLRenderer"""
    RENDERER = renderers.HTMLRenderer

    def node(self):
        self.build()
        return self.root(0).result.find('moose-content', attr='class')

    def testTree(self):
        node = self.node()
        a0 = node(3)(0)
        self.assertIsInstance(a0, html.Tag)
        self.assertEqual(a0.name, 'a')
        self.assertIn(u'file1.html', a0['href'])
        self.assertIsInstance(a0(0), html.String)
        self.assertEqual(a0(0).content, u'Page')

        self.assertIsInstance(a0(1), html.String)
        self.assertEqual(a0(1).content, u' ')

        self.assertIsInstance(a0(2), html.String)
        self.assertEqual(a0(2).content, u'1')


        a1 = node(4)(0)
        self.assertIsInstance(a1, html.Tag)
        self.assertEqual(a1.name, 'a')
        self.assertIn(u'file1.html#sub', a1['href'])
        self.assertIsInstance(a1(0), html.String)
        self.assertEqual(a1(0).content, u'Page')

        self.assertIsInstance(a1(1), html.String)
        self.assertEqual(a1(1).content, u' ')

        self.assertIsInstance(a1(2), html.String)
        self.assertEqual(a1(2).content, u'1')

        self.assertIsInstance(a1(3), html.String)
        self.assertEqual(a1(3).content, u':')

        self.assertIsInstance(a1(4), html.String)
        self.assertEqual(a1(4).content, u'Sub')

    def testWrite(self):
        node = self.node()
        gold = node(3)(0).write()
        self.assertIn(u'<a', gold)
        self.assertIn(u'file1.html', gold)
        self.assertIn(u'>Page 1</a>', gold)

        gold = node(4)(0).write()
        self.assertIn(u'<a', gold)
        self.assertIn(u'file1.html#sub"', gold)
        self.assertIn(u'>Page 1:Sub</a>', gold)

class TestRenderAutoShortcutLinkMaterialize(TestRenderAutoShortcutLinkHTML):
    """Test renderering of RenderAutoShortcutLink with MaterializeRenderer"""
    RENDERER = renderers.MaterializeRenderer

    def node(self):
        self.build()
        return self.root(0).result.find('moose-content', attr='class')(0)


@unittest.skip('WIP LaTeX')
class TestRenderAutoShortcutLinkLatex(testing.MooseDocsTestCase):
    """Test renderering of RenderAutoShortcutLink with LatexRenderer"""

    EXTENSIONS = [core, autolink]
    RENDERER = renderers.LatexRenderer
    TEXT = u'TEST STRING HERE'

    def node(self):
        return self.render(self.TEXT).find('document')(0)

    def testTree(self):
        #node = self.node()
        self.assertFalse(True)

    def testWrite(self):
        node = self.node()
        self.assertEqual(node.write(), "GOLD")

if __name__ == '__main__':
    unittest.main(verbosity=2)
