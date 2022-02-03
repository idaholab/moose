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
from MooseDocs.extensions import core, command, floats, algorithm
from MooseDocs import base
logging.basicConfig()

class TestAlgorithm(MooseDocsTestCase):
    EXTENSIONS = [core, command, floats, algorithm]
    TEXT = ("!algorithm\n"
            "[!function!begin name=testfunction param=a]\n"
            "[!procedure!begin name=testprocedure param=b]\n"
            "[!for!begin condition=1,N]\n"
            "[!while!begin condition=True]\n"
            "[!ifthen!if condition=false]\n"
            "[!state text=Statement 1]\n"
            "[!ifthen!elif condition=false]\n"
            "[!state text=Statement 2]\n"
            "[!ifthen!else]\n"
            "[!state text=Statement 3 comment=Comment]\n"
            "[!ifthen!end]\n"
            "[!while!end]\n"
            "[!for!end]\n"
            "[!procedure!end]\n"
            "[!function!end]")

    NLINES = 15
    LEVELS = [0, 1, 2, 3, 4, 5, 4, 5, 4, 5, 4, 3, 2, 1, 0]
    CONTENT = ['+function+ testfunction(a)',
               '+procedure+ testprocedure(b)',
               '+for+ 1,N +do+',
               '+while+ True +do+',
               '+if+ False +then+',
               'Statement 1',
               '+else if+ False +then+',
               'Statement 2',
               '+else+',
               'Statement 3',
               '+end if+',
               '+end while+',
               '+end for+',
               '+end procedure+',
               '+end function+']
    COMMENT = [None, None, None, None, None, None, None, None, None, 'Comment', None, None, None, None, None]

    def testAST(self):
        ast = self.tokenize(self.TEXT)

        self.assertEqual(len(ast), 1)
        self.assertEqual(len(ast(0)), 2 * self.NLINES - 1)

        self.assertToken(ast(0), 'Algorithm')
        for i in range(self.NLINES):
            self._assertAlgorithmToken(ast(0)(i*2), i+1, self.LEVELS[i], self.CONTENT[i], self.COMMENT[i])

    def _assertAlgorithmToken(self, ast, line, level, content=None, comment=None):
        self.assertToken(ast, 'AlgorithmComponent', line=line, level=level)


if __name__ == '__main__':
    unittest.main(verbosity=2)
