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
from MooseDocs.extensions import core, command, floats, alert, materialicon, template
from MooseDocs import base, common, tree
logging.basicConfig()

TEXT = """!template load file=example.template.md project=MOOSE

!template item key=field-with-item
This is some content that should be below the second heading.
"""

class TestTemplate(MooseDocsTestCase):
    EXTENSIONS = [core, command, floats, alert, materialicon, template]

    def setupExtension(self, ext):
        if ext == template:
            return dict(active=True)

    def setupContent(self):
        config = [dict(root_dir='python/MooseDocs/test/content', content=['extensions/example.template.md'])]
        x = common.get_content(config, '.md')
        return common.get_content(config, '.md')

    def testAST(self):
        ast = self.tokenize(TEXT)
        self.assertEqual(len(ast), 2)

        self.assertEqual(len(ast(0)), 8)
        self.assertToken(ast(0,0), 'Heading', size=3)
        self.assertToken(ast(0,1), 'Paragraph', size=10)
        self.assertToken(ast(0,2), 'Heading', size=5)
        self.assertToken(ast(0,3), 'TemplateField', size=1)
        self.assertToken(ast(0,3,0), 'Paragraph', size=17)
        self.assertToken(ast(0,4), 'Heading', size=5)
        self.assertToken(ast(0,5), 'TemplateField', size=0)
        self.assertToken(ast(0,6), 'Heading', size=7)
        self.assertToken(ast(0,7), 'TemplateField', size=1)
        self.assertToken(ast(0,7,0), 'Paragraph', size=29)

        self.assertEqual(len(ast(1)), 1)
        self.assertToken(ast(1), 'TemplateItem', size=1)
        self.assertToken(ast(1,0), 'Paragraph', size=22)

    def testRenderField(self):
        _, res = self.execute(TEXT, renderer=base.MaterializeRenderer())
        self.assertHTMLTag(res, 'div', size=8)

        self.assertHTMLTag(res(0), 'h1', size=3)
        self.assertEqual(res(0).text(), 'Template Extension')
        self.assertHTMLTag(res(1), 'p', size=10)
        self.assertEqual(res(1).text(), "The MOOSE project is amazing !")

        self.assertHTMLTag(res(2), 'h2', size=5)
        self.assertEqual(res(2).text(), 'Field with Defaults')
        self.assertHTMLTag(res(3), 'p', size=17)
        self.assertEqual(res(3).text(), "This is the default message , it is great .")

        self.assertHTMLTag(res(4), 'h2', size=5)
        self.assertEqual(res(4).text(), 'Field with Replacement')
        self.assertHTMLTag(res(5), 'p', size=22)
        self.assertEqual(res(5).text(), "This is some content that should be below the second heading .")

        self.assertHTMLTag(res(6), 'h2', size=7)
        self.assertEqual(res(6).text(), 'Field with Missing Replacement')
        self.assertHTMLTag(res(7), 'div', size=2)

if __name__ == '__main__':
    unittest.main(verbosity=2)
