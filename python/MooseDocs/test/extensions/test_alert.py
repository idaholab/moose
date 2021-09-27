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
from MooseDocs import common, base
from MooseDocs.test import MooseDocsTestCase
from MooseDocs.extensions import core, command, alert
logging.basicConfig()

class TestAlertBrands(MooseDocsTestCase):
    EXTENSIONS = [core, command, alert]

    # test all subcommands simultaneously - each with a unique message
    BRANDS = ['error', 'warning', 'note', 'tip']
    ICONS = ['report', 'warning', 'comment', 'school']
    MESSAGE = [
                ['An', ' ', 'error', '.'],
                ['A', ' ', 'warning', '.'],
                ['A', ' ', 'note', '.'],
                ['A', ' ', 'tip', '.'],
              ]
    TEXT = ['!alert ' + b + '\n' + ''.join(m) for b, m in zip(BRANDS, MESSAGE)]

    def testAST(self):
        for i, b in enumerate(self.BRANDS):
            ast = self.tokenize(self.TEXT[i])
            self.assertSize(ast, 1)
            self.assertToken(ast(0), 'AlertToken', size=2, brand=b)
            self.assertToken(ast(0)(0), 'AlertTitle', brand=b, prefix=True, icon=True, icon_name=self.ICONS[i])
            self.assertToken(ast(0)(1), 'AlertContent', size=1, brand=b)
            self.assertToken(ast(0)(1)(0), 'Paragraph', size=len(self.MESSAGE))

    def testHTML(self):
        for i, b in enumerate(self.BRANDS):
            _, res = self.execute(self.TEXT[i])

            self.assertHTMLTag(res, 'body', size=1)
            classref = 'moose-alert moose-alert-{}'.format(b)
            self.assertHTMLTag(res(0), 'div', size=2, class_=classref)

            self.assertHTMLTag(res(0,0), 'div', size=2, class_='moose-alert-title')
            self._assertHTML(res(0,1,0), self.MESSAGE[i])

    def testMaterialize(self):
        for i, b in enumerate(self.BRANDS):
            res = self.render(self.tokenize(self.TEXT[i]), renderer=base.MaterializeRenderer())
            self.assertHTMLTag(res, 'div', size=1, class_='moose-content')
            classref = 'card moose-alert moose-alert-{}'.format(b)
            self.assertHTMLTag(res(0), 'div', size=2, class_=classref)

            title = res(0)(0)
            self.assertHTMLTag(title, 'div', size=2, class_='card-title moose-alert-title')
            self.assertHTMLTag(title(0), 'i', size=1, class_='material-icons moose-inline-icon', string=self.ICONS[i])
            self.assertHTMLTag(title(1), 'span', size=1, class_='moose-alert-title-brand', string=b)

            content = res(0)(1)
            self.assertHTMLTag(content, 'div', size=1, class_='card-content')
            self.assertHTMLTag(content(0), 'div', size=1, class_='moose-alert-content')
            self._assertHTML(content(0)(0), self.MESSAGE[i])

    def _assertHTML(self, res, message):
        self.assertHTMLTag(res, 'p', size=len(message))
        for i in range(len(message)):
            self.assertHTMLString(res(i), content=message[i])

    def testLatex(self):
        for i, b in enumerate(self.BRANDS):
            res = self.render(self.tokenize(self.TEXT[i]), renderer=base.LatexRenderer())
            self.assertSize(res, 1)
            self.assertLatex(res(0), 'Environment', 'alert', size=len(self.MESSAGE)+1)
            self.assertLatexArg(res(0), 0, 'Bracket', b)
            self.assertLatexArg(res(0), 1, 'Brace', b)

            self.assertLatex(res(0)(0), 'Command', 'par')
            for j in range(len(self.MESSAGE[i])):
                self.assertLatexString(res(0)(j+1), content=self.MESSAGE[i][j])

