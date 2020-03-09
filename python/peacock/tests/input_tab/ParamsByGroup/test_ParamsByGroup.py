#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from peacock.Input.ParamsByGroup import ParamsByGroup
from peacock.utils import Testing
from peacock.Input.ParameterInfo import ParameterInfo
from peacock.Input.BlockInfo import BlockInfo
from PyQt5 import QtWidgets

class Tests(Testing.PeacockTester):
    qapp = QtWidgets.QApplication([])

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

    def createTabs(self, params):
        b = BlockInfo(None, "/Foo")
        for p in params:
            b.addParameter(p)
        tmap = {"VariableName": ["/Variables"]}
        t = ParamsByGroup(b, params, tmap)
        t.resize(480, 480)
        t.addName("some name")
        t.addUserParam("user_param")
        t.needBlockList.connect(lambda paths: self.needBlockList(t, paths))
        t.updateWatchers()
        if params:
            self.assertEqual(self.block_list_requested, 1)
        t.show()
        return t

    def createParams(self):
        params = []
        options = ["option_0", "option_1", "option_2"]
        params.append(self.createParam("p0"))
        params.append(self.createParam("p1", value="some val", required=True))
        params.append(self.createParam("p2", cpp_type="FileName"))
        params.append(self.createParam("p3", cpp_type="FileNameNoExtension"))
        params.append(self.createParam("p4", cpp_type="MeshFileName"))
        params.append(self.createParam("p5", options=options))
        params.append(self.createParam("p7", cpp_type="vector", options=options))
        params.append(self.createParam("p8"))
        params.append(self.createParam("p9", cpp_type="VariableName"))
        params.append(self.createParam("p10", cpp_type="vector<VariableName>"))
        params.append(self.createParam("p11", group="Group1"))
        params.append(self.createParam("p12", group="Group2"))
        params.append(self.createParam("p13", group="Group1"))
        params.append(self.createParam("p14", group="Group2"))
        return params

    def testEmpty(self):
        b = BlockInfo(None, "/Foo")
        t = ParamsByGroup(b, [], {})
        t.needBlockList.connect(lambda paths: self.needBlockList(t, paths))
        self.assertEqual(t.count(), 0)
        t.setWatchedBlockList(t.count(), [])
        t.save()
        t.reset()

    def testParamAdded(self):
        t = self.createTabs(self.createParams())
        table = t.findTable("Main")
        count_before = table.rowCount()
        t.addUserParam("new_param")
        count_after = table.rowCount()
        self.assertEqual(count_before+1, count_after)
        row = table.findRow("new_param")
        self.assertEqual(row, count_after-1)
        t.save()
        self.assertNotEqual(t.block.getParamInfo("new_param"), None)

        t.addUserParam("param1")
        row = table.findRow("param1")
        self.assertGreater(row, 0)
        self.assertEqual(t.block.getParamInfo("param1"), None)
        t.reset()
        row = table.findRow("param1")
        self.assertEqual(row, -1)


if __name__ == '__main__':
    Testing.run_tests()
