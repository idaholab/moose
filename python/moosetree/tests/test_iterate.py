#!/usr/bin/env python3
import unittest
import moosetree


def build_tree():
    root = moosetree.Node(None, "root")
    A = moosetree.Node(root, "A")
    B = moosetree.Node(root, "B")
    C = moosetree.Node(root, "C", year=2011)
    D = moosetree.Node(root, "D")

    AA = moosetree.Node(A, "AA")
    AB = moosetree.Node(A, "AB")
    AC = moosetree.Node(A, "AC")

    BA = moosetree.Node(B, "BA")
    BB = moosetree.Node(B, "BB", year=1949)
    BC = moosetree.Node(B, "BC")
    BD = moosetree.Node(B, "BD")

    CA = moosetree.Node(C, "CA")
    CB = moosetree.Node(C, "CB", year=1980)
    CC = moosetree.Node(C, "CC")

    DA = moosetree.Node(D, "DA")
    DB = moosetree.Node(D, "DB")

    ABA = moosetree.Node(AB, "ABA")
    ABB = moosetree.Node(AB, "ABB")
    ABC = moosetree.Node(AB, "ABC", year=1980)
    ABD = moosetree.Node(AB, "ABD")

    BAA = moosetree.Node(BA, "BAA")
    BAB = moosetree.Node(BA, "BAB", year=1954)

    CCA = moosetree.Node(CC, "CCA")
    CCB = moosetree.Node(CC, "CCB")
    CCC = moosetree.Node(CC, "CCC")

    ABCA = moosetree.Node(ABC, "ABCA")
    ABCB = moosetree.Node(ABC, "ABCB")

    ABCAA = moosetree.Node(ABCA, "ABCAA")
    ABCAB = moosetree.Node(ABCA, "ABCAB", year=2013)
    return root


class TestIterator(unittest.TestCase):
    def testPreOrder(self):
        root = build_tree()
        nodes = list(moosetree.iterate(root, method=moosetree.IterMethod.PRE_ORDER))
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
        nodes = list(moosetree.iterate(root))
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