class TestAlertConstruction(MooseDocsTestCase):
    EXTENSIONS = [core, command, alert]
    TEXT = '!alert construction\nUnder construction.'

    def testAST(self):
        ast = self.tokenize(self.TEXT)
        self.assertSize(ast, 1)
        self.assertToken(ast(0), 'AlertToken', size=2, brand='construction')
        self.assertToken(ast(0)(0), 'AlertTitle', brand='construction', prefix=True, icon=True, icon_name='build')
        self.assertToken(ast(0)(1), 'AlertContent', size=1, brand='construction')
        self.assertToken(ast(0)(1)(0), 'Paragraph', size=4)

    def testHTML(self):
        _, res = self.execute(self.TEXT)
        self.assertHTMLTag(res, 'body', size=1)

        self.assertHTMLTag(res(0), 'div', size=2, class_='moose-alert moose-alert-construction')
        self.assertHTMLTag(res(0,0), 'div', size=2, class_='moose-alert-title')

        self.assertHTMLTag(res(0,1), 'div', size=1, class_='moose-alert-content')
        self._assertHTML(res(0,1,0))

    def testMaterialize(self):
        _, res = self.execute(self.TEXT, renderer=base.MaterializeRenderer())
        self.assertHTMLTag(res, 'div', size=1, class_='moose-content')
        self.assertHTMLTag(res(0), 'div', size=2, class_='card moose-alert moose-alert-construction')

        title = res(0)(0)
        self.assertHTMLTag(title, 'div', size=2, class_='card-title moose-alert-title')
        self.assertHTMLTag(title(0), 'i', size=1, class_='material-icons moose-inline-icon', string='build')
        self.assertHTMLTag(title(1), 'span', size=1, class_='moose-alert-title-brand', string='construction')

    def _assertHTML(self, res):
        self.assertHTMLTag(res, 'p', size=4)
        self.assertHTMLString(res(0), content='Under')
        self.assertHTMLString(res(1), content=' ')
        self.assertHTMLString(res(2), content='construction')
        self.assertHTMLString(res(3), content='.')

    def testLatex(self):
        _, res = self.execute(self.TEXT, renderer=base.LatexRenderer())
        self.assertSize(res, 1)

        self.assertLatex(res(0), 'Environment', 'alert', size=5)
        self.assertLatexArg(res(0), 0, 'Bracket', 'construction')
        self.assertLatexArg(res(0), 1, 'Brace', 'construction')


class TestAlertConstructionNoIcon(MooseDocsTestCase):
    EXTENSIONS = [core, command, alert]
    TEXT = '!alert construction icon=False\nUnder construction.'

    def testAST(self):
        ast = self.tokenize(self.TEXT)
        self.assertSize(ast, 1)
        self.assertToken(ast(0), 'AlertToken', size=2, brand='construction')
        self.assertToken(ast(0)(0), 'AlertTitle', brand='construction', prefix=True, icon=False, icon_name='build')
        self.assertToken(ast(0)(1), 'AlertContent', size=1, brand='construction')
        self.assertToken(ast(0)(1)(0), 'Paragraph', size=4)

    def testHTML(self):
        _, res = self.execute(self.TEXT)
        self.assertHTMLTag(res, 'body', size=1)
        self.assertHTMLTag(res(0), 'div', size=2, class_='moose-alert moose-alert-construction')
        self.assertHTMLTag(res(0,0), 'div', size=1, class_='moose-alert-title')
        self.assertHTMLTag(res(0,1), 'div', size=1, class_='moose-alert-content')
        self._assertHTML(res(0,1,0))

    def testMaterialize(self):
        _, res = self.execute(self.TEXT, renderer=base.MaterializeRenderer())
        self.assertHTMLTag(res, 'div', size=1, class_='moose-content')
        self.assertHTMLTag(res(0), 'div', size=2, class_='card moose-alert moose-alert-construction')

        title = res(0)(0)
        self.assertHTMLTag(title, 'div', size=1, class_='card-title moose-alert-title')
        self.assertHTMLTag(title(0), 'span', size=1, class_='moose-alert-title-brand')
        self.assertHTMLString(title(0)(0), content='construction')

    def _assertHTML(self, res):
        self.assertHTMLTag(res, 'p', size=4)
        self.assertHTMLString(res(0), content='Under')
        self.assertHTMLString(res(1), content=' ')
        self.assertHTMLString(res(2), content='construction')
        self.assertHTMLString(res(3), content='.')

    def testLatex(self):
        _, res = self.execute(self.TEXT, renderer=base.LatexRenderer())
        self.assertSize(res, 1)
        self.assertLatex(res(0), 'Environment', 'alert', size=5)
        self.assertLatexArg(res(0), 0, 'Bracket', 'construction')
        self.assertLatexArg(res(0), 1, 'Brace', 'construction')

        self.assertLatex(res(0)(0), 'Command', 'par')
        self.assertLatexString(res(0)(1), content='Under')
        self.assertLatexString(res(0)(2), content=' ')
        self.assertLatexString(res(0)(3), content='construction')
        self.assertLatexString(res(0)(4), content='.')

