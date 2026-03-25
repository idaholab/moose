# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

import unittest

from moosetools import tree


def build_tree():
    root = tree.Node(None, "root")
    A = tree.Node(root, "A")
    B = tree.Node(root, "B")
    C = tree.Node(root, "C", year=2011)
    D = tree.Node(root, "D")

    AA = tree.Node(A, "AA")
    AB = tree.Node(A, "AB")
    AC = tree.Node(A, "AC")

    BA = tree.Node(B, "BA")
    BB = tree.Node(B, "BB", year=1949)
    BC = tree.Node(B, "BC")
    BD = tree.Node(B, "BD")

    CA = tree.Node(C, "CA")
    CB = tree.Node(C, "CB", year=1980)
    CC = tree.Node(C, "CC")

    DA = tree.Node(D, "DA")
    DB = tree.Node(D, "DB")

    ABA = tree.Node(AB, "ABA")
    ABB = tree.Node(AB, "ABB")
    ABC = tree.Node(AB, "ABC", year=1980)
    ABD = tree.Node(AB, "ABD")

    BAA = tree.Node(BA, "BAA")
    BAB = tree.Node(BA, "BAB", year=1954)

    CCA = tree.Node(CC, "CCA")
    CCB = tree.Node(CC, "CCB")
    CCC = tree.Node(CC, "CCC")

    ABCA = tree.Node(ABC, "ABCA")
    ABCB = tree.Node(ABC, "ABCB")

    ABCAA = tree.Node(ABCA, "ABCAA")
    ABCAB = tree.Node(ABCA, "ABCAB", year=2013)
    return root


class TestIterator(unittest.TestCase):
    def testPreOrder(self):
        root = build_tree()
        nodes = list(tree.iterate(root, method=tree.IterMethod.PRE_ORDER))
        self.assertEqual(len(nodes), 29)
        self.assertEqual(nodes[0].name, "A")
        self.assertEqual(nodes[1].name, "AA")
        self.assertEqual(nodes[2].name, "AB")
        self.assertEqual(nodes[3].name, "ABA")
        self.assertEqual(nodes[4].name, "ABB")
        self.assertEqual(nodes[5].name, "ABC")
        self.assertEqual(nodes[6].name, "ABCA")
        self.assertEqual(nodes[7].name, "ABCAA")
        self.assertEqual(nodes[8].name, "ABCAB")
        self.assertEqual(nodes[9].name, "ABCB")
        self.assertEqual(nodes[10].name, "ABD")
        self.assertEqual(nodes[11].name, "AC")
        self.assertEqual(nodes[12].name, "B")
        self.assertEqual(nodes[13].name, "BA")
        self.assertEqual(nodes[14].name, "BAA")
        self.assertEqual(nodes[15].name, "BAB")
        self.assertEqual(nodes[16].name, "BB")
        self.assertEqual(nodes[17].name, "BC")
        self.assertEqual(nodes[18].name, "BD")
        self.assertEqual(nodes[19].name, "C")
        self.assertEqual(nodes[20].name, "CA")
        self.assertEqual(nodes[21].name, "CB")
        self.assertEqual(nodes[22].name, "CC")
        self.assertEqual(nodes[23].name, "CCA")
        self.assertEqual(nodes[24].name, "CCB")
        self.assertEqual(nodes[25].name, "CCC")
        self.assertEqual(nodes[26].name, "D")
        self.assertEqual(nodes[27].name, "DA")
        self.assertEqual(nodes[28].name, "DB")

    def testBreadthFirst(self):
        root = build_tree()
        nodes = list(tree.iterate(root))
        self.assertEqual(len(nodes), 29)
        self.assertEqual(nodes[0].name, "A")
        self.assertEqual(nodes[1].name, "B")
        self.assertEqual(nodes[2].name, "C")
        self.assertEqual(nodes[3].name, "D")
        self.assertEqual(nodes[4].name, "AA")
        self.assertEqual(nodes[5].name, "AB")
        self.assertEqual(nodes[6].name, "AC")
        self.assertEqual(nodes[7].name, "BA")
        self.assertEqual(nodes[8].name, "BB")
        self.assertEqual(nodes[9].name, "BC")
        self.assertEqual(nodes[10].name, "BD")
        self.assertEqual(nodes[11].name, "CA")
        self.assertEqual(nodes[12].name, "CB")
        self.assertEqual(nodes[13].name, "CC")
        self.assertEqual(nodes[14].name, "DA")
        self.assertEqual(nodes[15].name, "DB")
        self.assertEqual(nodes[16].name, "ABA")
        self.assertEqual(nodes[17].name, "ABB")
        self.assertEqual(nodes[18].name, "ABC")
        self.assertEqual(nodes[19].name, "ABD")
        self.assertEqual(nodes[20].name, "BAA")
        self.assertEqual(nodes[21].name, "BAB")
        self.assertEqual(nodes[22].name, "CCA")
        self.assertEqual(nodes[23].name, "CCB")
        self.assertEqual(nodes[24].name, "CCC")
        self.assertEqual(nodes[25].name, "ABCA")
        self.assertEqual(nodes[26].name, "ABCB")
        self.assertEqual(nodes[27].name, "ABCAA")
        self.assertEqual(nodes[28].name, "ABCAB")


if __name__ == "__main__":
    unittest.main(verbosity=2)
