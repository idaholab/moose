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

class TestBox(unittest.TestCase):
    def testBasic(self):
        b = common.box('foo\nbar', 'title', 42, 12, color=None)
        gold = 'title\n  \u250c\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2510\n42\u2502' \
               'foo     \u2502\n43\u2502bar     \u2502\n  \u2514\u2500\u2500\u2500\u2500\u2500' \
               '\u2500\u2500\u2500\u2518'
        self.assertEqual(b, gold)

if __name__ == '__main__':
    unittest.main(verbosity=2)
