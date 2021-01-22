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
from MooseDocs import common
from MooseDocs.tree import tokens

class TestHasTokens(unittest.TestCase):
    def testBasic(self):
        root = tokens.Token('', None)
        tokens.Token('Test', root)

        self.assertTrue(common.has_tokens(root, 'Test'))
        self.assertFalse(common.has_tokens(root, 'Nope'))

if __name__ == '__main__':
    unittest.main(verbosity=2)
