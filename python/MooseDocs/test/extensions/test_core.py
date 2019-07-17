#!/usr/bin/env python3
import unittest
import logging
from MooseDocs.test import MooseDocsTestCase
from MooseDocs.extensions import core
from MooseDocs import base
logging.basicConfig()

class TestCore(MooseDocsTestCase):
    EXTENSIONS = [core]

    def testCodeBlock(self):
        text = u"```\nint x = 0;\n```"
        ast = self.tokenize(text)
        self.assertToken(ast(0), 'Code', language=u'text', content=u'\nint x = 0;\n', escape=True)

        def helper(r):
            self.assertHTMLTag(r(0), 'pre', class_='moose-pre')
            self.assertHTMLTag(r(0)(0), 'code', class_='language-text')
            self.assertHTMLString(r(0)(0)(0), u'\nint x = 0;\n', escape=True)

        res = self.render(ast)
        self.assertHTMLTag(res, 'body')
        helper(res)

        res = self.render(ast, renderer=base.MaterializeRenderer())
        self.assertHTMLTag(res, 'div')
        helper(res)

        tex = self.render(ast, renderer=base.LatexRenderer())
        self.assertLatex(tex(0), 'Environment', 'verbatim',
                         after_begin='\n', before_end='\n', escape=False)
        self.assertLatexString(tex(0)(0), u'int x = 0;', escape=False)

if __name__ == '__main__':
    unittest.main(verbosity=2)
