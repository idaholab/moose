#!/usr/bin/env python
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

if __name__ == '__main__':
    unittest.main(verbosity=2)
