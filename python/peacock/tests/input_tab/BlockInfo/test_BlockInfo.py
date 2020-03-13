#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from peacock.utils import Testing
from peacock.Input.BlockInfo import BlockInfo
from peacock.Input.ParameterInfo import ParameterInfo
from PyQt5 import QtWidgets

class Tests(Testing.PeacockTester):
    qapp = QtWidgets.QApplication([])

    def testBasic(self):
        b = BlockInfo(None, "/Foo", False, "")

        b1 = BlockInfo(None, "/Foo/bar", False, "")
        b.addChildBlock(b1)
        self.assertEqual(b1.parent, b)
        self.assertIn(b1.name, b.children_list)
        self.assertIn(b1.name, b.children)

        b.types[b1.name] = b1
        b.star_node = b1
        out = b.dump()
        self.assertIn("/Foo/bar", out)

    def testTypes(self):
        b = BlockInfo(None, "/Foo", False, "")
        self.assertEqual(b.paramValue("type"), None)
        b.setBlockType("t") # shouldn't do anything
        p = ParameterInfo(None, "type")
        b.addParameter(p)
        self.assertEqual(p.parent, b)
        self.assertEqual(b.paramValue("type"), "")
        b.setBlockType("t") # shouldn't do anything
        self.assertEqual(b.paramValue("type"), "t")
        b1 = BlockInfo(None, "/Foo/t", False, "")
        p = ParameterInfo(None, "p")
        b1.addParameter(p)
        b.types[b1.name] = b1
        self.assertEqual(b.paramValue("p"), "")
        b.setParamValue("p", "val")
        self.assertEqual(b.paramValue("p"), "val")
        b.setBlockType("t1")
        self.assertEqual(b.paramValue("p"), None)

    def testUserParams(self):
        b = BlockInfo(None, "/Foo", False, "")
        p = ParameterInfo(None, "p")
        p.user_added = False
        b.addParameter(p)
        b.addUserParam("bar", "val")
        b.addUserParam("bar", "val")
        p = b.getParamInfo("bar")
        self.assertEqual(p.user_added, True)
        self.assertEqual(b.paramValue("bar"), "val")

        b.setParamValue("bar", "val1")
        self.assertEqual(b.paramValue("bar"), "val1")

        b.removeUserParam("bar1")
        self.assertEqual(b.paramValue("bar"), "val1")

        b.removeUserParam("p")
        self.assertEqual(b.paramValue("p"), "")

        b.removeUserParam("bar")
        self.assertEqual(b.paramValue("bar"), None)
        self.assertNotIn("bar", b.parameters_list)
        self.assertNotIn("bar", b.parameters)

        b.addUserParam("bar", "val")
        b.addUserParam("foo", "val1")
        self.assertEqual(len(b.parameters_list), 3)
        self.assertEqual(b.parameters_list.index("bar"), 1)
        self.assertEqual(b.parameters_list.index("foo"), 2)

        b.moveUserParam("foo", 0)
        self.assertEqual(b.parameters_list.index("bar"), 2)
        self.assertEqual(b.parameters_list.index("foo"), 0)

        b.renameUserParam("bar1", "bar2")
        b.renameUserParam("p", "bar2")
        self.assertEqual(b.paramValue("bar2"), None)
        b.renameUserParam("bar", "bar1")
        self.assertEqual(b.paramValue("bar"), None)
        self.assertEqual(b.paramValue("bar1"), "val")

    def testChild(self):
        b = BlockInfo(None, "/Foo", False, "")
        b2 = BlockInfo(None, "/Foo/bar", False, "")
        b21 = BlockInfo(None, "/Foo/bar/child", False, "")
        b3 = BlockInfo(None, "/Foo/bar1", False, "")
        b2.addChildBlock(b21)
        b.addChildBlock(b2)
        b.addChildBlock(b3)
        self.assertEqual(b2.parent, b)
        self.assertEqual(b3.parent, b)
        self.assertIn(b2.name, b.children)
        self.assertIn(b2.name, b.children_list)
        self.assertEqual(b.children_list.index(b2.name), 0)
        self.assertEqual(b.children_list.index(b3.name), 1)
        b.moveChildBlock(b3.name, 0)
        self.assertEqual(b.children_list.index(b2.name), 1)
        self.assertEqual(b.children_list.index(b3.name), 0)

        b.renameChildBlock("bar", "foo")
        self.assertEqual(b2.path, "/Foo/foo")
        self.assertEqual(b2.children["child"].path, "/Foo/foo/child")

        b.renameChildBlock("foo", "bar1")
        self.assertEqual(b2.path, "/Foo/foo")

        b.renameChildBlock("foo1", "bar1")
        self.assertEqual(b2.path, "/Foo/foo")

        b2.removeChildBlock("foo")
        b2.removeChildBlock("child")
        self.assertEqual(len(b2.children_list), 0)
        self.assertEqual(len(b2.children.keys()), 0)

    def testCopy(self):
        b = BlockInfo(None, "/Foo", False, "")
        b.addUserParam("p0", "val0")
        b.addUserParam("p1", "val1")
        b2 = BlockInfo(None, "/Foo/bar", False, "")
        b2.addUserParam("p2", "val2")
        b2.addUserParam("p3", "val3")
        b3 = BlockInfo(None, "/Foo/bar1", False, "")
        b3.addUserParam("p4", "val4")
        b3.addUserParam("p5", "val5")
        b.addChildBlock(b2)
        b.addChildBlock(b3)
        b.setStarInfo(BlockInfo(None, "Foo/star", False, ""))
        b.addBlockType(BlockInfo(None, "Foo/t", False, ""))
        b_copy = b.copy(None)
        self.assertNotEqual(b_copy, b)
        self.assertEqual(b_copy.name, b.name)
        self.assertEqual(b_copy.types.keys(), b.types.keys())
        self.assertNotEqual(b_copy.types, b.types)
        self.assertEqual(b_copy.children.keys(), b.children.keys())
        self.assertEqual(b_copy.children_list, b.children_list)
        self.assertEqual(b_copy.parameters_list, b.parameters_list)
        self.assertEqual(b_copy.parameters.keys(), b.parameters.keys())

        o = b.dump()
        o1 = b_copy.dump()
        self.assertEqual(o, o1)

if __name__ == '__main__':
    Testing.run_tests()
