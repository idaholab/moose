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
from MooseDocs import MOOSE_DIR, common, base
from MooseDocs.test import MooseDocsTestCase
from MooseDocs.extensions import core, shortcut, command, floats, include, listing, modal
logging.basicConfig()

def extractContent(filename, opts=dict()):
    """Helper for reading contents of a file in order to make assertions."""
    settings = common.get_settings_as_dict(common.extractContentSettings())
    settings.update(opts)
    content, _ = common.extractContent(common.read(MOOSE_DIR + '/' + filename), settings)
    return content

class TestListingNumbers(MooseDocsTestCase):
    EXTENSIONS = [core, shortcut, command, floats, include, listing, modal]

    # also testing a listing from an '!include' file and referencing its id number chronologically
    SHORTCUTS = ['one', 'two', 'three']
    TEXT = (('!listing id=one\nOne\n\n'
             '!include listing_include.md\n\n'
             '!listing id=three\nThree\n\n')
            + ''.join(['[' + s + ']' for s in SHORTCUTS]))

    def setupContent(self):
        config = [dict(root_dir='python/MooseDocs/test/content',
                       content=['extensions/listing_include.md'])]
        return common.get_content(config, '.md')

    def testAST(self):
        ast = self.tokenize(self.TEXT)
        self.assertSize(ast, 7)

        for i, s in enumerate(self.SHORTCUTS):
            self.assertToken(ast(i), 'Listing', size=2)
            self.assertToken(ast(i,0), 'FloatCaption', key=s, prefix='Listing', number=i + 1)
            self.assertToken(ast(i,1), 'ListingCode', content=s.capitalize(),
                             style='max-height:350px;', language=None)

        self.assertToken(ast(3), 'Paragraph', size=3)
        for i, j, s in zip([0, 1, 2], [4, 5, 6], self.SHORTCUTS):
            self.assertToken(ast(3,i), 'ShortcutLink', key=s)
            self.assertToken(ast(j), 'Shortcut', size=1, key=s, link='#' + s,
                             string='Listing ' + str(i + 1))

    def testHTML(self):
        _, res = self.execute(self.TEXT)
        self.assertHTMLTag(res, 'body', size=4)

        self.assertHTMLTag(res(3), 'p', size=3)
        for i, s in enumerate(self.SHORTCUTS):
            self.assertHTMLTag(res(i), 'div', size=2, class_='moose-float-div')
            self._assertHTML(res(i,0), res(i,1), res(3,i), 'Listing ' + str(i + 1), s)

    def testMaterialize(self):
        _, res = self.execute(self.TEXT, renderer=base.MaterializeRenderer())
        self.assertHTMLTag(res, 'div', size=4, class_='moose-content')

        self.assertHTMLTag(res(3), 'p', size=3)
        for i, s in enumerate(self.SHORTCUTS):
            self.assertHTMLTag(res(i), 'div', size=1, class_='card moose-float')
            self.assertHTMLTag(res(i,0), 'div', size=2, class_='card-content')
            self._assertHTML(res(i,0,0), res(i,0,1), res(3,i), 'Listing ' + str(i + 1), s)

    def _assertHTML(self, p, pre, a, content, ref):
        self.assertHTMLTag(p, 'p', size=2, class_='moose-caption')
        self.assertHTMLTag(p(0), 'span', size=1, class_='moose-caption-heading')
        self.assertHTMLString(p(0,0), content=content + ': ')
        self.assertHTMLTag(p(1), 'span', class_='moose-caption-text')

        self.assertHTMLTag(pre, 'pre', size=1, class_='moose-pre', style='max-height:350px;')
        self.assertHTMLTag(pre(0), 'code', size=1, class_='language-None')
        self.assertHTMLString(pre(0,0), content=ref.capitalize())

        self.assertHTMLTag(a, 'a', size=1, href='#' + ref)
        self.assertHTMLString(a(0), content=content)

    def testLatex(self):
        ast, res = self.execute(self.TEXT, renderer=base.LatexRenderer())

        self.assertSize(res, 13)
        for i, j, s in zip([0, 2, 4], [1, 3, 5], self.SHORTCUTS):
            self.assertLatexEnvironment(res(i), 'lstlisting', size=1, escape=False,
                                        after_begin='\n', before_end='\n', info=ast.info)
            self.assertLatexArg(res(i), 0, 'Bracket', size=3)
            self.assertLatexString(res(i)['args'][0](0), content='label=' + s + ',')
            self.assertLatexString(res(i)['args'][0](1), content='caption=')
            self.assertLatexString(res(i)['args'][0](2), content='\\mbox{}', escape=False)
            self.assertLatexString(res(i,0), content=s.capitalize())
            self.assertLatexEnvironment(res(j), 'lstlisting', size=1, escape=False,
                                        after_begin='\n', before_end='\n', info=ast.info)
            self.assertLatexString(res(j,0), content=s.capitalize())

        self.assertLatexCommand(res(6), 'par')
        for i, j, s in zip([7, 9, 11], [8, 10, 12], self.SHORTCUTS):
            self.assertLatexString(res(i), content='Listing~')
            self.assertLatexCommand(res(j), 'ref', size=1)
            self.assertLatexString(res(j,0), content=s)

