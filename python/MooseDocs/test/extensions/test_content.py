#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import unittest
import logging
import re
from MooseDocs import common, base
from MooseDocs.test import MooseDocsTestCase
import MooseDocs.extensions as extensions
logging.basicConfig()

class TestContentList(MooseDocsTestCase):
    EXTENSIONS = [extensions.core, extensions.command, extensions.heading, extensions.content]

    def setupContent(self):
        config = [dict(root_dir='python/MooseDocs/test/content',
                       content=['extensions/core.md', 'extensions/content.md'])]
        return common.get_content(config, '.md')

    def testAST(self):
        ast = self.tokenize('!content list')
        self.assertSize(ast, 1)
        self.assertToken(ast(0), 'ContentToken', size=0, level=2, location='')

        ast = self.tokenize('!content list location=foo level=42')
        self.assertSize(ast, 1)
        self.assertToken(ast(0), 'ContentToken', size=0, level=42, location='foo')

    def testHTML(self):
        _, res = self.execute('!content list location=extensions')
        self.assertHTMLTag(res, 'body', size=1)
        self._assertHTML(res)

    def testMaterialize(self):
        _, res = self.execute('!content list location=extensions', renderer=base.MaterializeRenderer())
        self.assertHTMLTag(res, 'div', size=1)
        self._assertHTML(res)

    def _assertHTML(self, res):
        self.assertHTMLTag(res(0), 'div', size=3, class_='row')

        col = res(0)(0)
        self.assertHTMLTag(col, 'div', size=1, class_='col s12 m6 l4')
        self.assertHTMLTag(col(0), 'ul', size=1, class_='moose-a-to-z')
        self.assertHTMLTag(col(0)(0), 'li', size=1)
        self.assertHTMLTag(col(0)(0)(0), 'a', href='extensions/content.html', string='Content Extension')

        col = res(0)(1)
        self.assertHTMLTag(col, 'div', size=1, class_='col s12 m6 l4')
        self.assertHTMLTag(col(0), 'ul', size=1, class_='moose-a-to-z')
        self.assertHTMLTag(col(0)(0), 'li', size=1)
        self.assertHTMLTag(col(0)(0)(0), 'a', href='extensions/core.html', string='Core Extension')

        col = res(0)(2)
        self.assertHTMLTag(col, 'div', size=1, class_='col s12 m6 l4')
        self.assertHTMLTag(col(0), 'ul', size=0, class_='moose-a-to-z')

    def testLatex(self):
        _, res = self.execute('!content list location=extensions', renderer=base.LatexRenderer())
        self.assertSize(res, 4)

        self.assertLatex(res(0), 'Command' ,'par')
        self.assertLatex(res(1), 'Command', 'ContentItem', string='Content Extension')
        self.assertLatexArg(res(1), 0, 'Brace', 'extensions/content.tex')
        self.assertLatexArg(res(1), 1, 'Brace', 'content-extension')

        self.assertLatex(res(2), 'Command', 'ContentItem', string='Core Extension')
        self.assertLatexArg(res(2), 0, 'Brace', 'extensions/core.tex')
        self.assertLatexArg(res(2), 1, 'Brace', 'core-extension')
        self.assertLatex(res(3), 'Command' ,'par')

