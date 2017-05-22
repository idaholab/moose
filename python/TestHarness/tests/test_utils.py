#!/usr/bin/env python
import unittest
import path_tool
path_tool.activate_module('TestHarness')
from util import *

class TestUtils(unittest.TestCase):
    def testRerverseReachability1(self):
        r = ReverseReachability()

        r.insertDependency('f', ['d'])
        r.insertDependency('e', ['d'])
        r.insertDependency('d', ['b'])
        r.insertDependency('c', ['b'])
        r.insertDependency('b', ['a'])

        all_sets = r.getReverseReachabilitySets()
        self.assertEqual(all_sets['a'], set(['b', 'c', 'd', 'e', 'f']))
        self.assertEqual(all_sets['b'], set(['c', 'd', 'e', 'f']))
        self.assertEqual(all_sets['c'], set())
        self.assertEqual(all_sets['d'], set(['e', 'f']))
        self.assertEqual(all_sets['e'], set())
        self.assertEqual(all_sets['f'], set())


    def testRerverseReachability2(self):
        r = ReverseReachability()

        r.insertDependency('c', ['a'])
        r.insertDependency('a', ['b'])

        all_sets = r.getReverseReachabilitySets()
        self.assertEqual(all_sets['a'], set('c'))
        self.assertEqual(all_sets['b'], set(['a', 'c']))
        self.assertEqual(all_sets['c'], set())


    def testRerverseReachability3(self):
        r = ReverseReachability()

        r.insertDependency('aa', ['dd'])
        r.insertDependency('bb', ['aa', 'ee'])
        r.insertDependency('cc', ['aa', 'ee'])

        all_sets = r.getReverseReachabilitySets()
        self.assertEqual(all_sets['aa'], set(['cc', 'bb']))
        self.assertEqual(all_sets['bb'], set())
        self.assertEqual(all_sets['cc'], set())
        self.assertEqual(all_sets['dd'], set(['aa', 'bb', 'cc']))
        self.assertEqual(all_sets['ee'], set(['cc', 'bb']))


    def testRerverseReachabilityCyclic(self):
        r = ReverseReachability()

        r.insertDependency('c', ['b'])
        r.insertDependency('b', ['a'])
        r.insertDependency('a', ['c', 'd'])

        all_sets = r.getReverseReachabilitySets()
        self.assertEqual(all_sets['a'], set(['a', 'b', 'c']))
        self.assertEqual(all_sets['b'], set(['a', 'b', 'c']))
        self.assertEqual(all_sets['c'], set(['a', 'b', 'c']))
        self.assertEqual(all_sets['d'], set(['a', 'b', 'c']))

if __name__ == '__main__':
    unittest.main(module=__name__, verbosity=2)
