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
import datetime as dt
from MooseDocs import common, base
from MooseDocs.test import MooseDocsTestCase
from MooseDocs.extensions import core, command, datetime
logging.basicConfig()

class TestDateTimeToday(MooseDocsTestCase):
    EXTENSIONS = [core, command, datetime]

    def testBlock(self):
        ast = self.tokenize('!datetime today')
        self.assertToken(ast(0), 'DateTime', size=0, format='%Y-%m-%d', inline=False)

        ast = self.tokenize('!datetime today format=%B %d, %Y')
        self.assertToken(ast(0), 'DateTime', size=0, format='%B %d, %Y', inline=False)

    def testInline(self):
        ast = self.tokenize('[!datetime!today]')
        self.assertToken(ast(0), 'Paragraph', size=1)
        self.assertToken(ast(0,0), 'DateTime', size=0, format='%Y-%m-%d', inline=True)

        ast = self.tokenize('[!datetime!today format=%B %d, %Y]')
        self.assertToken(ast(0), 'Paragraph', size=1)
        self.assertToken(ast(0,0), 'DateTime', size=0, format='%B %d, %Y', inline=True)

    def testContentException(self):
        ast = self.tokenize('!datetime today\nContent')
        self.assertToken(ast(0), "ErrorToken")
        self.assertIn("Content is not supported for the 'datetime today' command", ast(0)['message'])

        ast = self.tokenize('[!datetime!today](Content)')
        self.assertToken(ast(0), 'Paragraph', size=1)
        self.assertToken(ast(0,0), "ErrorToken")
        self.assertIn("Content is not supported for the 'datetime today' command", ast(0,0)['message'])

    def testRender(self):
        d = dt.datetime(year=1980, month=6, day=24)
        ast = datetime.DateTime(None, datetime=d, format='%Y-%m-%d')

        res = self.render(ast, renderer=base.HTMLRenderer())
        self.assertHTMLTag(res(0), 'span', class_='moose-datetime', string='1980-06-24')

        ast['inline'] = False
        res = self.render(ast, renderer=base.HTMLRenderer())
        self.assertHTMLTag(res(0), 'p', class_='moose-datetime', string='1980-06-24')

        res = self.render(ast, renderer=base.LatexRenderer())
        self.assertLatexString(res(0), content='1980-06-24')


if __name__ == '__main__':
    unittest.main(verbosity=2)