class TestContentAtoZ(MooseDocsTestCase):
    EXTENSIONS = [extensions.core, extensions.command, extensions.heading, extensions.content,
                  extensions.table, extensions.floats, extensions.acronym]

    def setupContent(self):
        config = [dict(root_dir='python/MooseDocs/test/content',
                       content=['extensions/core.md', 'extensions/content.md',
                                'extensions/acronym.md', 'extensions/table.md'])]
        return common.get_content(config, '.md')

    def setupExtension(self, ext):
        if ext == extensions.acronym:
            return dict(active=False)

    def testAST(self):
        ast = self.tokenize('!content a-to-z')
        self.assertSize(ast, 1)
        self.assertToken(ast(0), 'AtoZToken', size=0, level=2, location='', buttons=True)

        ast = self.tokenize('!content a-to-z location=foo level=42 buttons=False')
        self.assertSize(ast, 1)
        self.assertToken(ast(0), 'AtoZToken', size=0, level=42, location='foo', buttons=False)

    def testHTML(self):
        _, res = self.execute('!content a-to-z location=extensions')
        self.assertHTMLTag(res, 'body', size=6)
        self._assertHTML(res)

    def testMaterialize(self):
        _, res = self.execute('!content a-to-z location=extensions',
                              renderer=base.MaterializeRenderer())

        b = '0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ'
        self.assertHTMLTag(res(0), 'div', size=len(b), class_='moose-a-to-z-buttons')
        for i in range(0, len(b)):
            if b[i] in ['A', 'C', 'T']:
                self.assertHTMLTag(res(0)(i), 'a', class_='btn moose-a-to-z-button', string=b[i])
            else:
                self.assertHTMLTag(res(0)(i), 'a', class_='btn moose-a-to-z-button disabled', string=b[i])

        res(0).parent = None
        self.assertHTMLTag(res, 'div', size=6)
        self._assertHTML(res)

    def testLatex(self):
        _, res = self.execute('!content a-to-z location=extensions', renderer=base.LatexRenderer())
        self.assertSize(res, 8)
        self.assertLatex(res(0), 'Command' ,'par')
        self.assertLatex(res(1), 'Command', 'ContentItem', string='Acronym Extension')
        self.assertLatexArg(res(1), 0, 'Brace', 'extensions/acronym.tex')
        self.assertLatexArg(res(1), 1, 'Brace', 'acronym-extension')

        self.assertLatex(res(2), 'Command' ,'par')
        self.assertLatex(res(3), 'Command', 'ContentItem', string='Content Extension')
        self.assertLatexArg(res(3), 0, 'Brace', 'extensions/content.tex')
        self.assertLatexArg(res(3), 1, 'Brace', 'content-extension')

        self.assertLatex(res(4), 'Command', 'ContentItem', string='Core Extension')
        self.assertLatexArg(res(4), 0, 'Brace', 'extensions/core.tex')
        self.assertLatexArg(res(4), 1, 'Brace', 'core-extension')

        self.assertLatex(res(5), 'Command' ,'par')
        self.assertLatex(res(6), 'Command', 'ContentItem', string='Table Extensions')
        self.assertLatexArg(res(6), 0, 'Brace', 'extensions/table.tex')
        self.assertLatexArg(res(6), 1, 'Brace', 'table-extensions')

    def _assertHTML(self, res):

        # A
        self.assertHTMLTag(res(0), 'h2', string='a', class_='moose-a-to-z')
        row = res(1)
        self._assertCols(row, [1,0,0])

        col = row(0)
        self.assertHTMLTag(col(0)(0), 'li', size=1)
        self.assertHTMLTag(col(0)(0)(0), 'a', href='extensions/acronym.html', string='Acronym Extension')

        # C
        self.assertHTMLTag(res(2), 'h2', string='c', class_='moose-a-to-z')
        row = res(3)
        self._assertCols(row, [1,1,0])

        col = row(0)
        self.assertHTMLTag(col(0)(0), 'li', size=1)
        self.assertHTMLTag(col(0)(0)(0), 'a', href='extensions/content.html', string='Content Extension')

        col = row(1)
        self.assertHTMLTag(col(0)(0), 'li', size=1)
        self.assertHTMLTag(col(0)(0)(0), 'a', href='extensions/core.html', string='Core Extension')

        # T
        self.assertHTMLTag(res(4), 'h2', string='t', class_='moose-a-to-z')
        row = res(5)
        self._assertCols(row, [1,0,0])

        col = row(0)
        self.assertHTMLTag(col(0)(0), 'li', size=1)
        self.assertHTMLTag(col(0)(0)(0), 'a', href='extensions/table.html', string='Table Extensions')

    def _assertCols(self, row, sz):
        self.assertHTMLTag(row, 'div', size=3, class_='row')
        for i in range(0, 2):
            self.assertHTMLTag(row(i), 'div', size=1, class_='col s12 m6 l4')
            self.assertHTMLTag(row(i)(0), 'ul', size=sz[i], class_='moose-a-to-z')