class TestListingCaptions(MooseDocsTestCase):
    EXTENSIONS = [core, shortcut, command, floats, listing, modal]
    CODE = ['no caption',
            'caption with inline content',
            'caption with prefix and number']
    TEXT = ['!listing\n' + CODE[0],
            '!listing caption=the +caption+\n' + CODE[1],
            '!listing prefix=File caption=the =caption= id=file1\n' + CODE[2] + '\n\n[file1]']

    def testAST(self):
        ast = self.tokenize(self.TEXT[0])
        self.assertSize(ast, 1)
        self.assertToken(ast(0), 'Code', content=self.CODE[0], style='max-height:350px;',
                         language=None)

        ast = self.tokenize(self.TEXT[1])
        self.assertSize(ast, 1)
        self.assertToken(ast(0), 'Listing', size=2)
        self.assertToken(ast(0,0), 'FloatCaption', size=3, key='', prefix='', number=1)
        self.assertToken(ast(0,0,0), 'Word', content='the')
        self.assertToken(ast(0,0,1), 'Space', count=1)
        self.assertToken(ast(0,0,2), 'Strong', size=1)
        self.assertToken(ast(0,0,2,0), 'Word', content='caption')
        self.assertToken(ast(0,1), 'ListingCode', content=self.CODE[1], style='max-height:350px;',
                         language=None)

        ast = self.tokenize(self.TEXT[2])
        self.assertSize(ast, 3)
        self.assertToken(ast(0), 'Listing', size=2)
        self.assertToken(ast(0,0), 'FloatCaption', size=3, key='file1', prefix='File', number=1)
        self.assertToken(ast(0,0,0), 'Word', content='the')
        self.assertToken(ast(0,0,1), 'Space', count=1)
        self.assertToken(ast(0,0,2), 'Underline', size=1)
        self.assertToken(ast(0,0,2,0), 'Word', content='caption')
        self.assertToken(ast(0,1), 'ListingCode', content=self.CODE[2], style='max-height:350px;',
                         language=None)
        self.assertToken(ast(1), 'Paragraph', size=1)
        self.assertToken(ast(1,0), 'ShortcutLink', key='file1')
        self.assertToken(ast(2), 'Shortcut', size=1, key='file1', link='#file1', string='File 1')

    def testHTML(self):
        _, res = self.execute(self.TEXT[0])
        self.assertHTMLTag(res, 'body', size=1)
        self._assertHTML(res(0), self.CODE[0])

        _, res = self.execute(self.TEXT[1])
        self.assertHTMLTag(res, 'body', size=1)
        self.assertHTMLTag(res(0), 'div', size=2, class_='moose-float-div')
        self.assertHTMLTag(res(0,0), 'p', size=1, class_='moose-caption')
        self.assertHTMLTag(res(0,0,0), 'span', size=3, class_='moose-caption-text')
        self.assertHTMLString(res(0,0,0,0), content='the')
        self.assertHTMLString(res(0,0,0,1), content=' ')
        self.assertHTMLTag(res(0,0,0,2), 'strong', size=1)
        self.assertHTMLString(res(0,0,0,2,0), content='caption')
        self._assertHTML(res(0,1), self.CODE[1])

        _, res = self.execute(self.TEXT[2])
        self.assertHTMLTag(res, 'body', size=2)
        self.assertHTMLTag(res(0), 'div', size=2, class_='moose-float-div')
        self.assertHTMLTag(res(0,0), 'p', size=2, class_='moose-caption')
        self.assertHTMLTag(res(0,0,0), 'span', size=1, class_='moose-caption-heading')
        self.assertHTMLString(res(0,0,0,0), content='File 1: ')
        self.assertHTMLTag(res(0,0,1), 'span', size=3, class_='moose-caption-text')
        self.assertHTMLString(res(0,0,1,0), content='the')
        self.assertHTMLString(res(0,0,1,1), content=' ')
        self.assertHTMLTag(res(0,0,1,2), 'u', size=1)
        self.assertHTMLString(res(0,0,1,2,0), content='caption')
        self._assertHTML(res(0,1), self.CODE[2])
        self.assertHTMLTag(res(1), 'p', size=1)
        self.assertHTMLTag(res(1,0), 'a', size=1, href='#file1')
        self.assertHTMLString(res(1,0,0), content='File 1')

    def testMaterialize(self):
        _, res = self.execute(self.TEXT[0], renderer=base.MaterializeRenderer())
        self.assertHTMLTag(res, 'div', size=1, class_='moose-content')
        self._assertHTML(res(0), self.CODE[0])

        _, res = self.execute(self.TEXT[1], renderer=base.MaterializeRenderer())
        self.assertHTMLTag(res, 'div', size=1, class_='moose-content')
        self.assertHTMLTag(res(0), 'div', size=1, class_='card moose-float')
        self.assertHTMLTag(res(0,0), 'div', size=2, class_='card-content')
        self.assertHTMLTag(res(0,0,0), 'p', size=1, class_='moose-caption')
        self.assertHTMLTag(res(0,0,0,0), 'span', size=3, class_='moose-caption-text')
        self.assertHTMLString(res(0,0,0,0,0), content='the')
        self.assertHTMLString(res(0,0,0,0,1), content=' ')
        self.assertHTMLTag(res(0,0,0,0,2), 'strong', size=1)
        self.assertHTMLString(res(0,0,0,0,2,0), content='caption')
        self._assertHTML(res(0,0,1), self.CODE[1])

        _, res = self.execute(self.TEXT[2], renderer=base.MaterializeRenderer())
        self.assertHTMLTag(res, 'div', size=2, class_='moose-content')
        self.assertHTMLTag(res(0), 'div', size=1, class_='card moose-float')
        self.assertHTMLTag(res(0,0), 'div', size=2, class_='card-content')
        self.assertHTMLTag(res(0,0,0), 'p', size=2, class_='moose-caption')
        self.assertHTMLTag(res(0,0,0,0), 'span', size=1, class_='moose-caption-heading')
        self.assertHTMLString(res(0,0,0,0,0), content='File 1: ')
        self.assertHTMLTag(res(0,0,0,1), 'span', size=3, class_='moose-caption-text')
        self.assertHTMLString(res(0,0,0,1,0), content='the')
        self.assertHTMLString(res(0,0,0,1,1), content=' ')
        self.assertHTMLTag(res(0,0,0,1,2), 'u', size=1)
        self.assertHTMLString(res(0,0,0,1,2,0), content='caption')
        self._assertHTML(res(0,0,1), self.CODE[2])
        self.assertHTMLTag(res(1), 'p', size=1)
        self.assertHTMLTag(res(1,0), 'a', size=1, href='#file1')
        self.assertHTMLString(res(1,0,0), content='File 1')

    def _assertHTML(self, res, content):
        self.assertHTMLTag(res, 'pre', size=1, class_='moose-pre', style='max-height:350px;')
        self.assertHTMLTag(res(0), 'code', size=1, class_='language-None')
        self.assertHTMLString(res(0,0), content=content)

    def testLatex(self):
        ast, res = self.execute(self.TEXT[0], renderer=base.LatexRenderer())
        self.assertSize(res, 1)
        self.assertLatexEnvironment(res(0), 'verbatim', size=1, escape=False, after_begin='\n',
                                    before_end='\n', info=ast.info)
        self.assertLatexString(res(0,0), content=self.CODE[0])

        ast, res = self.execute(self.TEXT[1], renderer=base.LatexRenderer())
        self.assertSize(res, 2)
        self.assertLatexEnvironment(res(0), 'lstlisting', size=1, escape=False, after_begin='\n',
                                    before_end='\n', info=ast.info)
        self.assertLatexArg(res(0), 0, 'Bracket', size=2)
        self.assertLatexString(res(0)['args'][0](0), content='title=')
        self.assertLatex(res(0)['args'][0](1), 'Brace', 'Brace', size=3)
        self.assertLatexString(res(0)['args'][0](1)(0), content='the')
        self.assertLatexString(res(0)['args'][0](1)(1), content=' ')
        self.assertLatexCommand(res(0)['args'][0](1)(2), 'textbf', size=1)
        self.assertLatexString(res(0)['args'][0](1)(2)(0), content='caption')
        self.assertLatexString(res(0,0), content=self.CODE[1])
        self.assertLatexEnvironment(res(1), 'lstlisting', size=1, escape=False, after_begin='\n',
                                    before_end='\n', info=ast.info)
        self.assertLatexArg(res(1), 0, 'Bracket')
        self.assertLatexString(res(1,0), content=self.CODE[1])

        ast, res = self.execute(self.TEXT[2], renderer=base.LatexRenderer())
        self.assertSize(res, 5)
        self.assertLatexEnvironment(res(0), 'lstlisting', size=1, escape=False, after_begin='\n',
                                    before_end='\n', info=ast.info)
        self.assertLatexArg(res(0), 0, 'Bracket', size=3)
        self.assertLatexString(res(0)['args'][0](0), content='label=file1,')
        self.assertLatexString(res(0)['args'][0](1), content='caption=')
        self.assertLatex(res(0)['args'][0](2), 'Brace', 'Brace', size=3)
        self.assertLatexString(res(0)['args'][0](2)(0), content='the')
        self.assertLatexString(res(0)['args'][0](2)(1), content=' ')
        self.assertLatexCommand(res(0)['args'][0](2)(2), 'ul', size=1)
        self.assertLatexString(res(0)['args'][0](2)(2)(0), content='caption')
        self.assertLatexString(res(0,0), content=self.CODE[2])
        self.assertLatexEnvironment(res(1), 'lstlisting', size=1, escape=False, after_begin='\n',
                                    before_end='\n', info=ast.info)
        self.assertLatexArg(res(1), 0, 'Bracket')
        self.assertLatexString(res(1,0), content=self.CODE[2])
        self.assertLatexCommand(res(2), 'par')
        self.assertLatexString(res(3), content='File~')
        self.assertLatexCommand(res(4), 'ref', size=1)
        self.assertLatexString(res(4,0), content='file1')

