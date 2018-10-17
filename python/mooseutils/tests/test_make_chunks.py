#!/usr/bin/env python

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