class TestContentToc(MooseDocsTestCase):
    EXTENSIONS = [extensions.core, extensions.command, extensions.heading, extensions.content]
    TEXT = '# TOC id=toc\n\n!content toc hide=toc levels=1 2\n\n## One\n\n## Two'

    def testAST(self):
        ast = self.tokenize(self.TEXT)
        self.assertSize(ast, 4)
        self.assertToken(ast(1), 'TableOfContents', size=0, levels=[1, 2], columns=1)

    def testHTML(self):
        _, res = self.execute(self.TEXT)
        self.assertHTMLTag(res, 'body', size=4)
        self._assertHTML(res(1))

    def testMaterialize(self):
        _, res = self.execute(self.TEXT, renderer=base.MaterializeRenderer())
        self.assertHTMLTag(res, 'div', size=4)
        self._assertHTML(res(1))

    def _assertHTML(self, res):

        self.assertHTMLTag(res, 'div', size=2, class_='moose-table-of-contents')
        self.assertHTMLTag(res(0), 'a', size=2, href='#one')
        self.assertHTMLString(res(0)(0), content='One')
        self.assertHTMLTag(res(0)(1), 'br', close=False)

        self.assertHTMLTag(res(1), 'a', size=2, href='#two')
        self.assertHTMLString(res(1)(0), content='Two')
        self.assertHTMLTag(res(1)(1), 'br', close=False)

