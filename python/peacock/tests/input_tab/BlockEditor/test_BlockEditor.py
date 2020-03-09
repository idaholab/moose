#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from peacock.Input.BlockEditor import BlockEditor
from PyQt5.QtWidgets import QMessageBox, QApplication
from peacock.Input.ExecutableInfo import ExecutableInfo
from peacock.Input.InputTree import InputTree
from peacock.Input.ParamsTable import ParamsTable
from peacock.Input.ParamsByGroup import ParamsByGroup
from peacock.Input.ParamsByType import ParamsByType
from peacock.Input.BlockInfo import BlockInfo
from mock import patch
from peacock.utils import Testing


class Tests(Testing.PeacockTester):
    qapp = QApplication([])

    def setUp(self):
        super(Tests, self).setUp()
        self.test_input_file = "../../common/fsp_test.i"
        self.app_info = ExecutableInfo()
        self.app_info.setPath(Testing.find_moose_test_exe())
        self.tree = InputTree(self.app_info)
        self.tree.setInputFile(self.test_input_file)
        self.assertTrue(self.app_info.valid())
        self.block_list_requested = 0
        self.block_removed_count = 0
        self.last_block_removed = 0
        self.block_cloned_count = 0
        self.last_block_cloned = 0
        self.editing_finished = False
        self.block_changed_count = 0
        self.last_block_changed = 0

    def newWidget(self, path):
        b = self.tree.getBlockInfo(path)
        e = BlockEditor(b, self.tree.app_info.type_to_block_map)
        e.needBlockList.connect(lambda paths: self.needBlockList(e, paths))
        e.removeBlock.connect(self.removeBlock)
        e.cloneBlock.connect(self.cloneBlock)
        e.blockChanged.connect(self.blockChanged)
        e.editingFinished.connect(self.editingFinished)
        self.editing_finished = False
        return e

    def blockChanged(self, block):
        self.block_changed_count += 1
        self.last_block_changed = block

    def editingFinished(self):
        self.editing_finished = True

    def removeBlock(self, block):
        self.block_removed_count += 1
        self.last_block_removed = block

    def cloneBlock(self, block):
        self.block_cloned_count += 1
        self.last_block_cloned = block

    def needBlockList(self, e, blocks):
        self.block_list_requested += 1
        for b in blocks:
            e.setWatchedBlockList(b, ["child0", "child1", "child2"])

    def newCounter(self, node):
        self.new_node = node
        self.new_node_counter += 1

    def checkParamEditor(self, e, has_params, has_types):
        if has_types:
            self.assertTrue(isinstance(e.param_editor, ParamsByType))
        elif has_params:
            self.assertTrue(isinstance(e.param_editor, ParamsByGroup))
        else:
            self.assertTrue(isinstance(e.param_editor, ParamsTable))

    def checkButton(self, button, exists, enabled):
        if button is None:
            self.assertTrue(button is None and not exists)
        else:
            self.assertTrue(button is not None and exists)
        if button:
            self.assertEqual(button.isEnabled(), enabled)

    def checkWidget(self,
            e,
            apply_button=True,
            apply_enabled=False,
            user_block=False,
            reset_button=True,
            reset_enabled=False,
            new_param_button=True,
            new_param_enabled=True,
            has_params=False,
            has_types=False,
            comments=""
            ):
        self.checkParamEditor(e, has_params, has_types)
        self.checkButton(e.clone_button, user_block, True)
        self.checkButton(e.apply_button, apply_button, apply_enabled)
        self.checkButton(e.remove_button, user_block, True)
        self.checkButton(e.reset_button, reset_button, reset_enabled)
        self.checkButton(e.new_parameter_button, new_param_button, new_param_enabled)
        self.assertEqual(e.comment_edit.getComments(), comments)

    def testBlockComments(self):
        b = BlockInfo(None, "/Foo", True, "")
        c = "some comments"
        e = BlockEditor(b, self.tree.app_info.type_to_block_map)
        self.checkWidget(e)
        self.assertEqual(b.comments, "")
        e.comment_edit.setComments(c)
        self.checkWidget(e, apply_enabled=True, reset_enabled=True, comments=c)
        # This should get updated after apply is pressed
        self.assertEqual(b.comments, "")
        e.applyChanges()
        self.assertEqual(b.comments, c)
        self.checkWidget(e, comments=c)

        e.resetChanges()
        self.checkWidget(e, comments=c)

        e.comment_edit.setComments("")
        self.checkWidget(e, apply_enabled=True, reset_enabled=True, comments="")
        self.assertEqual(b.comments, c)
        e.resetChanges()
        self.checkWidget(e, comments=c)
        self.assertEqual(b.comments, c)


    def testUserParams(self):
        b = BlockInfo(None, "/Foo", True, "")
        e = BlockEditor(b, self.tree.app_info.type_to_block_map)
        self.checkWidget(e)
        self.assertEqual(len(b.parameters_list), 0)
        e.addUserParamPressed()
        self.checkWidget(e, apply_enabled=True, reset_enabled=True)
        self.assertEqual(len(b.parameters_list), 0)

        e.applyChanges()
        self.checkWidget(e)
        self.assertEqual(len(b.parameters_list), 1)

        e.addUserParamPressed()
        self.checkWidget(e, apply_enabled=True, reset_enabled=True)
        self.assertEqual(len(b.parameters_list), 1)

        e.resetChanges()
        self.checkWidget(e)
        self.assertEqual(len(b.parameters_list), 1)

    def testClone(self):
        e = self.newWidget("/Kernels/diff_u")
        self.checkWidget(e, has_types=True, user_block=True)
        e._cloneBlock()
        self.assertEqual(self.block_cloned_count, 1)
        self.assertEqual(self.last_block_cloned, e.block)

    @patch.object(QMessageBox, "question")
    def testRemoveBlock(self, mock_box):
        mock_box.return_value = QMessageBox.No
        e = self.newWidget("/Kernels/diff_u")
        self.checkWidget(e, has_types=True, user_block=True)
        e._removeBlock()
        self.assertEqual(self.block_removed_count, 0)

        mock_box.return_value = QMessageBox.Yes
        self.assertEqual(self.editing_finished, False)
        e._removeBlock()
        self.assertEqual(self.block_removed_count, 1)
        self.assertEqual(self.last_block_removed, e.block)
        self.assertEqual(self.editing_finished, True)

    def checkAddParam(self, path, has_types, has_params, user=False):
        e = self.newWidget(path)
        b = e.block
        num_params = len(b.parameters_list)
        self.checkWidget(e, has_types=has_types, has_params=has_params, user_block=user)
        e.addUserParamPressed()
        self.checkWidget(e, apply_enabled=True, reset_enabled=True, has_types=has_types, has_params=has_params, user_block=user)
        self.assertEqual(len(b.parameters_list), num_params)

        e.applyChanges()
        self.checkWidget(e, has_types=has_types, has_params=has_params, user_block=user)
        self.assertEqual(len(b.parameters_list), num_params+1)
        p = b.getParamInfo(b.parameters_list[-1])
        self.assertNotEqual(p, None)
        self.assertEqual(p.value, "")

        e.addUserParamPressed()
        self.checkWidget(e, apply_enabled=True, reset_enabled=True, has_types=has_types, has_params=has_params, user_block=user)
        self.assertEqual(len(b.parameters_list), num_params+1)

        e.resetChanges()
        self.checkWidget(e, has_types=has_types, has_params=has_params, user_block=user)
        self.assertEqual(len(b.parameters_list), num_params+1)

        self.assertEqual(self.editing_finished, False)
        e.close()
        self.assertEqual(self.editing_finished, True)

    def testAddParams(self):
        # has no types, is not a star, and no default params
        self.checkAddParam("/GlobalParams", False, False)
        # Has types
        self.checkAddParam("/Mesh", True, True)
        # Is star node, had an 'active' parameter but we get rid of it
        self.checkAddParam("/Kernels", False, False)
        # Is user block
        self.checkAddParam("/Kernels/diff_u", True, True, True)

if __name__ == '__main__':
    Testing.run_tests()
