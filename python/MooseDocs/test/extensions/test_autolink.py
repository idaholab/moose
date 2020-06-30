#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import mock
import unittest
import logging
import collections
from MooseDocs.test import MooseDocsTestCase
from MooseDocs.extensions import core, floats, heading, autolink
from MooseDocs import base, common
from MooseDocs.tree import pages, tokens
logging.basicConfig()

class TestShortcutLink(MooseDocsTestCase):
    EXTENSIONS = [core, floats, heading, autolink]

    def setupContent(self):
        config = [dict(root_dir='python/MooseDocs/test/content', content=['extensions/core.md'])]
        return common.get_content(config, '.md')

    def testAutoLink(self):
        ast = self.tokenize('[core.md]')
        self.assertSize(ast, 1)
        self.assertToken(ast(0), 'Paragraph', size=1)
        self.assertToken(ast(0)(0), 'AutoLink', size=0, bookmark=None, exact=False, optional=False, page='core.md')

        ast = self.tokenize('[core.md#bookmark]')
        self.assertSize(ast, 1)
        self.assertToken(ast(0), 'Paragraph', size=1)
        self.assertToken(ast(0)(0), 'AutoLink', size=0, bookmark='bookmark', exact=False, optional=False, page='core.md')

        ast = self.tokenize('[core.md exact=True]')
        self.assertSize(ast, 1)
        self.assertToken(ast(0), 'Paragraph', size=1)
        self.assertToken(ast(0)(0), 'AutoLink', size=0, bookmark=None, exact=True, optional=False, page='core.md')

        ast = self.tokenize('[core.md optional=True]')
        self.assertSize(ast, 1)
        self.assertToken(ast(0), 'Paragraph', size=1)
        self.assertToken(ast(0)(0), 'AutoLink', size=0, bookmark=None, exact=False, optional=True, page='core.md')

        ast = self.tokenize('[core.md#bookmark optional=True exact=True]')
        self.assertSize(ast, 1)
        self.assertToken(ast(0), 'Paragraph', size=1)
        self.assertToken(ast(0)(0), 'AutoLink', size=0, bookmark='bookmark', exact=True, optional=True, page='core.md')

    def testLocalLink(self):
        ast = self.tokenize('[#bookmark]')
        self.assertSize(ast, 1)
        self.assertToken(ast(0), 'Paragraph', size=1)
        self.assertToken(ast(0)(0), 'LocalLink', size=0, bookmark='bookmark')

        ast = self.tokenize('[#]')
        self.assertSize(ast, 1)
        self.assertToken(ast(0), 'Paragraph', size=1)
        self.assertToken(ast(0)(0), 'LocalLink', size=0, bookmark='')

    def testSourceLink(self):
        ast = self.tokenize('[framework/Makefile]')
        self.assertSize(ast, 2)
        self.assertToken(ast(0), 'Paragraph', size=1)
        self.assertToken(ast(0)(0), 'SourceLink', size=1)
        self.assertToken(ast(0)(0)(0), 'Link', string='Makefile')

        url = ast(0)(0)(0)['url']
        self.assertToken(ast(1), 'ModalLink', size=2, bookmark=url[1:])
        self.assertToken(ast(1)(0), 'ModalLinkTitle', string='/framework/Makefile')
        self.assertToken(ast(1)(1), 'ModalLinkContent', size=1)
        self.assertToken(ast(1)(1)(0), 'Code', size=0)

        # If a file is not given, it should fall through to a regular link
        ast = self.tokenize('[not_a_file]')
        self.assertSize(ast, 1)
        self.assertToken(ast(0), 'Paragraph', size=1)
        self.assertToken(ast(0)(0), 'ShortcutLink', size=0, key='not_a_file')

        with self.assertLogs(None, level='ERROR') as cm:
            ast = self.tokenize('[Makefile]')
        self.assertIn('Multiple files match the supplied filename Makefile', cm.output[0])

