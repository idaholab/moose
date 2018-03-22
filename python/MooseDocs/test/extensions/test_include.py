#!/usr/bin/env python2
import os
import unittest
import tempfile
import shutil

from MooseDocs.extensions import core, include, command
from MooseDocs.tree import tokens, html, latex, page
from MooseDocs.base import testing, renderers

class TestIncludeBase(testing.MooseDocsTestCase):

    EXTENSIONS = [core, command, include]
    def setUpContent(self):
        self.loc = tempfile.mkdtemp()
        self.files = [os.path.join(self.loc, 'file0.md'),
                      os.path.join(self.loc, 'file1.md'),
                      os.path.join(self.loc, 'file2.md'),
                      os.path.join(self.loc, 'file3.md')]

        with open(self.files[0], 'w') as fid:
            fid.write('File 0\n\n!include {}'.format(os.path.basename(self.files[1])))
        with open(self.files[1], 'w') as fid:
            fid.write('File 1\n\n!include {}'.format(os.path.basename(self.files[2])))
        with open(self.files[2], 'w') as fid:
            fid.write('File 2')
        with open(self.files[3], 'w') as fid:
            fid.write('File 3\n\n!include {}'.format(os.path.basename(self.files[2])))

        self.root = page.DirectoryNode(None, source=self.loc)
        page.MarkdownNode(self.root, base=os.path.dirname(self.loc), source=self.files[0])
        page.MarkdownNode(self.root, base=os.path.dirname(self.loc), source=self.files[1])
        page.MarkdownNode(self.root, base=os.path.dirname(self.loc), source=self.files[2])
        page.MarkdownNode(self.root, base=os.path.dirname(self.loc), source=self.files[3])

        return self.root

    def setUp(self):
        testing.MooseDocsTestCase.setUp(self)
        self._translator.execute(num_threads=1)

    def tearDown(self):
        shutil.rmtree(self.loc)

# TOKENIZE TESTS
class TestIncludeTokenize(TestIncludeBase):
    """Test tokenization of Include"""

    def testToken(self):
        for i in range(4):
            ast = self.root(i).ast
            self.assertIsInstance(ast(0), tokens.Paragraph)
            self.assertIsInstance(ast(0)(0), tokens.Word)
            self.assertIsInstance(ast(0)(1), tokens.Space)
            self.assertIsInstance(ast(0)(2), tokens.Number)
            self.assertEqual(ast(0)(0).content, u'File')
            self.assertEqual(ast(0)(2).content, unicode(i))

# RENDERER TESTS
@unittest.skip('WIP')
class TestRenderIncludeHTML(TestIncludeBase):
    """Test renderering of RenderInclude with HTMLRenderer"""

    RENDERER = renderers.HTMLRenderer
    def node(self):
        return self.root(0).result.find('moose-content', attr='class')

    def testTree(self):
        node = self.node()

        for i in range(3):
            self.assertIsInstance(node(i), html.Tag)
            self.assertIsInstance(node(i)(0), html.String)
            self.assertEqual(node(i)(0).content, u'File')

            self.assertIsInstance(node(i)(1), html.String)
            self.assertEqual(node(i)(1).content, u' ')

            self.assertIsInstance(node(i)(2), html.String)
            self.assertEqual(node(i)(2).content, unicode(i))


    def testWrite(self):
        node = self.node()
        for i in range(3):
            self.assertEqual(node(i).write(), u'<p>File {}</p>'.format(i))

class TestRenderIncludeMaterialize(TestRenderIncludeHTML):
    """Test renderering of RenderInclude with MaterializeRenderer"""

    RENDERER = renderers.MaterializeRenderer

@unittest.skip('LaTeX WIP')
class TestRenderIncludeLatex(TestIncludeBase):
    """Test renderering of RenderInclude with LatexRenderer"""

    RENDERER = renderers.LatexRenderer
    def node(self):
        return self.root(0).result.find('document')

    def testTree(self):
        node = self.node()
        for i in range(0, 3, 4):
            self.assertIsInstance(node(i), latex.Command)
            self.assertIsInstance(node(i+1), latex.String)
            self.assertEqual(node(i+1).content, u'File')

            self.assertIsInstance(node(i+2), latex.String)
            self.assertEqual(node(i+2).content, u' ')

            self.assertIsInstance(node(i+3), latex.String)
            self.assertEqual(node(i+3).content, unicode(i))

    def testWrite(self):
        node = self.node()
        self.assertEqual(node.write(), u'\n\\begin{document}\n\n\\par\nFile 0\n\\par\nFile 1\n\\par\nFile 2\n\\end{document}\n')

if __name__ == '__main__':
    unittest.main(verbosity=2)