class TestListingWithSpace(MooseDocsTestCase):
    EXTENSIONS = [core, command, floats, listing, modal]

    # tests !listing! command for rendering code with empty lines and also setting max-height
    CODE = ('#include <stdio.h>\n\n'
            'void\n'
            'greeting(void)\n'
            '{\n'
            '  printf("Hello, World!\\n");\n'
            '}\n\n'
            'int\n'
            'main(int argc, char **argv)\n'
            '{\n'
            '  greeting();\n'
            '}\n')
    TEXT = ('!listing! max-height=92px\n'
            + CODE
            + '!listing-end!')

    def testAST(self):
        ast = self.tokenize(self.TEXT)
        self.assertSize(ast, 1)
        self.assertToken(ast(0), 'Code', content=self.CODE, style='max-height:92px;', language=None)

    def testHTML(self):
        _, res = self.execute(self.TEXT)
        self.assertHTMLTag(res, 'body', size=1)
        self._assertHTML(res(0), self.CODE)

    def testMaterialize(self):
        _, res = self.execute(self.TEXT, renderer=base.MaterializeRenderer())
        self.assertHTMLTag(res, 'div', size=1, class_='moose-content')
        self._assertHTML(res(0), self.CODE)

    def _assertHTML(self, res, content):
        self.assertHTMLTag(res, 'pre', size=1, class_='moose-pre', style='max-height:92px;')
        self.assertHTMLTag(res(0), 'code', size=1, class_='language-None')
        self.assertHTMLString(res(0,0), content=content)

    def testLatex(self):
        ast, res = self.execute(self.TEXT, renderer=base.LatexRenderer())
        self.assertSize(res, 1)
        self.assertLatexEnvironment(res(0), 'verbatim', size=1, escape=False, after_begin='\n',
                                    before_end='\n', info=ast.info)
        self.assertLatexString(res(0,0), content=self.CODE.strip('\n'))