class TestLink(MooseDocsTestCase):
    EXTENSIONS = [core, floats, heading, autolink]

    def setupContent(self):
        config = [dict(root_dir='python/MooseDocs/test/content', content=['extensions/core.md'])]
        return common.get_content(config, '.md')

    def testAutoLink(self):
        ast = self.tokenize('[core](core.md)')
        self.assertSize(ast, 1)
        self.assertToken(ast(0), 'Paragraph', size=1)
        self.assertToken(ast(0)(0), 'AutoLink', size=1, bookmark=None, exact=False, optional=False, page='core.md')
        self.assertToken(ast(0)(0)(0), 'Word', size=0, content='core')

        ast = self.tokenize('[core](core.md#bookmark)')
        self.assertSize(ast, 1)
        self.assertToken(ast(0), 'Paragraph', size=1)
        self.assertToken(ast(0)(0), 'AutoLink', size=1, bookmark='bookmark', exact=False, optional=False, page='core.md')
        self.assertToken(ast(0)(0)(0), 'Word', size=0, content='core')

        ast = self.tokenize('[core](core.md exact=True)')
        self.assertSize(ast, 1)
        self.assertToken(ast(0), 'Paragraph', size=1)
        self.assertToken(ast(0)(0), 'AutoLink', size=1, bookmark=None, exact=True, optional=False, page='core.md')
        self.assertToken(ast(0)(0)(0), 'Word', size=0, content='core')

        ast = self.tokenize('[core](core.md optional=True)')
        self.assertSize(ast, 1)
        self.assertToken(ast(0), 'Paragraph', size=1)
        self.assertToken(ast(0)(0), 'AutoLink', size=1, bookmark=None, exact=False, optional=True, page='core.md')
        self.assertToken(ast(0)(0)(0), 'Word', size=0, content='core')

        ast = self.tokenize('[core](core.md#bookmark optional=True exact=True)')
        self.assertSize(ast, 1)
        self.assertToken(ast(0), 'Paragraph', size=1)
        self.assertToken(ast(0)(0), 'AutoLink', size=1, bookmark='bookmark', exact=True, optional=True, page='core.md')
        self.assertToken(ast(0)(0)(0), 'Word', size=0, content='core')

    def testLocalLink(self):
        ast = self.tokenize('[text](#bookmark)')
        self.assertSize(ast, 1)
        self.assertToken(ast(0), 'Paragraph', size=1)
        self.assertToken(ast(0)(0), 'LocalLink', size=1, bookmark='bookmark')
        self.assertToken(ast(0)(0)(0), 'Word', size=0, content='text')

        ast = self.tokenize('[text](#)')
        self.assertSize(ast, 1)
        self.assertToken(ast(0), 'Paragraph', size=1)
        self.assertToken(ast(0)(0), 'LocalLink', size=1, bookmark='')
        self.assertToken(ast(0)(0)(0), 'Word', size=0, content='text')

    def testSourceLink(self):
        ast = self.tokenize('[make](framework/Makefile)')
        self.assertSize(ast, 2)
        self.assertToken(ast(0), 'Paragraph', size=1)
        self.assertToken(ast(0)(0), 'SourceLink', size=1)
        self.assertToken(ast(0)(0)(0), 'Link', size=1)
        self.assertToken(ast(0)(0)(0)(0), 'Word', size=0, content='make')

        url = ast(0)(0)(0)['url']
        self.assertToken(ast(1), 'ModalLink', size=2, bookmark=url[1:])
        self.assertToken(ast(1)(0), 'ModalLinkTitle', string='/framework/Makefile')
        self.assertToken(ast(1)(1), 'ModalLinkContent', size=1)
        self.assertToken(ast(1)(1)(0), 'Code', size=0)

        # If a file is not given, it should fall through to a regular link
        ast = self.tokenize('[text](not_a_file)')
        self.assertSize(ast, 1)
        self.assertToken(ast(0), 'Paragraph', size=1)
        self.assertToken(ast(0)(0), 'Link', size=1, url='not_a_file')
        self.assertToken(ast(0)(0)(0), 'Word', size=0, content='text')

