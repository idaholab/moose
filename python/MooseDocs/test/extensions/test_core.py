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
from MooseDocs.test import MooseDocsTestCase
from MooseDocs.extensions import core
from MooseDocs import base
logging.basicConfig()

class TestCore(MooseDocsTestCase):
    EXTENSIONS = [core]

    def testCodeBlock(self):
        text = "```\nint x = 0;\n```"
        ast = self.tokenize(text)
        self.assertToken(ast(0), 'Code', language='text', content='\nint x = 0;\n', escape=True)

        def helper(r):
            self.assertHTMLTag(r(0), 'pre', class_='moose-pre')
            self.assertHTMLTag(r(0)(0), 'code', class_='language-text')
            self.assertHTMLString(r(0)(0)(0), '\nint x = 0;\n', escape=True)

        res = self.render(ast)
        self.assertHTMLTag(res, 'body')
        helper(res)

        res = self.render(ast, renderer=base.MaterializeRenderer())
        self.assertHTMLTag(res, 'div')
        helper(res)

        tex = self.render(ast, renderer=base.LatexRenderer())
        self.assertLatex(tex(0), 'Environment', 'verbatim',
                         after_begin='\n', before_end='\n', escape=False)
        self.assertLatexString(tex(0)(0), 'int x = 0;', escape=False)

    def testLineBreak(self):
        text = r'Break\\ this'
        ast = self.tokenize(text)
        self.assertToken(ast(0), 'Paragraph', size=3)
        self.assertToken(ast(0)(0), 'Word', content='Break')
        self.assertToken(ast(0)(1), 'LineBreak')
        self.assertToken(ast(0)(2), 'Word', content='this')

        text = r'''Break\\
this'''
        ast = self.tokenize(text)
        self.assertToken(ast(0), 'Paragraph', size=3)
        self.assertToken(ast(0)(0), 'Word', content='Break')
        self.assertToken(ast(0)(1), 'LineBreak')
        self.assertToken(ast(0)(2), 'Word', content='this')

    def testEscapeCharacter(self):
        text = "No \[link\] and no \!\! comment"
        ast = self.tokenize(text)
        self.assertToken(ast(0), 'Paragraph', size=14)
        self.assertToken(ast(0)(0), 'Word', content='No')
        self.assertToken(ast(0)(1), 'Space', count=1)
        self.assertToken(ast(0)(2), 'Punctuation', content='[')
        self.assertToken(ast(0)(3), 'Word', content='link')
        self.assertToken(ast(0)(4), 'Punctuation', content=']')
        self.assertToken(ast(0)(5), 'Space', count=1)
        self.assertToken(ast(0)(6), 'Word', content='and')
        self.assertToken(ast(0)(7), 'Space', count=1)
        self.assertToken(ast(0)(8), 'Word', content='no')
        self.assertToken(ast(0)(9), 'Space', count=1)
        self.assertToken(ast(0)(10), 'Punctuation', content='!')
        self.assertToken(ast(0)(11), 'Punctuation', content='!')
        self.assertToken(ast(0)(12), 'Space', count=1)
        self.assertToken(ast(0)(13), 'Word', content='comment')

        for c in ['!', '[', ']', '@', '^', '*', '+', '~', '-']:
            text = r'foo \{} bar'.format(c)
            ast = self.tokenize(text)
            self.assertToken(ast(0), 'Paragraph', size=5)
            self.assertToken(ast(0)(0), 'Word', content='foo')
            self.assertToken(ast(0)(1), 'Space', count=1)
            self.assertToken(ast(0)(2), 'Punctuation', content=c)
            self.assertToken(ast(0)(3), 'Space', count=1)
            self.assertToken(ast(0)(4), 'Word', content='bar')

    def testLinkInline(self):
        text = '[Bar](bar)'
        ast = self.tokenize(text)
        self.assertToken(ast(0), 'Paragraph', size=1)
        self.assertToken(ast(0,0), 'Link', size=1, url='bar')
        self.assertToken(ast(0,0,0), 'Word', content='Bar')

        text = '[Bar](bar id=y)'
        ast = self.tokenize(text)
        self.assertToken(ast(0), 'Paragraph', size=1)
        self.assertToken(ast(0,0), 'Link', size=1, url='bar', id_='y')
        self.assertToken(ast(0,0,0), 'Word', content='Bar')

        text = '[foo] [Bar](bar)'
        ast = self.tokenize(text)
        self.assertToken(ast(0), 'Paragraph', size=3)
        self.assertToken(ast(0,0), 'ShortcutLink', key='foo')
        self.assertToken(ast(0,1), 'Space', count=1)
        self.assertToken(ast(0,2), 'Link', size=1, url='bar')
        self.assertToken(ast(0,2,0), 'Word', content='Bar')

        text = '[Bar](bar.md) [foo]'
        ast = self.tokenize(text)
        self.assertToken(ast(0), 'Paragraph', size=3)
        self.assertToken(ast(0,0), 'Link', size=1, url='bar.md')
        self.assertToken(ast(0,0,0), 'Word', content='Bar')
        self.assertToken(ast(0,1), 'Space', count=1)
        self.assertToken(ast(0,2), 'ShortcutLink', key='foo')

        text = '[bar.md], [Foo](foo.md)'
        ast = self.tokenize(text)
        self.assertToken(ast(0), 'Paragraph', size=4)
        self.assertToken(ast(0,0), 'ShortcutLink', key='bar.md')
        self.assertToken(ast(0,1), 'Punctuation', content=',')
        self.assertToken(ast(0,2), 'Space', count=1)
        self.assertToken(ast(0,3), 'Link', size=1, url='foo.md')
        self.assertToken(ast(0,3,0), 'Word', content='Foo')

        text = '[`[Foo]`](foo)'
        ast = self.tokenize(text)
        self.assertToken(ast(0), 'Paragraph', size=1)
        self.assertToken(ast(0,0), 'Link', size=1, url='foo')
        self.assertToken(ast(0,0,0), 'Monospace', content='[Foo]')

if __name__ == '__main__':
    unittest.main(verbosity=2)