class TestListingLanguage(MooseDocsTestCase):
    EXTENSIONS = [core, command, floats, listing, modal]
    CODE = 'void function();'
    TEXT = '!listing language=c++\n' + CODE

    def testAST(self):
        ast = self.tokenize(self.TEXT)
        self.assertSize(ast, 1)
        self.assertToken(ast(0), 'Code', content=self.CODE, style='max-height:350px;',
                         language='c++')

    def testHTML(self):
        _, res = self.execute(self.TEXT)
        self.assertHTMLTag(res, 'body', size=1)
        self._assertHTML(res(0), self.CODE)

    def testMaterialize(self):
        _, res = self.execute(self.TEXT, renderer=base.MaterializeRenderer())
        self.assertHTMLTag(res, 'div', size=1, class_='moose-content')
        self._assertHTML(res(0), self.CODE)

    def _assertHTML(self, res, content):
        self.assertHTMLTag(res, 'pre', size=1, class_='moose-pre', style='max-height:350px;')
        self.assertHTMLTag(res(0), 'code', size=1, class_='language-c++')
        self.assertHTMLString(res(0,0), content=content)

    def testLatex(self):
        ast, res = self.execute(self.TEXT, renderer=base.LatexRenderer())
        self.assertSize(res, 1)
        self.assertLatexEnvironment(res(0), 'verbatim', size=1, escape=False, after_begin='\n',
                                    before_end='\n', info=ast.info)
        self.assertLatexString(res(0,0), content=self.CODE.strip('\n'))