class TestAlertTitle(MooseDocsTestCase):
    EXTENSIONS = [core, command, alert]
    TEXT = '!alert note title=A Title\nNote with a title.'

    def testAST(self):
        ast = self.tokenize(self.TEXT)
        self.assertSize(ast, 1)
        self.assertToken(ast(0), 'AlertToken', size=2, brand='note')

        self.assertToken(ast(0)(0), 'AlertTitle', size=3, brand='note', prefix=True, icon=True, icon_name='comment')
        self.assertToken(ast(0)(0)(0), 'Word', content='A')
        self.assertToken(ast(0)(0)(1), 'Space')
        self.assertToken(ast(0)(0)(2), 'Word', content='Title')

        self.assertToken(ast(0)(1), 'AlertContent', size=1, brand='note')
        self.assertToken(ast(0)(1)(0), 'Paragraph', size=8)

    def testHTML(self):
        _, res = self.execute(self.TEXT)
        self.assertHTMLTag(res, 'body', size=1)
        self.assertHTMLTag(res(0), 'div', size=2, class_='moose-alert moose-alert-note')
        content = res(0)(0)
        self.assertHTMLTag(content, 'div', size=5, class_='moose-alert-title')
        self.assertHTMLTag(content(0), 'i', size=1)
        self.assertHTMLTag(content(1), 'span', size=2)
        self.assertHTMLString(content(2), content='A')
        self.assertHTMLString(content(3), content=' ')
        self.assertHTMLString(content(4), content='Title')
        self._assertHTML(res(0,1,0))

    def testMaterialize(self):
        _, res = self.execute(self.TEXT, renderer=base.MaterializeRenderer())
        self.assertHTMLTag(res, 'div', size=1, class_='moose-content')
        self.assertHTMLTag(res(0), 'div', size=2, class_='card moose-alert moose-alert-note')

        title = res(0)(0)
        self.assertHTMLTag(title, 'div', size=5, class_='card-title moose-alert-title')
        self.assertHTMLTag(title(0), 'i', size=1, class_='material-icons moose-inline-icon', string='comment')
        self.assertHTMLTag(title(1), 'span', size=2, class_='moose-alert-title-brand')
        self.assertHTMLString(title(1)(0), content='note')
        self.assertHTMLString(title(1)(1), content=':')
        self.assertHTMLString(title(2), content='A')
        self.assertHTMLString(title(3), content=' ')
        self.assertHTMLString(title(4), content='Title')

        content = res(0)(1)
        self.assertHTMLTag(content, 'div', size=1, class_='card-content')
        self.assertHTMLTag(content(0), 'div', size=1, class_='moose-alert-content')
        self._assertHTML(res(0,1,0,0))

    def _assertHTML(self, res):
        self.assertHTMLTag(res, 'p', size=8)
        self.assertHTMLString(res(0), content='Note')
        self.assertHTMLString(res(1), content=' ')
        self.assertHTMLString(res(2), content='with')
        self.assertHTMLString(res(3), content=' ')
        self.assertHTMLString(res(4), content='a')
        self.assertHTMLString(res(5), content=' ')
        self.assertHTMLString(res(6), content='title')
        self.assertHTMLString(res(7), content='.')

    def testLatex(self):
        _, res = self.execute(self.TEXT, renderer=base.LatexRenderer())
        self.assertSize(res, 1)
        self.assertLatex(res(0), 'Environment', 'alert', size=9)
        self.assertLatexArg(res(0), 0, 'Bracket', 'note')
        self.assertLatexArg(res(0), 1, 'Brace', 'note')
        self.assertLatexArg(res(0), 2, 'Bracket')

        self.assertLatex(res(0)(0), 'Command', 'par')
        self.assertLatexString(res(0)(1), content='Note')
        self.assertLatexString(res(0)(2), content=' ')
        self.assertLatexString(res(0)(3), content='with')
        self.assertLatexString(res(0)(4), content=' ')
        self.assertLatexString(res(0)(5), content='a')
        self.assertLatexString(res(0)(6), content=' ')
        self.assertLatexString(res(0)(7), content='title')
        self.assertLatexString(res(0)(8), content='.')

