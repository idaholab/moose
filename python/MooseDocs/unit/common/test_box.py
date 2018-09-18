#!/usr/bin/env python2

import unittest
from MooseDocs import common

class TestBox(unittest.TestCase):
    def testBasic(self):
        b = common.box('foo\nbar', 'title', 42, 12)
        gold = u'title\n  \u250c\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2510\n42\u2502' \
               u'foo     \u2502\n43\u2502bar     \u2502\n  \u2514\u2500\u2500\u2500\u2500\u2500' \
               u'\u2500\u2500\u2500\u2518'
        self.assertEqual(b, gold)

if __name__ == '__main__':
    unittest.main(verbosity=2)