class TestFileListing(MooseDocsTestCase):
    EXTENSIONS = [core, command, floats, listing, modal]
    FILE = 'framework/src/kernels/Diffusion.C'

    # Test 0: Display a C++ source file (excluding MOOSE header) and hide modal link
    CODE = [extractContent(FILE)]
    TEXT = ['!listing ' + FILE + ' link=False id=diffusion-c']

    # Test 1: Display only `validParams()` definition and limit height to 92 pixels, vertically
    CODE.append(extractContent(FILE, opts=dict(start='InputParameters', end='::Diffusion')))
    TEXT.append('!listing ' + FILE + ' start=InputParameters end=::Diffusion max-height=92px')

    def testAST(self):
        ast = self.tokenize(self.TEXT[0])
        self.assertSize(ast, 2)
        self.assertToken(ast(0), 'Listing', size=2)
        self.assertToken(ast(0,0), 'FloatCaption', key='diffusion-c', prefix='Listing', number=1)
        self.assertToken(ast(0,1), 'ListingCode', content=self.CODE[0], style='max-height:350px;',
                         language='cpp')
        self.assertToken(ast(1), 'Shortcut', size=1, key='diffusion-c', link='#diffusion-c',
                         string='Listing 1')

        ast = self.tokenize(self.TEXT[1])
        self.assertSize(ast, 2)
        self.assertToken(ast(0), 'Code', content=self.CODE[1], style='max-height:92px;',
                         language='cpp')
        self.assertToken(ast(1), 'ModalSourceLink', size=0)

    def testHTML(self):
        _, res = self.execute(self.TEXT[0])
        self.assertHTMLTag(res, 'body', size=1)
        self.assertHTMLTag(res(0), 'div', size=2, class_='moose-float-div')
        self.assertHTMLTag(res(0,0), 'p', size=2, class_='moose-caption')
        self.assertHTMLTag(res(0,0,0), 'span', size=1, class_='moose-caption-heading')
        self.assertHTMLString(res(0,0,0,0), content='Listing 1: ')
        self.assertHTMLTag(res(0,0,1), 'span', class_='moose-caption-text')
        self._assertHTML(res(0,1), 'max-height:350px;', self.CODE[0])

        ast, res = self.execute(self.TEXT[1])
        self.assertHTMLTag(res, 'body', size=2)
        self._assertHTML(res(0), 'max-height:92px;', self.CODE[1])
        self.assertHTMLTag(res(1), 'span', size=1, class_='moose-source-filename')
        self.assertHTMLString(res(1,0), content='({})'.format(self.FILE))

    def testMaterialize(self):
        _, res = self.execute(self.TEXT[0], renderer=base.MaterializeRenderer())
        self.assertHTMLTag(res, 'div', size=1, class_='moose-content')
        self.assertHTMLTag(res(0), 'div', size=1, class_='card moose-float')
        self.assertHTMLTag(res(0,0), 'div', size=2, class_='card-content')
        self.assertHTMLTag(res(0,0,0), 'p', size=2, class_='moose-caption')
        self.assertHTMLTag(res(0,0,0,0), 'span', size=1, class_='moose-caption-heading')
        self.assertHTMLString(res(0,0,0,0,0), content='Listing 1: ')
        self.assertHTMLTag(res(0,0,0,1), 'span', class_='moose-caption-text')
        self._assertHTML(res(0,0,1), 'max-height:350px;', self.CODE[0])

        ast, res = self.execute(self.TEXT[1], renderer=base.MaterializeRenderer())
        self.assertHTMLTag(res, 'div', size=3, class_='moose-content')
        self._assertHTML(res(0), 'max-height:92px;', self.CODE[1])
        self.assertHTMLTag(res(1), 'a', size=1, class_='moose-source-filename tooltipped modal-trigger')
        self.assertHTMLString(res(1,0), content='({})'.format(self.FILE))
        self.assertHTMLTag(res(2), 'div', class_='moose-modal modal')

    def _assertHTML(self, res, style, content):
        self.assertHTMLTag(res, 'pre', size=1, class_='moose-pre', style=style)
        self.assertHTMLTag(res(0), 'code', size=1, class_='language-cpp')
        self.assertHTMLString(res(0,0), content=content)

    def testLatex(self):
        ast, res = self.execute(self.TEXT[0], renderer=base.LatexRenderer())
        self.assertSize(res, 2)
        self.assertLatexEnvironment(res(0), 'lstlisting', size=1, escape=False, after_begin='\n',
                                    before_end='\n', info=ast.info)
        self.assertLatexArg(res(0), 0, 'Bracket', size=4)
        self.assertLatexString(res(0)['args'][0](0), content='language=C++,')
        self.assertLatexString(res(0)['args'][0](1), content='label=diffusion-c,')
        self.assertLatexString(res(0)['args'][0](2), content='caption=')
        self.assertLatexString(res(0)['args'][0](3), content='\\mbox{}', escape=False)
        self.assertLatexString(res(0,0), content=self.CODE[0].strip('\n'))
        self.assertLatexEnvironment(res(1), 'lstlisting', size=1, escape=False, after_begin='\n',
                                    before_end='\n', info=ast.info)
        self.assertLatexArg(res(1), 0, 'Bracket', size=1)
        self.assertLatexString(res(1)['args'][0](0), content='language=C++,')
        self.assertLatexString(res(1,0), content=self.CODE[0].strip('\n'))

        ast, res = self.execute(self.TEXT[1], renderer=base.LatexRenderer())
        self.assertSize(res, 2)
        self.assertLatexEnvironment(res(0), 'verbatim', size=1, escape=False, after_begin='\n',
                                    before_end='\n', info=ast.info)
        self.assertLatexString(res(0,0), content=self.CODE[1].strip('\n'))
        self.assertLatexString(res(1), content='(framework/src/kernels/Diffusion.C)')


