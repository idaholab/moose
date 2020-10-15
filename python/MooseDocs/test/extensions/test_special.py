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
from MooseDocs.extensions import core, special
from MooseDocs import base
logging.basicConfig()

class TestSpecial(MooseDocsTestCase):
    EXTENSIONS = [core, special]
    TEXT = "Sch&auml;dle and Sch&#228;dle"

    def testAST(self):
        ast = self.tokenize(self.TEXT)
        self.assertEqual(len(ast), 1)
        self.assertEqual(len(ast(0)), 9)
        self.assertToken(ast(0), 'Paragraph')
        self.assertToken(ast(0,1), 'String', content='&auml;', escape=False)
        self.assertToken(ast(0,7), 'String', content='&#228;', escape=False)

if __name__ == '__main__':
    unittest.main(verbosity=2)
