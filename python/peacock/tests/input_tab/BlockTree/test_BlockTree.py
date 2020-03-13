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
from PyQt5.QtCore import Qt, QMimeData
from PyQt5.QtWidgets import QMenu, QApplication, QMessageBox
from PyQt5.QtGui import QDragEnterEvent, QDropEvent
from peacock.Input.ExecutableInfo import ExecutableInfo
from peacock.Input.InputTree import InputTree
from peacock.utils import Testing
from peacock.Input.BlockTree import BlockTree
from mock import patch

class Tests(Testing.PeacockTester):
    qapp = QApplication([])

    def setUp(self):
        super(Tests, self).setUp()
        self.test_exe = Testing.find_moose_test_exe()
        self.test_input_file = "../../common/fsp_test.i"
        self.changed_count = 0
        self.block_selected_count = 0
        self.block_clicked_count = 0
        self.block_doubleclicked_count = 0
        self.last_block = None

    def newWidget(self, modules=False):
        app_info = ExecutableInfo()
        if modules:
            app_info.setPath(Testing.find_moose_test_exe(dirname="modules/combined", exe_base="combined"))
        else:
            app_info.setPath(Testing.find_moose_test_exe())
        tree = InputTree(app_info)
        tree.setInputFile(self.test_input_file)
        w = BlockTree(tree)
        w.changed.connect(self.changed)
        w.blockSelected.connect(self.blockSelected)
        w.blockClicked.connect(self.blockClicked)
        w.blockDoubleClicked.connect(self.blockDoubleClicked)
        #w.show()
        #w.raise_()
        return w

    def changed(self, block):
        self.changed_count += 1
        self.last_block = block

    def blockSelected(self, block):
        self.block_selected_count += 1
        self.last_block = block

    def blockClicked(self, block):
        self.block_clicked_count += 1
        self.last_block = block

    def blockDoubleClicked(self, block):
        self.block_doubleclicked_count += 1
        self.last_block = block

    def checkInputItem(self, item, name, num_children, checked):
        self.assertEqual(item.text(0), name)
        node = self.widget.getNode(item)
        self.assertEqual(node.name, name)
        self.assertEqual(item.checkState(0) == Qt.Checked, checked)
        self.assertEqual(len(node.children), num_children)
        self.assertEqual(item.childCount(), num_children)

    def numChildrenChecked(self, item):
        count = 0
        for i in range(item.childCount()):
            if item.child(i).checkState(0) == Qt.Checked:
                count += 1
        return count

    def testDumpTree(self):
        # Just get some coverage
        w = self.newWidget(modules=True)
        s = w.dumpTreeToString()
        self.assertIn("Preconditioning: True: True", s)
        self.assertIn("  FSP: True: True", s)
        self.assertIn("    u: False: True", s)
        self.assertIn("    uv: False: True", s)
        self.assertIn("    v: False: True", s)

    def testChanges(self):
        w = self.newWidget()
        item = w._path_item_map.get("/Mesh")
        self.assertNotEqual(item, None)
        self.assertEqual(self.changed_count, 0)
        b = w.tree.getBlockInfo("/Mesh")
        self.assertEqual(b.included, True)

        item.setCheckState(0, Qt.Unchecked)
        self.assertEqual(self.changed_count, 1)
        self.assertEqual(b.included, False)
        self.assertEqual(self.last_block, b)

        item.setCheckState(0, Qt.Checked)
        self.assertEqual(self.changed_count, 2)
        self.assertEqual(b.included, True)

        self.assertEqual(self.block_clicked_count, 0)
        w.onItemClicked(item, 0)
        self.assertEqual(self.block_clicked_count, 1)
        self.assertEqual(b, self.last_block)

        self.assertEqual(self.block_doubleclicked_count, 0)
        w.onItemDoubleClicked(item, 0)
        self.assertEqual(self.block_doubleclicked_count, 1)
        self.assertEqual(b, self.last_block)

    @unittest.skip("Requires work for Python3")
    def testMove(self):
        w = self.newWidget()
        item = w._path_item_map.get("/Variables/u")
        #b = w.tree.getBlockInfo("/Variables/u")
        w.scrollToItem(item)
        point = w.visualItemRect(item).center()
        item1 = w._path_item_map.get("/Variables/v")
        #b1 = w.tree.getBlockInfo("/Variables/v")
        w.scrollToItem(item1)
        point1 = w.visualItemRect(item1).bottomLeft()

        #idx = b.parent.children_list.index(b.name)
        #idx1 = b.parent.children_list.index(b1.name)
        w.setCurrentItem(item)
        mime = QMimeData()
        mime.setData(w._mime_type, "some data")
        ee = QDragEnterEvent(w.mapToGlobal(point), Qt.MoveAction, mime, Qt.LeftButton, Qt.NoModifier)
        w.dragEnterEvent(ee)
        #Testing.process_events(t=1)
        de = QDropEvent(w.mapToGlobal(point1), Qt.MoveAction, mime, Qt.LeftButton, Qt.NoModifier)
        w.dropEvent(de)
        # This doesn't seem to work for some reason
        #self.assertEqual(idx1, b.parent.children_list.index(b.name))
        #self.assertEqual(idx, b.parent.children_list.index(b1.name))

        w.setCurrentItem(None)
        self.assertEqual(w._current_drag, None)
        w.dropEvent(de)

        w.dragEnterEvent(ee)
        self.assertEqual(w._current_drag, None)
        w.setCurrentItem(item1)
        w.dragEnterEvent(ee)
        self.assertNotEqual(w._current_drag, None)
        w.dropEvent(de)

    def testRename(self):
        w = self.newWidget()
        b = w.tree.getBlockInfo("/Variables/u")
        w.renameBlock(b, "foo")
        self.assertEqual(b.path, "/Variables/foo")
        self.assertEqual(b.name, "foo")
        item = w._path_item_map.get("/Variables/foo")
        self.assertNotEqual(item, None)
        self.assertEqual(item.text(0), "foo")
        b1 = w.tree.getBlockInfo("/Variables/u")
        self.assertEqual(b1, None)
        b1 = w.tree.getBlockInfo("/Variables/foo")
        self.assertEqual(b1, b)

    @patch.object(QMenu, "exec_")
    @patch.object(QMessageBox, "question")
    def testRemove(self, mock_question, mock_exec):
        w = self.newWidget()
        b = w.tree.getBlockInfo("/Variables/u")
        parent = b.parent
        self.assertEqual(len(parent.children_list), 2)
        w.removeBlock(b)
        self.assertEqual(b.parent, None)
        self.assertNotIn(b.name, parent.children_list)
        item = w._path_item_map.get("/Variables/u")
        self.assertEqual(item, None)
        self.assertEqual(len(parent.children_list), 1)

        b = w.tree.getBlockInfo("/Variables/v")
        mock_exec.return_value = w.remove_action
        mock_question.return_value = QMessageBox.No
        item = w._path_item_map.get(b.path)
        self.assertNotEqual(item, None)
        w.scrollToItem(item)
        point = w.visualItemRect(item).center()
        w._treeContextMenu(point)
        self.assertEqual(len(parent.children_list), 1)

        mock_question.return_value = QMessageBox.Yes
        w._treeContextMenu(point)
        self.assertEqual(len(parent.children_list), 0)
        item = w._path_item_map.get("/Variables/v")
        self.assertEqual(item, None)


    @patch.object(QMenu, "exec_")
    def testCopy(self, mock_exec):
        w = self.newWidget()
        v = w.tree.getBlockInfo("/Variables")
        b = w.tree.getBlockInfo("/Variables/u")
        self.assertEqual(len(b.children_list), 1)
        nchilds = len(v.children_list)
        self.assertEqual(self.changed_count, 0)
        w.copyBlock(b)
        self.assertEqual(nchilds+1, len(v.children_list))
        self.assertEqual(self.changed_count, 1)
        new_block = w.tree.getBlockInfo("/Variables/New_0")
        self.assertEqual(len(new_block.children_list), 1)
        self.assertEqual(["InitialCondition"], new_block.children_list)
        item = w._path_item_map.get("/Variables/New_0/InitialCondition")
        self.assertNotEqual(item, None)

        # simulate them not choosing any of the menu options
        mock_exec.return_value = None
        nchilds = len(v.children_list)
        item = w._path_item_map.get(b.path)
        w.scrollToItem(item)
        point = w.visualItemRect(item).center()
        w._treeContextMenu(point)

        self.assertEqual(nchilds, len(v.children_list))
        self.assertEqual(self.changed_count, 1)

        mock_exec.return_value = w.add_action
        w._treeContextMenu(point)

        # simulate choosing the add menu option
        self.assertEqual(nchilds+1, len(v.children_list))
        self.assertEqual(self.changed_count, 2)

        # /Variables is a star so we can copy it
        b = w.tree.getBlockInfo("/Variables")
        nchilds = len(v.children_list)
        w.copyBlock(b)
        self.assertEqual(nchilds+1, len(v.children_list))
        self.assertEqual(self.changed_count, 3)
        p = "/Variables/%s" % v.children_list[-1]
        new_block = w.tree.getBlockInfo(p)
        self.assertEqual(len(new_block.children_list), 1)
        self.assertEqual(["InitialCondition"], new_block.children_list)
        item = w._path_item_map.get("%s/InitialCondition" % p)
        self.assertNotEqual(item, None)

        mock_exec.return_value = None
        nchilds = len(v.children_list)
        item = w._path_item_map.get(v.path)
        w.scrollToItem(item)
        point = w.visualItemRect(item).center()
        w._treeContextMenu(point)

        self.assertEqual(nchilds, len(v.children_list))
        self.assertEqual(self.changed_count, 3)

        mock_exec.return_value = w.add_action
        w._treeContextMenu(point)

        self.assertEqual(nchilds+1, len(v.children_list))
        self.assertEqual(self.changed_count, 4)

        w.setCurrentItem(item)
        w._newBlockShortcut()
        self.assertEqual(nchilds+2, len(v.children_list))
        self.assertEqual(self.changed_count, 5)

        # /Executioner is not a star so we shouldn't be able to copy it
        b = w.tree.getBlockInfo("/Executioner")
        nchilds = len(b.children_list)
        w.copyBlock(b)
        self.assertEqual(nchilds, len(b.children_list))

        item = w._path_item_map.get(b.path)
        w.scrollToItem(item)
        point = w.visualItemRect(item).center()
        w._treeContextMenu(point)

if __name__ == '__main__':
    Testing.run_tests()