class TestInputListing(MooseDocsTestCase):
    EXTENSIONS = [core, command, floats, listing, modal]
    FILE = ['test/tests/kernels/simple_diffusion/simple_diffusion.i',
            'test/tests/kernels/simple_diffusion/tests']
    FILE = [FILE[0], FILE[1], FILE[0], FILE[1], FILE[0]]

    # Test 0: Extract the `[Mesh]` and `[Kernels]` block
    CODE = [extractContent(FILE[0], opts=dict(start='Mesh', end='Variables'))
            + '\n'
            + extractContent(FILE[0], opts=dict(start='Kernels', end='BCs'))]
    TEXT = ['block=Mesh Kernels prefix=xxxxx id=prfx']

    # Test 1: Extract `[Mesh]` & `[Kernels]` blocks and use a special prefix
    CODE.append('[Tests]\n[]\n')
    TEXT.append('link=False remove=test')

    # Test 2: Extract `[diff]` block, indent 2 spaces, add `[AuxKernels]` & `[]` as header & footer
    CODE.append(extractContent(FILE[2], opts={'start': 'diff', 'end': '[]', 'include-end': True,
                                              'header': '[AuxKernels]', 'footer': '[]'}))
    TEXT.append('block=Kernels/diff indent=2 header=[AuxKernels] footer=[]')

    # Test 3: Remove `issues` & `design` parameters
    CODE.append(extractContent(FILE[3], opts={'end': 'exodiff', 'include-end': True})
                + '\n'
                + extractContent(FILE[3], opts=dict(start='requirement')))
    TEXT.append('remove=/Tests/test/issues Tests/test/design')

    # Test 4: Extract `[BCs]` block then remove `[left]` block and `value` parameter
    CODE.append(extractContent(FILE[4], opts={'start': '[right]', 'end': 'value',
                                              'header': '[BCs]', 'footer': '  []\n[]\n'}))
    TEXT.append('block=BCs remove=left BCs/right/value')

    # complete commands for tokenization
    TEXT = ['!listing moose/' + f + ' ' + t for f, t in zip(FILE, TEXT)]

    def testAST(self):
        ast = self.tokenize(self.TEXT[0])
        self.assertSize(ast, 2)
        self.assertToken(ast(0), 'Listing', size=3)
        self.assertToken(ast(0,0), 'FloatCaption', key='prfx', prefix='xxxxx', number=1)
        self.assertToken(ast(0,1), 'ListingCode', content=self.CODE[0], style='max-height:350px;',
                         language='text')
        self.assertToken(ast(0,2), 'ModalSourceLink', size=0)
        self.assertToken(ast(1), 'Shortcut', size=1, key='prfx', link='#prfx', string='Xxxxx 1')

        ast = self.tokenize(self.TEXT[1])
        self.assertSize(ast, 1)
        self.assertToken(ast(0), 'Code', content=self.CODE[1], style='max-height:350px;', language='text')

        for i in range(2, 5):
            ast = self.tokenize(self.TEXT[i])
            self.assertSize(ast, 2)
            self.assertToken(ast(0), 'Code', content=self.CODE[i], style='max-height:350px;',
                             language='text')
            self.assertToken(ast(1), 'ModalSourceLink', size=0)

    def testHTML(self):
        ast, res = self.execute(self.TEXT[0])
        self.assertHTMLTag(res, 'body', size=1)
        self.assertHTMLTag(res(0), 'div', size=3, class_='moose-float-div')
        self.assertHTMLTag(res(0,0), 'p', size=2, class_='moose-caption')
        self.assertHTMLTag(res(0,0,0), 'span', size=1, class_='moose-caption-heading')
        self.assertHTMLString(res(0,0,0,0), content='xxxxx 1: ')
        self.assertHTMLTag(res(0,0,1), 'span', class_='moose-caption-text')
        self._assertHTML(res(0,1), 'max-height:350px;', self.CODE[0])
        self.assertHTMLTag(res(0,2), 'span', size=1, class_='moose-source-filename')
        self.assertHTMLString(res(0,2,0), content='({})'.format(self.FILE[0]))

        _, res = self.execute(self.TEXT[1])
        self.assertHTMLTag(res, 'body', size=1)
        self._assertHTML(res(0), 'max-height:350px;', self.CODE[1])

        for i in range(2, 5):
            ast, res = self.execute(self.TEXT[i])
            self.assertHTMLTag(res, 'body', size=2)
            self._assertHTML(res(0), 'max-height:350px;', self.CODE[i])
            self.assertHTMLTag(res(1), 'span', size=1, class_='moose-source-filename')
            self.assertHTMLString(res(1,0), content='({})'.format(self.FILE[i]))

    def testMaterialize(self):
        ast, res = self.execute(self.TEXT[0], renderer=base.MaterializeRenderer())
        self.assertHTMLTag(res, 'div', size=2, class_='moose-content')
        self.assertHTMLTag(res(0), 'div', size=1, class_='card moose-float')
        self.assertHTMLTag(res(0,0), 'div', size=3, class_='card-content')
        self.assertHTMLTag(res(0,0,0), 'p', size=2, class_='moose-caption')
        self.assertHTMLTag(res(0,0,0,0), 'span', size=1, class_='moose-caption-heading')
        self.assertHTMLString(res(0,0,0,0,0), content='xxxxx 1: ')
        self.assertHTMLTag(res(0,0,0,1), 'span', class_='moose-caption-text')
        self._assertHTML(res(0,0,1), 'max-height:350px;', self.CODE[0])
        self.assertHTMLTag(res(0,0,2), 'a', size=1, class_='moose-source-filename tooltipped modal-trigger')
        self.assertHTMLString(res(0,0,2,0), content='({})'.format(self.FILE[0]))

        _, res = self.execute(self.TEXT[1], renderer=base.MaterializeRenderer())
        self.assertHTMLTag(res, 'div', size=1, class_='moose-content')
        self._assertHTML(res(0), 'max-height:350px;', self.CODE[1])

        for i in range(2, 5):
            ast, res = self.execute(self.TEXT[i], renderer=base.MaterializeRenderer())
            self.assertHTMLTag(res, 'div', size=3, class_='moose-content')
            self._assertHTML(res(0), 'max-height:350px;', self.CODE[i])
            self.assertHTMLTag(res(1), 'a', size=1, class_='moose-source-filename tooltipped modal-trigger')
            self.assertHTMLString(res(1,0), content='({})'.format(self.FILE[i]))

    def _assertHTML(self, res, style, content):
        self.assertHTMLTag(res, 'pre', size=1, class_='moose-pre', style=style)
        self.assertHTMLTag(res(0), 'code', size=1, class_='language-text')
        self.assertHTMLString(res(0,0), content=content)

    def testLatex(self):
        ast, res = self.execute(self.TEXT[0], renderer=base.LatexRenderer())
        self.assertSize(res, 2)
        self.assertLatexEnvironment(res(0), 'lstlisting', size=1, escape=False, after_begin='\n',
                                    before_end='\n', info=ast.info)
        self.assertLatexArg(res(0), 0, 'Bracket', size=3)
        self.assertLatexString(res(0)['args'][0](0), content='label=prfx,')
        self.assertLatexString(res(0)['args'][0](1), content='caption=')
        self.assertLatexString(res(0)['args'][0](2), content='\\mbox{}', escape=False)
        self.assertLatexString(res(0,0), content=self.CODE[0].strip('\n'))
        self.assertLatexEnvironment(res(1), 'lstlisting', size=1, escape=False, after_begin='\n',
                                    before_end='\n', info=ast.info)
        self.assertLatexArg(res(1), 0, 'Bracket')
        self.assertLatexString(res(1,0), content=self.CODE[0].strip('\n'))

        for i in range(1, 5):
            ast, res = self.execute(self.TEXT[i], renderer=base.LatexRenderer())
            self.assertLatexEnvironment(res(0), 'verbatim', size=1, escape=False, after_begin='\n',
                                        before_end='\n', info=ast.info)
            self.assertLatexString(res(0,0), content=self.CODE[i].strip('\n'))

if __name__ == '__main__':
    unittest.main(verbosity=2)