class TestAlertTitleNoPrefix(MooseDocsTestCase):
    EXTENSIONS = [core, command, alert]
    TEXT = '!alert note title=A Title prefix=False\nTitle but no prefix.'

    def testAST(self):
        ast = self.tokenize(self.TEXT)
        self.assertSize(ast, 1)
        self.assertToken(ast(0), 'AlertToken', size=2, brand='note')

        self.assertToken(ast(0)(0), 'AlertTitle', size=3, brand='note', prefix=False, icon=True, icon_name='comment')
        self.assertToken(ast(0)(0)(0), 'Word', content='A')
        self.assertToken(ast(0)(0)(1), 'Space')
        self.assertToken(ast(0)(0)(2), 'Word', content='Title')

        self.assertToken(ast(0)(1), 'AlertContent', size=1, brand='note')
        self.assertToken(ast(0)(1)(0), 'Paragraph', size=8)

    def testHTML(self):
        _, res = self.execute(self.TEXT)
        self.assertHTMLTag(res, 'body', size=1)
        self.assertHTMLTag(res(0), 'div', size=2, class_='moose-alert moose-alert-note')
        self.assertHTMLTag(res(0,0), 'div', size=4, class_='moose-alert-title')
        self.assertHTMLTag(res(0,0,0), 'i', string='comment')
        self.assertHTMLString(res(0,0,1), content='A')
        self.assertHTMLString(res(0,0,2), content=' ')
        self.assertHTMLString(res(0,0,3), content='Title')

        self.assertHTMLTag(res(0,1), 'div', size=1, class_='moose-alert-content')
        self._assertHTML(res(0,1,0))

    def testMaterialize(self):
        _, res = self.execute(self.TEXT, renderer=base.MaterializeRenderer())
        self.assertHTMLTag(res, 'div', size=1, class_='moose-content')
        self.assertHTMLTag(res(0), 'div', size=2, class_='card moose-alert moose-alert-note')

        title = res(0)(0)
        self.assertHTMLTag(title, 'div', size=4, class_='card-title moose-alert-title')
        self.assertHTMLTag(title(0), 'i', size=1, class_='material-icons moose-inline-icon', string='comment')
        self.assertHTMLString(title(1), content='A')
        self.assertHTMLString(title(2), content=' ')
        self.assertHTMLString(title(3), content='Title')

        content = res(0)(1)
        self.assertHTMLTag(content, 'div', size=1, class_='card-content')
        self.assertHTMLTag(content(0), 'div', size=1, class_='moose-alert-content')
        self._assertHTML(content(0)(0))

    def _assertHTML(self, res):
        self.assertHTMLTag(res, 'p', size=8)
        self.assertHTMLString(res(0), content='Title')
        self.assertHTMLString(res(1), content=' ')
        self.assertHTMLString(res(2), content='but')
        self.assertHTMLString(res(3), content=' ')
        self.assertHTMLString(res(4), content='no')
        self.assertHTMLString(res(5), content=' ')
        self.assertHTMLString(res(6), content='prefix')
        self.assertHTMLString(res(7), content='.')

    def testLatex(self):
        _, res = self.execute(self.TEXT, renderer=base.LatexRenderer())
        self.assertSize(res, 1)
        self.assertLatex(res(0), 'Environment', 'alert', size=9)
        self.assertLatexArg(res(0), 0, 'Bracket')
        self.assertLatexArg(res(0), 1, 'Brace', 'note')
        self.assertLatexArg(res(0), 2, 'Bracket')

        self.assertLatex(res(0)(0), 'Command', 'par')
        self.assertLatexString(res(0)(1), content='Title')
        self.assertLatexString(res(0)(2), content=' ')
        self.assertLatexString(res(0)(3), content='but')
        self.assertLatexString(res(0)(4), content=' ')
        self.assertLatexString(res(0)(5), content='no')
        self.assertLatexString(res(0)(6), content=' ')
        self.assertLatexString(res(0)(7), content='prefix')
        self.assertLatexString(res(0)(8), content='.')

