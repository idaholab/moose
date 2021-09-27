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
from MooseDocs.extensions import core, command, table, floats, coverage
logging.basicConfig()

class TestTable(MooseDocsTestCase):
    EXTENSIONS = [core, command, table, floats, coverage]

    def testAST(self):
       ast = self.tokenize('!coverage table')
       self.assertSize(ast, 1)
       self.assertToken(ast(0), 'Table', size=2)

    def testInlineError(self):
       ast = self.tokenize('[!coverage!table]')
       self.assertSize(ast, 1)
       self.assertToken(ast(0,0), 'ErrorToken',
                        message="The 'coverage table' command is a block level command, use '!coverage table' instead.")

class TestValue(MooseDocsTestCase):
    EXTENSIONS = [core, command, table, floats, coverage]

    def testAST(self):
       ast = self.tokenize('!coverage value section=framework option=require_total')
       self.assertSize(ast, 1)
       self.assertToken(ast(0), 'String', size=0)

       ast = self.tokenize('[!coverage!value section=framework option=require_total]')
       self.assertSize(ast, 1)
       self.assertToken(ast(0,0), 'String', size=0)

    def testSectionError(self):
       ast = self.tokenize('[!coverage!value section=wrong option=require_total]')
       self.assertSize(ast, 1)
       self.assertToken(ast(0,0), 'ErrorToken',
                        message="The 'wrong' section does not exist in coverage file.")

       ast = self.tokenize('[!coverage!value section=framework option=wrong]')
       self.assertSize(ast, 1)
       self.assertToken(ast(0,0), 'ErrorToken',
                        message="The 'wrong' option does not exist in 'framework' section of the coverage file.")


if __name__ == '__main__':
    unittest.main(verbosity=2)