class TestContentOutline(MooseDocsTestCase):
    EXTENSIONS = [extensions.core, extensions.command, extensions.heading, extensions.content]

    ### TEST CASE 1: OUTLINE DIRECTORY ###
    TEXT = ['!content outline location=extensions max_level=2 hide=outline-directory']
    HEADINGS = [{
        'Config Extension': {'page': 'config', 'id': 'config-extension', 'level': 1},
        'Content Extension': {'page': 'content', 'id': 'content-extension', 'level': 1},
        'Table of Contents': {'page': 'content', 'id': 'table-of-contents', 'level': 2},
        'A-to-Z Index': {'page': 'content', 'id': 'a-to-z-index', 'level': 2},
        'Page List': {'page': 'content', 'id': 'page-list', 'level': 2},
        'Some Content for TOC': {'page': 'content', 'id': 'some-content-for-toc', 'level': 2},
        'Outline Pages': {'page': 'content', 'id': 'outline-pages', 'level': 2},
        'Next/Previous (Pagination)': {'page': 'content', 'id': 'next-previous-pagination', 'level': 2},
        'Core Extension': {'page': 'core', 'id': 'core-extension', 'level': 1},
        'Code Blocks': {'page': 'core', 'id': 'code-blocks', 'level': 2},
        'Quotations': {'page': 'core', 'id': 'quotations', 'level': 2},
        'Headings': {'page': 'core', 'id': 'headings', 'level': 2},
        'One': {'page': 'core', 'id': 'one', 'level': 1},
        'Two': {'page': 'core', 'id': 'two', 'level': 2},
        'Heading with Style': {'page': 'core', 'id': 'heading-with-style', 'level': 2},
        'Unordered Lists': {'page': 'core', 'id': 'unordered-lists', 'level': 2},
        'Ordered List': {'page': 'core', 'id': 'ordered-list', 'level': 2},
        'Shortcuts and Shortcut links': {'page': 'core', 'id': 'shortcuts-and-shortcut-links', 'level': 2},
        'Inline formatting': {'page': 'core', 'id': 'inline-formatting', 'level': 2},
        'Links': {'page': 'core', 'id': 'links', 'level': 2},
        'Material Icon': {'page': 'materialicon', 'id': 'material-icon', 'level': 1}
    }]

    ### TEST CASE 2: OUTLINE DIRECTORY RECURSIVELY ###
    TEXT.append('!content outline location=extensions recursive=True')
    HEADINGS.append({
        'Config Extension': {'page': 'config', 'id': 'config-extension', 'level': 1},
        'Content Extension': {'page': 'content', 'id': 'content-extension', 'level': 1},
        'Core Extension': {'page': 'core', 'id': 'core-extension', 'level': 1},
        'One': {'page': 'core', 'id': 'one', 'level': 1},
        'Folder': {'page': 'folder/index', 'id': 'folder', 'level': 1},
        'Material Icon': {'page': 'materialicon', 'id': 'material-icon', 'level': 1}
    })

    ### TEST CASE 3: OUTLINE PAGES ###
    TEXT.append('!content outline max_level=6\n'
                '                 hide=outline-pages\n'
                '                 pages=core.md materialicon.md content.md')
    HEADINGS.append({
        'Core Extension': {'page': 'core', 'id': 'core-extension', 'level': 1},
        'Code Blocks': {'page': 'core', 'id': 'code-blocks', 'level': 2},
        'Quotations': {'page': 'core', 'id': 'quotations', 'level': 2},
        'Multiline Quotation': {'page': 'core', 'id': 'multiline-quotation', 'level': 3},
        'Nested Quotations and Lists': {'page': 'core', 'id': 'nested-quotations-and-lists', 'level': 3},
        'Headings': {'page': 'core', 'id': 'headings', 'level': 2},
        'One': {'page': 'core', 'id': 'one', 'level': 1},
        'Two': {'page': 'core', 'id': 'two', 'level': 2},
        'Three': {'page': 'core', 'id': 'three', 'level': 3},
        'Four': {'page': 'core', 'id': 'four', 'level': 4},
        'Five': {'page': 'core', 'id': 'five', 'level': 5},
        'Six': {'page': 'core', 'id': 'six', 'level': 6},
        'Heading with Style': {'page': 'core', 'id': 'heading-with-style', 'level': 2},
        'Unordered Lists': {'page': 'core', 'id': 'unordered-lists', 'level': 2},
        'Single level lists': {'page': 'core', 'id': 'unordered-single-level-lists', 'level': 3},
        'Nested lists': {'page': 'core', 'id': 'unordered-nested-lists', 'level': 3},
        'Ordered List': {'page': 'core', 'id': 'ordered-list', 'level': 2},
        '`Single level lists': {'page': 'core', 'id': 'ordered-single-level-lists', 'level': 3},
        'Starting number': {'page': 'core', 'id': 'starting-number', 'level': 3},
        '`Nested lists': {'page': 'core', 'id': 'ordered-nested-lists', 'level': 3},
        'Shortcuts and Shortcut links': {'page': 'core', 'id': 'shortcuts-and-shortcut-links', 'level': 2},
        'Inline formatting': {'page': 'core', 'id': 'inline-formatting', 'level': 2},
        'Links': {'page': 'core', 'id': 'links', 'level': 2},
        'Skip the level': {'page': 'core', 'id': 'skip-the-level', 'level': 4},
        'Material Icon': {'page': 'materialicon', 'id': 'material-icon', 'level': 1},
        'Content Extension': {'page': 'content', 'id': 'content-extension', 'level': 1},
        'Table of Contents': {'page': 'content', 'id': 'table-of-contents', 'level': 2},
        'A-to-Z Index': {'page': 'content', 'id': 'a-to-z-index', 'level': 2},
        'Page List': {'page': 'content', 'id': 'page-list', 'level': 2},
        'Some Content for TOC': {'page': 'content', 'id': 'some-content-for-toc', 'level': 2},
        'TOC 1': {'page': 'content', 'id': 'toc-1', 'level': 3},
        'TOC 2': {'page': 'content', 'id': 'toc-2', 'level': 3},
        'Outline Directory': {'page': 'content', 'id': 'outline-directory', 'level': 2},
        'Next/Previous (Pagination)': {'page': 'content', 'id': 'next-previous-pagination', 'level': 2},
        'Buttons with Page Titles': {'page': 'content', 'id': 'buttons-with-page-titles', 'level': 3},
    })

    def setupContent(self):
        names = ['core.md', 'materialicon.md', 'content.md', 'config.md', 'folder/index.md']
        config = [dict(root_dir='python/MooseDocs/test/content',
                       content=['extensions/' + name for name in names])]
        return common.get_content(config, '.md')

    def testAST(self):
        ast = self.tokenize(self.TEXT[0])
        self.assertSize(ast, 1)
        self.assertToken(ast(0), 'ContentOutline', location='extensions', recursive=False, pages=[],
                         max_level=2, hide=['outline-directory'], no_prefix=[], no_count=[])

        ast = self.tokenize(self.TEXT[1])
        self.assertSize(ast, 1)
        self.assertToken(ast(0), 'ContentOutline', location='extensions', recursive=True, pages=[],
                         max_level=1, hide=[], no_prefix=[], no_count=[])

        ast = self.tokenize(self.TEXT[2])
        self.assertSize(ast, 1)
        self.assertToken(ast(0), 'ContentOutline', location=None, recursive=False,
                         pages=['core.md', 'materialicon.md', 'content.md'], max_level=6,
                         hide=['outline-pages'], no_prefix=[], no_count=[])

    def testHTML(self):
        for case in range(len(self.TEXT)):
            _, res = self.execute(self.TEXT[case])
            self.assertHTMLTag(res, 'body', size=1)
            self._assertHTML(res(0), case)

    def testMaterialize(self):
        for case in range(len(self.TEXT)):
            res = self.render(self.tokenize(self.TEXT[case]), renderer=base.MaterializeRenderer())
            self.assertHTMLTag(res, 'div', class_='moose-content')
            self._assertHTML(res(0), case)

    def _assertHTML(self, res, case):
        self.assertHTMLTag(res, 'div', class_='moose-outline', size=1)

        # all tags are tested, so no need to specify size when calling assertHTMLTag()
        outline = res(0)
        self.assertHTMLTag(outline, 'ol')

        # loop through HEADINGS and assert html tags for each
        previous = 1; indices = '(0)'; count = [0 for i in range(6)];
        for key, head in self.HEADINGS[case].items():
            # initiate level counters
            current = head['level']
            diff = current - previous

            # determine MooseTree indices for current header's `li` tag
            if diff == 0:
                indices = indices[:-3] + '(' + str(count[current - 1]) + ')'

            elif diff > 0:
                # new `ol` tag follows an `a` tag, both inherit from a `li`
                count[current - diff] = 0
                self.assertHTMLTag(eval('outline' + indices + '(1)'), 'ol')
                indices += '(1)(0)'

                # new 'ol' and 'li' tags generated for each skipped level
                if diff > 1:
                    self.assertHTMLTag(eval('outline' + indices), 'li')
                    for j in range(diff - 1):
                        count[current - j - 1] = 0
                        self.assertHTMLTag(eval('outline' + indices + '(0)'), 'ol')
                        indices += '(0)(0)'
                        self.assertHTMLTag(eval('outline' + indices), 'li')

            else:
                revise = 6 * diff - 3
                indices = indices[:revise] + '(' + str(count[current - 1]) + ')'

            # assert list item and url for current header
            self.assertHTMLTag(eval('outline' + indices), 'li')
            href = 'extensions/' + head['page'] + '.html#' + head['id']
            self.assertHTMLTag(eval('outline' + indices + '(0)'), 'a', href=href)
            title = re.findall("[\w]+|[.,!?;:\"'%&@$^*+-=~_\|\/<>(){}\[\]]|[\s]", key)
            for i, s in enumerate(title):
                index = '(0)(' + str(i) + ')'
                self.assertHTMLString(eval('outline' + indices + index), content=s)

            # swap and update level counters
            previous = current
            count[current - 1] += 1