class TestAlertTitleLink(MooseDocsTestCase):
    EXTENSIONS = [core, command, alert]
    TEXT = '!alert tip title=[Google](https://google.com)\nTitle with link.'

    def testAST(self):
        ast = self.tokenize(self.TEXT)
        self.assertSize(ast, 1)
        self.assertToken(ast(0), 'AlertToken', size=2, brand='tip')

        self.assertToken(ast(0)(0), 'AlertTitle', size=1, brand='tip', prefix=True, icon=True, icon_name='school')
        self.assertToken(ast(0)(0)(0), 'Link', size=1, url='https://google.com')
        self.assertToken(ast(0)(0)(0)(0), 'Word', content='Google')

        self.assertToken(ast(0)(1), 'AlertContent', size=1, brand='tip')
        self.assertToken(ast(0)(1)(0), 'Paragraph', size=6)

    def testHTML(self):
        _, res = self.execute(self.TEXT)

        self.assertHTMLTag(res, 'body', size=1)
        self.assertHTMLTag(res(0), 'div', size=2, class_='moose-alert moose-alert-tip')
        self.assertHTMLTag(res(0,0), 'div', size=3, class_='moose-alert-title')
        self.assertHTMLTag(res(0,0,0), 'i', string='school')
        self.assertHTMLTag(res(0,0,1), 'span', size=2)
        self.assertHTMLTag(res(0,0,2), 'a', size=1, href='https://google.com', string='Google')

        self.assertHTMLTag(res(0,1), 'div', size=1, class_='moose-alert-content')
        self._assertHTML(res(0,1,0))

    def testMaterialize(self):
        _, res = self.execute(self.TEXT, renderer=base.MaterializeRenderer())
        self.assertHTMLTag(res, 'div', size=1, class_='moose-content')
        self.assertHTMLTag(res(0), 'div', size=2, class_='card moose-alert moose-alert-tip')

        title = res(0)(0)
        self.assertHTMLTag(title, 'div', size=3, class_='card-title moose-alert-title')
        self.assertHTMLTag(title(0), 'i', size=1, class_='material-icons moose-inline-icon', string='school')
        self.assertHTMLTag(title(1), 'span', size=2, class_='moose-alert-title-brand')
        self.assertHTMLString(title(1)(0), content='tip')
        self.assertHTMLString(title(1)(1), content=':')
        self.assertHTMLTag(title(2), 'a', size=1, href='https://google.com', string='Google')

        content = res(0)(1)
        self.assertHTMLTag(content, 'div', size=1, class_='card-content')
        self.assertHTMLTag(content(0), 'div', size=1, class_='moose-alert-content')
        self._assertHTML(content(0)(0))

    def _assertHTML(self, res):
        self.assertHTMLTag(res, 'p', size=6)
        self.assertHTMLString(res(0), content='Title')
        self.assertHTMLString(res(1), content=' ')
        self.assertHTMLString(res(2), content='with')
        self.assertHTMLString(res(3), content=' ')
        self.assertHTMLString(res(4), content='link')
        self.assertHTMLString(res(5), content='.')

    def testLatex(self):
        _, res = self.execute(self.TEXT, renderer=base.LatexRenderer())
        self.assertSize(res, 1)
        self.assertLatex(res(0), 'Environment', 'alert', size=7)
        self.assertLatexArg(res(0), 0, 'Bracket', 'tip')
        self.assertLatexArg(res(0), 1, 'Brace', 'tip')
        self.assertLatexArg(res(0), 2, 'Bracket')

        self.assertLatex(res(0)(0), 'Command', 'par')
        self.assertLatexString(res(0)(1), content='Title')
        self.assertLatexString(res(0)(2), content=' ')
        self.assertLatexString(res(0)(3), content='with')
        self.assertLatexString(res(0)(4), content=' ')
        self.assertLatexString(res(0)(5), content='link')
        self.assertLatexString(res(0)(6), content='.')

