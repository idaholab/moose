#!/usr/bin/env python
from peacock.Input.ParamsByType import ParamsByType
from peacock.utils import Testing, InputTesting
from peacock.Input.ParameterInfo import ParameterInfo
from peacock.Input.BlockInfo import BlockInfo

class Tests(Testing.PeacockTester):
    def setUp(self):
        super(Tests, self).setUp()
        self.block_list_requested = 0
        self.block_children = ["child0", "child1", "child2"]

    def createParam(self, name, value="", cpp_type="string", options=[], required=False, user_added=False, group="Main"):
        p = ParameterInfo(None, name)
        p.value = value
        p.cpp_type = cpp_type
        p.options = options
        p.required = required
        p.user_added = user_added
        p.group_name = group
        return p

    def needBlockList(self, w, blocks):
        self.block_list_requested += 1
        for b in blocks:
            w.setWatchedBlockList(b, self.block_children)

    def createWidget(self, block, name="myname", user_params=True):
        t = ParamsByType(block)
        t.resize(480, 480)
        t.needBlockList.connect(lambda paths: self.needBlockList(t, paths))
        if block:
            t.setBlockType("type_0")
            t.block.addUserParam("user1", "foo")
            t.block.addUserParam("user2", "bar")
        t.updateWatchers()
        t.show()
        return t

    def createParams(self, b):
        b.addParameter(self.createParam("%s_p0" % b.name))
        b.addParameter(self.createParam("%s_p1" % b.name, group="Group1"))
        b.addParameter(self.createParam("%s_p2" % b.name, group="Group2"))
        b.addParameter(self.createParam("%s_p3" % b.name, group="Group1"))
        b.addParameter(self.createParam("%s_p4" % b.name, group="Group2"))
        b.addParameter(self.createParam("%s_p5" % b.name))
        b.addParameter(self.createParam("%s_p6" % b.name))

    def createTypes(self):
        b = BlockInfo(None, "Foo")
        b.addParameter(self.createParam("type"))
        b.addParameter(self.createParam("parent0"))
        b.addParameter(self.createParam("parent1"))
        b.addParameter(self.createParam("parent2"))
        for i in range(4):
            bt = BlockInfo(b, "type_%s" % i)
            b.addBlockType(bt)
            self.createParams(bt)
        return b

    def testParams(self):
        t = self.createWidget(self.createTypes(), "")
        self.assertEqual(t.combo.count(), 4)
        self.assertEqual(t.table_stack.count(), 1)
        # add a user param
        t.addUserParam("foo")
        row = t.getTable().findTable("Main").findRow("foo")
        self.assertNotEqual(row, -1)

        # make sure the user param still shows up after switching types
        t.setBlockType("type_2")
        self.assertEqual(t.table_stack.count(), 2)
        self.assertEqual(t.getTable().findTable("Main").findRow("foo"), row)
        t.reset()
        # the user param shouldn't be there after a reset
        self.assertEqual(t.getTable().findTable("Main").findRow("foo"), -1)
        self.assertEqual(t.block.getParamInfo("foo"), None)

        # add user params again
        t.addUserParam("foo1")
        t.addUserParam("foo")
        t.setBlockType("type_3")
        self.assertNotEqual(t.getTable().findTable("Main").findRow("foo"), -1)
        self.assertNotEqual(t.getTable().findTable("Main").findRow("foo1"), -1)
        # remove one of the user params
        InputTesting.clickTableButton(t.getTable().findTable("Main"), "foo1", 2)
        self.assertEqual(t.getTable().findTable("Main").findRow("foo1"), -1)
        self.assertNotEqual(t.getTable().findTable("Main").findRow("foo"), -1)

        t.setBlockType("type_2")
        self.assertEqual(t.combo.currentText(), "type_2")
        self.assertEqual(t.getTable().findTable("Main").findRow("foo1"), -1)
        self.assertNotEqual(t.getTable().findTable("Main").findRow("foo"), -1)
        t.save()

        # saved, so one should be gone and one should be there
        self.assertEqual(t.block.blockType(), "type_2")
        self.assertNotEqual(t.block.getParamInfo("foo"), None)
        self.assertEqual(t.block.getParamInfo("foo1"), None)
        self.assertEqual(t.currentType(), "type_2")

        # had a problem where after we created a user param it
        # wasn't showing up in a new widget
        t = ParamsByType(t.block)
        self.assertEqual(t.block.blockType(), "type_2")
        self.assertEqual(t.combo.currentText(), "type_2")
        self.assertNotEqual(t.getTable().findTable("Main").findRow("foo"), -1)

    def testEmpty(self):
        b = BlockInfo(None, "Foo")
        t = self.createWidget(b)
        self.assertEqual(t.combo.count(), 0)
        self.assertEqual(t.table_stack.count(), 0)

if __name__ == '__main__':
    Testing.run_tests()