class testContentPagination(MooseDocsTestCase):
    EXTENSIONS = [extensions.core, extensions.command, extensions.heading, extensions.content,
                  extensions.materialicon, extensions.config]
    PREV = 'materialicon.md'
    NEXT = 'config.md'
    MARGINS = [['10.2%', '-2pt'], ['24px', '24px']] # second test uses default '24px' margins
    TEXT = ('!content pagination previous=' + PREV + ' next=' + NEXT
            + ' margin-top=' + MARGINS[0][0] + ' margin-bottom=' + MARGINS[0][1] + '\n\n'
            '!content pagination previous=' + PREV + ' next=' + NEXT + ' use_title=True')

    def setupContent(self):
        config = [dict(root_dir='python/MooseDocs/test/content',
                       content=['extensions/' + self.PREV, 'extensions/' + self.NEXT])]
        content = common.get_content(config, '.md')
        return content

    def setupExtension(self, ext):
        if ext in [extensions.materialicon, extensions.config]:
            return dict(active=False)

    def testAST(self):
        ast = self.tokenize(self.TEXT)
        self.assertSize(ast, 2)
        tok = 'PaginationToken'
        self.assertToken(ast(0), tok, previous=self.PREV, next=self.NEXT, use_title=False,
                         margins=self.MARGINS[0])
        self.assertToken(ast(1), tok, previous=self.PREV, next=self.NEXT, use_title=True,
                         margins=self.MARGINS[1])

    def testHTML(self):
        _, res = self.execute(self.TEXT)
        self.assertHTMLTag(res, 'body', size=2)

        prev = 'extensions/' + self.PREV[:-2] + 'html'
        next = 'extensions/' + self.NEXT[:-2] + 'html'

        self.assertHTMLTag(res(0), 'div', size=2, class_='moose-content-pagination',
                           style='margin-top:{};margin-bottom:{};'.format(*self.MARGINS[0]))
        self.assertHTMLTag(res(0)(0), 'a', size=1, class_='moose-content-previous', href=prev)
        self.assertHTMLString(res(0)(0)(0), content='Previous')
        self.assertHTMLTag(res(0)(1), 'a', size=1, class_='moose-content-next', href=next)
        self.assertHTMLString(res(0)(1)(0), content='Next')

        self.assertHTMLTag(res(1), 'div', size=2, class_='moose-content-pagination',
                           style='margin-top:{};margin-bottom:{};'.format(*self.MARGINS[1]))
        self.assertHTMLTag(res(1)(0), 'a', size=1, class_='moose-content-previous', href=prev)
        self.assertHTMLString(res(1)(0)(0), content='Material Icon')
        self.assertHTMLTag(res(1)(1), 'a', size=1, class_='moose-content-next', href=next)
        self.assertHTMLString(res(1)(1)(0), content='Config Extension')

    def testMaterialize(self):
        _, res = self.execute(self.TEXT, renderer=base.MaterializeRenderer())
        self.assertHTMLTag(res, 'div', size=2, class_='moose-content')

        prev = 'extensions/' + self.PREV[:-2] + 'html'
        next = 'extensions/' + self.NEXT[:-2] + 'html'

        self.assertHTMLTag(res(0), 'div', size=2, class_='moose-content-pagination',
                           style='margin-top:{};margin-bottom:{};'.format(*self.MARGINS[0]))
        self.assertHTMLTag(res(0)(0), 'a', size=2, class_='moose-content-previous btn', href=prev)
        self.assertHTMLString(res(0)(0)(0), content='Previous')
        self.assertHTMLTag(res(0)(0)(1), 'i', size=1, class_='material-icons left')
        self.assertHTMLString(res(0)(0)(1)(0), content='arrow_back')
        self.assertHTMLTag(res(0)(1), 'a', size=2, class_='moose-content-next btn', href=next)
        self.assertHTMLString(res(0)(1)(0), content='Next')
        self.assertHTMLTag(res(0)(1)(1), 'i', size=1, class_='material-icons right')
        self.assertHTMLString(res(0)(1)(1)(0), content='arrow_forward')

        self.assertHTMLTag(res(1), 'div', size=2, class_='moose-content-pagination',
                           style='margin-top:{};margin-bottom:{};'.format(*self.MARGINS[1]))
        self.assertHTMLTag(res(1)(0), 'a', size=2, class_='moose-content-previous btn', href=prev)
        self.assertHTMLString(res(1)(0)(0), content='Material Icon')
        self.assertHTMLTag(res(1)(0)(1), 'i', size=1, class_='material-icons left')
        self.assertHTMLString(res(1)(0)(1)(0), content='arrow_back')
        self.assertHTMLTag(res(1)(1), 'a', size=2, class_='moose-content-next btn', href=next)
        self.assertHTMLString(res(1)(1)(0), content='Config Extension')
        self.assertHTMLTag(res(1)(1)(1), 'i', size=1, class_='material-icons right')
        self.assertHTMLString(res(1)(1)(1)(0), content='arrow_forward')

if __name__ == '__main__':
    unittest.main(verbosity=2)
