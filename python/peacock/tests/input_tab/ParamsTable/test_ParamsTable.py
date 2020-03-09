#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from peacock.Input.ParamsTable import ParamsTable
from peacock.utils import Testing, InputTesting
from peacock.Input.ParameterInfo import ParameterInfo
from peacock.Input.BlockInfo import BlockInfo
from PyQt5.QtWidgets import QFileDialog, QApplication
from mock import patch

class Tests(Testing.PeacockTester):
    qapp = QApplication([])

    def setUp(self):
        super(Tests, self).setUp()
        self.table = None
        self.changed = 0
        self.block_list_requested = 0
        self.block_children = ["child0", "child1", "child2"]

    def commentsChanged(self):
        self.comments_changed += 1

    def createParam(self, name, value="", cpp_type="string", options=[], required=False, user_added=False, basic_type="String"):
        p = ParameterInfo(None, name)
        p.value = value
        p.cpp_type = cpp_type
        p.basic_type = basic_type
        p.options = options
        p.required = required
        p.user_added = user_added
        return p

    def needBlockList(self, w, blocks):
        self.block_list_requested += 1
        for b in blocks:
            w.setWatchedBlockList(b, self.block_children)

    def onChanged(self):
        self.changed += 1

    def createTable(self, params):
        b = BlockInfo(None, "/Foo")
        for p in params:
            b.addParameter(p)
        tmap = {"VariableName": ["/Variables", "/AuxVariables"]}
        t = ParamsTable(b, params, tmap)
        t.resize(480, 480)
        t.addName("some name")
        t.addName("some name") # shouldn't be a problem
        t.addUserParam("user_param")
        t.needBlockList.connect(lambda paths: self.needBlockList(t, paths))
        t.changed.connect(self.onChanged)
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
        params.append(self.createParam("p7", cpp_type="vector", options=options, basic_type="Array"))
        params.append(self.createParam("p8"))
        params.append(self.createParam("p9", cpp_type="VariableName"))
        params.append(self.createParam("p10", cpp_type="vector<VariableName>", basic_type="Array"))
        return params

    def testEmpty(self):
        b = BlockInfo(None, "/Foo")
        t = ParamsTable(b, [], {})
        t.needBlockList.connect(lambda paths: self.needBlockList(t, paths))
        self.assertEqual(t.rowCount(), 0)
        t.setWatchedBlockList("/Bar", [])

    def testParamRename(self):
        t = self.createTable(self.createParams())
        row = t.findRow("user_param")
        self.assertEqual(t.block.getParamInfo("user_param"), None)
        InputTesting.changeTableCell(t, "user_param", 0, "new_param")
        new_row = t.findRow("new_param")
        self.assertEqual(row, new_row)

        t.save()
        self.assertNotEqual(t.block.getParamInfo("new_param"), None)

        InputTesting.changeTableCell(t, "new_param", 0, "new_param1")
        new_row = t.findRow("new_param1")
        self.assertEqual(row, new_row)
        self.assertNotEqual(t.block.getParamInfo("new_param"), None)

        t.reset()
        self.assertNotEqual(t.block.getParamInfo("new_param"), None)
        new_row = t.findRow("new_param1")
        self.assertEqual(new_row, -1)
        new_row = t.findRow("new_param")
        self.assertEqual(new_row, row)

    def testParamRemoved(self):
        t = self.createTable(self.createParams())
        t.save()
        count_before = t.rowCount()
        row = t.findRow("user_param")
        InputTesting.clickTableButton(t, "user_param", 2)
        self.assertEqual(t.rowCount(), count_before - 1)
        self.assertNotEqual(t.block.getParamInfo("user_param"), None)
        new_row = t.findRow("user_param")
        self.assertEqual(new_row, -1)
        t.reset()

        self.assertNotEqual(t.block.getParamInfo("user_param"), None)
        new_row = t.findRow("user_param")
        self.assertEqual(new_row, row)

        InputTesting.clickTableButton(t, "user_param", 2)
        t.save()
        self.assertEqual(t.block.getParamInfo("user_param"), None)

    def testParamAdded(self):
        t = self.createTable(self.createParams())
        t.save()
        count_before = t.rowCount()
        t.addUserParam("new_param")
        count_after = t.rowCount()
        self.assertEqual(count_before+1, count_after)
        row = t.findRow("new_param")
        self.assertEqual(row, count_after-1)
        self.assertEqual(t.block.getParamInfo("new_param"), None)

        t.reset()
        row = t.findRow("new_param")
        self.assertEqual(row, -1)
        count_after = t.rowCount()
        self.assertEqual(count_before, count_after)

        t.addUserParam("new_param")
        t.save()
        self.assertNotEqual(t.block.getParamInfo("new_param"), None)

    def getItem(self, t, param, col):
        row = t.findRow(param)
        item = t.item(row, col)
        return item

    def changeParam(self, t, param, col, new_val, final_value=None, button=False):
        if col == 1 or col == 3:
            InputTesting.changeTableCell(t, param, col, new_val)
        elif col == 2:
            if button:
                InputTesting.clickTableButton(t, param, col)
            else:
                InputTesting.changeTableCombo(t, param, col, new_val)

        t.save()
        p = t.block.getParamInfo(param)
        self.assertNotEqual(p, None)
        if final_value is None:
            final_value = new_val
        if col == 1 or col == 2:
            self.assertEqual(p.value, final_value)
        else:
            self.assertEqual(p.comments, final_value)

    def testParamChanged(self):
        t = self.createTable(self.createParams())
        self.changeParam(t, "p0", 1, "new_value")
        self.changeParam(t, "p5", 2, "option_1")
        self.changeParam(t, "p5", 2, "option_2")
        self.changeParam(t, "p7", 2, "option_1")
        self.changeParam(t, "p7", 2, "option_2", "option_1 option_2")
        self.changeParam(t, "p7", 2, "option_0", "option_1 option_2 option_0")
        self.changeParam(t, "p9", 2, "child1")
        self.changeParam(t, "p9", 2, "child2")
        self.changeParam(t, "p10", 2, "child1")
        self.changeParam(t, "p10", 2, "child2", "child1 child2")

    def testParamComments(self):
        t = self.createTable(self.createParams())
        self.changeParam(t, "p0", 3, "some comments")
        self.changeParam(t, "p0", 3, "more comments")
        self.changeParam(t, "p1", 3, "f")

    @patch.object(QFileDialog, "getOpenFileName")
    def testFiles(self, mock_file):
        mock_file.return_value = (None, None)
        t = self.createTable(self.createParams())
        self.changeParam(t, "p2", 2, "", button=True)

        mock_file.return_value = ("foo", "filter")
        self.changeParam(t, "p2", 2, "foo", button=True)

        mock_file.return_value = ("bar", "filter")
        self.changeParam(t, "p2", 2, "bar", button=True)

        mock_file.return_value = ("foo", "filter")
        self.changeParam(t, "p3", 2, "foo", button=True)

        mock_file.return_value = ("bar", "filter")
        self.changeParam(t, "p3", 2, "bar", button=True)

    def testWatchers(self):
        t = self.createTable(self.createParams())
        row = t.findRow("p9")
        combo = t.cellWidget(row, 2)
        self.assertEqual(combo.count(), 7)
        self.block_children = []
        t.updateWatchers()
        self.assertEqual(combo.count(), 1)

if __name__ == '__main__':
    Testing.run_tests()
