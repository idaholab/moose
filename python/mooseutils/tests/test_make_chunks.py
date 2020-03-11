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
import mooseutils

class Test(unittest.TestCase):

    def setUp(self):
        self.data = [1,2,3,4,5,6,7,8,9,10,11]

    def assertChunk(self, n, gold):
        out = list(mooseutils.make_chunks(self.data, n))
        self.assertEqual(out, gold)

    def test(self):
        self.assertChunk(1, [self.data])
        self.assertChunk(2, [[1,2,3,4,5,6], [7,8,9,10,11]])
        self.assertChunk(3, [[1,2,3,4], [5,6,7,8], [9,10,11]])
        self.assertChunk(4, [[1,2,3], [4,5,6], [7,8,9], [10,11]])
        self.assertChunk(5, [[1,2,3], [4,5], [6,7], [8,9], [10,11]])
        self.assertChunk(6, [[1,2], [3,4], [5,6], [7,8], [9,10], [11]])
        self.assertChunk(7, [[1,2], [3,4], [5,6], [7,8], [9], [10], [11]])
        self.assertChunk(8, [[1,2], [3,4], [5,6], [7], [8], [9], [10], [11]])
        self.assertChunk(9, [[1,2], [3,4], [5], [6], [7], [8], [9], [10], [11]])
        self.assertChunk(10, [[1,2], [3], [4], [5], [6], [7], [8], [9], [10], [11]])
        self.assertChunk(11, [[1], [2], [3], [4], [5], [6], [7], [8], [9], [10], [11]])
        self.assertChunk(12, [[1], [2], [3], [4], [5], [6], [7], [8], [9], [10], [11], []])

if __name__ == '__main__':
    unittest.main(module=__name__, verbosity=2)