class TestAutoLinkRender(MooseDocsTestCase):
    EXTENSIONS = [core, floats, heading, autolink]

    def setupContent(self):
        config = [dict(root_dir='python/MooseDocs/test/content', content=['extensions/core.md'])]
        return common.get_content(config, '.md')

    def _assertHTML(self, a):
        self.assertHTMLTag(a, 'a', size=3, href='extensions/core.html')
        self.assertHTMLString(a(0), 'Core')
        self.assertHTMLString(a(1), ' ')
        self.assertHTMLString(a(2), 'Extension')

    def _assertLatex(self, a):
        self.assertLatex(a, 'Command', 'hyperref', size=3, nargs=1)
        self.assertLatexArg(a, 0, 'Bracket', string='core-extension')
        self.assertLatexString(a(0), 'Core')
        self.assertLatexString(a(1), ' ')
        self.assertLatexString(a(2), 'Extension')

    def testMinimal(self):

        link = autolink.AutoLink(None, page='core.md')

        res = self.render(link, renderer=base.HTMLRenderer())
        self.assertHTMLTag(res, 'body', size=1)
        self._assertHTML(res(0))

        res = self.render(link, renderer=base.MaterializeRenderer())
        self.assertHTMLTag(res, 'div', size=1)
        self._assertHTML(res(0))

        res = self.render(link, renderer=base.RevealRenderer())
        self.assertHTMLTag(res, 'div', size=1)
        self._assertHTML(res(0))

        res = self.render(link, renderer=base.LatexRenderer())
        self.assertSize(res, 1)
        self._assertLatex(res(0))

    def testExact(self):
        link = autolink.AutoLink(None, page='extensions/core.md', exact=True)

        res = self.render(link, renderer=base.HTMLRenderer())
        self.assertHTMLTag(res, 'body', size=1)
        self._assertHTML(res(0))

        res = self.render(link, renderer=base.MaterializeRenderer())
        self.assertHTMLTag(res, 'div', size=1)
        self._assertHTML(res(0))

        res = self.render(link, renderer=base.RevealRenderer())
        self.assertHTMLTag(res, 'div', size=1)
        self._assertHTML(res(0))

        res = self.render(link, renderer=base.LatexRenderer())
        self.assertSize(res, 1)
        self._assertLatex(res(0))

    @mock.patch.object(base.renderers.LOG, 'error')
    def testExactError(self, mock_log):
        link = autolink.AutoLink(None, page='core.md', exact=True)
        res = self.render(link, renderer=base.HTMLRenderer())

        # TODO: Use assertLogs in python 3.4
        self.assertIn("Unable to locate a page that ends with the name 'core.md'",
                      mock_log.call_args[0][0])

    def testOptional(self):
        link = autolink.AutoLink(None, page='not_a_file.md', optional=True)

        res = self.render(link, renderer=base.HTMLRenderer())
        self.assertHTMLTag(res, 'body', string='_text_')

        res = self.render(link, renderer=base.MaterializeRenderer())
        self.assertHTMLTag(res, 'div', string='_text_')

        res = self.render(link, renderer=base.RevealRenderer())
        self.assertHTMLTag(res, 'div', string='_text_')

        res = self.render(link, renderer=base.LatexRenderer())
        self.assertSize(res, 1)
        self.assertLatexString(res(0), '_text_')

    def testBookmark(self):
        link = autolink.AutoLink(None, page='core.md', bookmark='unordered-nested-lists')

        res = self.render(link, renderer=base.HTMLRenderer())
        self.assertHTMLTag(res, 'body', size=1)
        self.assertHTMLTag(res(0), 'a', size=3, href='extensions/core.html#unordered-nested-lists')
        self.assertHTMLString(res(0)(0), 'Nested')
        self.assertHTMLString(res(0)(1), ' ')
        self.assertHTMLString(res(0)(2), 'lists')

        res = self.render(link, renderer=base.MaterializeRenderer())
        self.assertHTMLTag(res, 'div', size=1)
        self.assertHTMLTag(res(0), 'a', size=3, href='extensions/core.html#unordered-nested-lists')
        self.assertHTMLString(res(0)(0), 'Nested')
        self.assertHTMLString(res(0)(1), ' ')
        self.assertHTMLString(res(0)(2), 'lists')

        res = self.render(link, renderer=base.RevealRenderer())
        self.assertHTMLTag(res, 'div', size=1)
        self.assertHTMLTag(res(0), 'a', size=3, href='extensions/core.html#unordered-nested-lists')
        self.assertHTMLString(res(0)(0), 'Nested')
        self.assertHTMLString(res(0)(1), ' ')
        self.assertHTMLString(res(0)(2), 'lists')

        res = self.render(link, renderer=base.LatexRenderer())
        self.assertSize(res, 1)
        self.assertLatex(res(0), 'Command', 'hyperref')
        self.assertLatexArg(res(0), 0, 'Bracket', 'unordered-nested-lists')
        self.assertLatexString(res(0)(0), 'Nested')
        self.assertLatexString(res(0)(1), ' ')
        self.assertLatexString(res(0)(2), 'lists')

    def testBookmarkWrong(self):
        link = autolink.AutoLink(None, page='core.md', bookmark='wrong')

        res = self.render(link, renderer=base.HTMLRenderer())
        self.assertHTMLTag(res, 'body', size=1)
        self.assertHTMLTag(res(0), 'a', string='extensions/core.html#wrong',
                           href='extensions/core.html#wrong', class_='moose-error')


        res = self.render(link, renderer=base.MaterializeRenderer())
        self.assertHTMLTag(res, 'div', size=1)
        self.assertHTMLTag(res(0), 'a', string='extensions/core.html#wrong',
                           href='extensions/core.html#wrong', class_='moose-error')

        res = self.render(link, renderer=base.RevealRenderer())
        self.assertHTMLTag(res, 'div', size=1)
        self.assertHTMLTag(res(0), 'a', string='extensions/core.html#wrong',
                           href='extensions/core.html#wrong', class_='moose-error')

        res = self.render(link, renderer=base.LatexRenderer())
        self.assertSize(res, 1)
        self.assertLatexString(res(0), '_text_')