class TestAlertWithCode(MooseDocsTestCase):
    EXTENSIONS = [core, command, alert]
    TEXT = ('!alert! error\n'
            'Alert with code.\n\n'
            '```c++\n'
            'intx;\n'
            '```\n'
            '!alert-end!')

    def testAST(self):
        ast = self.tokenize(self.TEXT)
        self.assertSize(ast, 1)
        self.assertToken(ast(0), 'AlertToken', size=2, brand='error')
        self.assertToken(ast(0)(0), 'AlertTitle', brand='error', prefix=True, icon=True, icon_name='report')
        self.assertToken(ast(0)(1), 'AlertContent', size=2, brand='error')
        self.assertToken(ast(0)(1)(0), 'Paragraph', size=6)
        self.assertToken(ast(0)(1)(1), 'Code', content='\nintx;\n', language='c++')

    def testHTML(self):
        _, res = self.execute(self.TEXT)

        self.assertHTMLTag(res, 'body', size=1)
        self.assertHTMLTag(res(0), 'div', size=2, class_='moose-alert moose-alert-error')
        self.assertHTMLTag(res(0,0), 'div', size=2, class_='moose-alert-title')

        self.assertHTMLTag(res(0,1), 'div', size=2, class_='moose-alert-content')
        self._assertHTML(res(0,1))

    def testMaterialize(self):
        _, res = self.execute(self.TEXT, renderer=base.MaterializeRenderer())

        self.assertHTMLTag(res, 'div', size=1, class_='moose-content')
        self.assertHTMLTag(res(0), 'div', size=2, class_='card moose-alert moose-alert-error')

        self.assertHTMLTag(res(0,0), 'div', size=2, class_='card-title moose-alert-title')
        self.assertHTMLTag(res(0,0,0), 'i', size=1, class_='material-icons moose-inline-icon', string='report')
        self.assertHTMLTag(res(0,0,1), 'span', size=1, class_='moose-alert-title-brand', string='error')

        self.assertHTMLTag(res(0,1), 'div', size=1, class_='card-content')
        self.assertHTMLTag(res(0,1,0), 'div', size=2, class_='moose-alert-content')
        self._assertHTML(res(0,1,0))

    def _assertHTML(self, res):
        self.assertHTMLTag(res, 'div', size=2)

        self.assertHTMLTag(res(0), 'p', size=6)
        self.assertHTMLString(res(0)(0), content='Alert')
        self.assertHTMLString(res(0)(1), content=' ')
        self.assertHTMLString(res(0)(2), content='with')
        self.assertHTMLString(res(0)(3), content=' ')
        self.assertHTMLString(res(0)(4), content='code')
        self.assertHTMLString(res(0)(5), content='.')

        self.assertHTMLTag(res(1), 'pre', size=1, class_='moose-pre')
        self.assertHTMLTag(res(1)(0), 'code', size=1, class_='language-c++')
        self.assertHTMLString(res(1)(0)(0), content='\nintx;\n')

    def testLatex(self):
        _, res = self.execute(self.TEXT, renderer=base.LatexRenderer())
        self.assertSize(res, 1)
        self.assertLatex(res(0), 'Environment', 'alert', size=8)
        self.assertLatexArg(res(0), 0, 'Bracket', 'error')
        self.assertLatexArg(res(0), 1, 'Brace', 'error')

        self.assertLatex(res(0)(0), 'Command', 'par')
        self.assertLatexString(res(0)(1), content='Alert')
        self.assertLatexString(res(0)(2), content=' ')
        self.assertLatexString(res(0)(3), content='with')
        self.assertLatexString(res(0)(4), content=' ')
        self.assertLatexString(res(0)(5), content='code')
        self.assertLatexString(res(0)(6), content='.')

        self.assertLatex(res(0)(7), 'Environment', 'verbatim', size=1)
        self.assertLatexString(res(0)(7)(0), content='intx;')


if __name__ == '__main__':
    unittest.main(verbosity=2)