class TestLocalLinkRender(MooseDocsTestCase):
    EXTENSIONS = [core, floats, heading, autolink]

    def setupContent(self):
        config = [dict(root_dir='python/MooseDocs/test/content', content=['extensions/core.md'])]
        return common.get_content(config, '.md')

    def testMinimal(self):

        link = autolink.LocalLink(None, bookmark='bookmark')
        res = self.render(link, renderer=base.HTMLRenderer())
        self.assertHTMLTag(res, 'body', size=1)
        self.assertHTMLTag(res(0), 'a', string='_text_#bookmark')

        res = self.render(link, renderer=base.MaterializeRenderer())
        self.assertHTMLTag(res, 'div', size=1)
        self.assertHTMLTag(res(0), 'a', string='_text_#bookmark')

        res = self.render(link, renderer=base.RevealRenderer())
        self.assertHTMLTag(res, 'div', size=1)
        self.assertHTMLTag(res(0), 'a', string='_text_#bookmark')

        # Heading is not founc (see testMinimalLatex)
        res = self.render(link, renderer=base.LatexRenderer())
        self.assertSize(res, 1)
        self.assertLatexString(res(0), '_text_')

    @mock.patch.object(heading, 'find_heading')
    def testMinimalLatex(self, mock_find_heading):
        head = core.Heading(None, level=1, id_='bookmark')
        core.Word(head, content='heading')
        mock_find_heading.return_value = head

        link = autolink.LocalLink(None, bookmark='bookmark')
        res = self.render(link, renderer=base.LatexRenderer())
        self.assertSize(res, 1)
        self.assertLatex(res(0), 'Command', 'hyperref')
        self.assertLatexArg(res(0), 0, 'Bracket', 'bookmark')
        self.assertLatexString(res(0)(0), 'heading')

class TestSourceLinkRender(MooseDocsTestCase):
    EXTENSIONS = [core, floats, heading, autolink]

    def setupContent(self):
        config = [dict(root_dir='python/MooseDocs/test/content', content=['extensions/core.md'])]
        return common.get_content(config, '.md')

    def testMinimalHTML(self):
        """SourceLink is just a container for other tokens; by itself it does nothing."""

        link = autolink.SourceLink(None)
        res = self.render(link, renderer=base.HTMLRenderer())
        self.assertSize(res, 0)
        res = self.render(link, renderer=base.MaterializeRenderer())
        self.assertSize(res, 0)
        res = self.render(link, renderer=base.RevealRenderer())
        self.assertSize(res, 0)

    def testMinimalLatex(self):
        """The link and floats stuff is dropped by the LaTeX renderer"""
        link = autolink.SourceLink(None)
        core.Link(link, string='Content', url='something')
        res = self.render(link, renderer=base.LatexRenderer())
        self.assertSize(res, 1)
        self.assertLatexString(res(0), 'Content')

if __name__ == '__main__':
    unittest.main(verbosity=2, buffer=False)
