#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from PyQt5.QtWidgets import QWidget, QSplitter, QMessageBox
from PyQt5.QtCore import Qt, pyqtSignal
from peacock.utils import WidgetUtils
from peacock.base.MooseWidget import MooseWidget
from .CommentEditor import CommentEditor
from .ParamsTable import ParamsTable
from .ParamsByGroup import ParamsByGroup
from .ParamsByType import ParamsByType

class BlockEditor(QWidget, MooseWidget):
    """
    The complete editing widget for a Block.
    The input file will only change when "Apply changes" has been clicked.
    Until then all changes only live in the widgets.
    The exceptions to this are the "Clone" and "Remove" buttons which just sends out signals to let
    others do work.
    Signals:
        needBlockList(list): When the type of a parameter references blocks (for example it is a VariableName),
            this is used to update the available options since they can change based on what the user does.
        blockRenamed(object, str): A block has been renamed. This is emitted so that the BlockTree can update the name of the block.
        blockChanged(object): Apply has been clicked for this block.
        cloneBlock(object): The user wants to clone the block we are currently editing.
        removeBlock(object): The user wants to remove the block we are currently editing.
        editingFinished(): The user is done editing this block. Typically done by closing the window.
        appliedAndClosed(object): Emit the block when the user hits the "Apply & Close" button
    """
    needBlockList = pyqtSignal(list) # list of paths that we need children for
    blockRenamed = pyqtSignal(object, str) # block with changes, old path
    blockChanged = pyqtSignal(object) # block that has changed

    cloneBlock = pyqtSignal(object) # block to clone
    removeBlock = pyqtSignal(object) # block to remove
    editingFinished = pyqtSignal()
    appliedAndClosed = pyqtSignal(object)

    def __init__(self, block, type_to_block_map, **kwds):
        """
        Sets up an editor for a block.
        Input:
            block[BlockInfo]: Block to be edited.
        """
        super(BlockEditor, self).__init__(**kwds)
        self.block = block
        self.comment_edit = CommentEditor()
        self.comment_edit.setComments(self.block.comments)
        self.comment_edit.textChanged.connect(self._blockChanged)
        self.splitter = None
        self.clone_button = None
        self.clone_shortcut = None
        self.remove_button = None
        self.apply_button = None
        self.reset_button = None
        self.new_parameter_button = None
        self.param_editor = None
        self.setWindowTitle(block.path)

        if block.types:
            self.param_editor = ParamsByType(block, type_to_block_map)
        elif block.parameters:
            self.param_editor = ParamsByGroup(block, block.orderedParameters(), type_to_block_map)
        else:
            self.param_editor = ParamsTable(block, block.orderedParameters(), type_to_block_map)

        self.param_editor.needBlockList.connect(self.needBlockList)
        self.param_editor.changed.connect(self._blockChanged)
        self.param_editor.blockRenamed.connect(self.blockRenamed)

        self._createButtons()
        self.applyChanges()
        self._current_commands = []
        self._command_index = 0
        self.user_params = []

        self.splitter = QSplitter(self)
        self.splitter.setOrientation(Qt.Vertical)
        self.splitter.setChildrenCollapsible(False)
        self.splitter.addWidget(self.param_editor)
        self.splitter.addWidget(self.comment_edit)
        self.splitter.setStretchFactor(0,2)
        self.splitter.setStretchFactor(1,1)
        self.top_layout = WidgetUtils.addLayout(vertical=True)
        self.top_layout.addWidget(self.splitter)
        self.top_layout.addLayout(self.button_layout)
        self.setLayout(self.top_layout)

        self.setup()

    def _blockChanged(self, enabled=True):
        """
        Sets the Apply and Reset buttons based on enabled.
        Input:
            enabled[bool]: Whether to set the buttons to enabled
        """
        self.apply_button.setEnabled(enabled)
        self.reset_button.setEnabled(enabled)
        self.setWindowTitle(self.block.path)

    def setWatchedBlockList(self, path, children):
        self.param_editor.setWatchedBlockList(path, children)

    def _createButtons(self):
        """
        Create allowable buttons for this Block.
        This will depend on whether this is a user added block.
        """
        self.button_layout = WidgetUtils.addLayout()

        self.close_button = WidgetUtils.addButton(self.button_layout, self, "Apply && Close", self._applyAndClose)
        self.close_button.setToolTip("Apply any changes and close the window")

        self.apply_button = WidgetUtils.addButton(self.button_layout, self, "Apply", self.applyChanges)
        self.apply_button.setEnabled(False)
        self.apply_button.setToolTip("Apply changes made")

        self.reset_button = WidgetUtils.addButton(self.button_layout, self, "Reset", self.resetChanges)
        self.reset_button.setEnabled(False)
        self.reset_button.setToolTip("Reset changes to when this window was opened")

        self.new_parameter_button = WidgetUtils.addButton(self.button_layout, self, "Add parameter", self.addUserParamPressed)
        self.new_parameter_button.setToolTip("Add a non standard parameter")

        if self.block.user_added:
            self.clone_button = WidgetUtils.addButton(self.button_layout, self, "Clone Block", self._cloneBlock)
            self.clone_shortcut = WidgetUtils.addShortcut(self, "Ctrl+N", self._cloneBlock, shortcut_with_children=True)
            self.clone_button.setToolTip("Clone this block with the same parameters")

            self.remove_button = WidgetUtils.addButton(self.button_layout, self, "Remove Block", self._removeBlock)
            self.remove_button.setToolTip("Remove this block")

    def _findFreeParamName(self, max_params=1000):
        """
        Find a free parameter name that can be safely added.
        Input:
            max_params[int]: Maximum number of tries before giving up.
        """
        base = "NewParam"
        for i in range(max_params):
            param = '%s%s' % (base, i)
            if self.param_editor.paramValue(param) == None:
                return param

    def addUserParamPressed(self):
        """
        The user wants to add a new user parameter to this block.
        """
        new_name = self._findFreeParamName()
        self._blockChanged()
        self.param_editor.addUserParam(new_name)

    def _cloneBlock(self):
        """
        The user wants to clone this block
        """
        self.cloneBlock.emit(self.block)

    def _removeBlock(self):
        """
        The user wants to remove this block.
        We ask to make sure they want to do this.
        """
        button = QMessageBox.question(self, "Confirm remove", "Are you sure you want to delete %s" % self.block.path, QMessageBox.Yes, QMessageBox.No)
        if button == QMessageBox.Yes:
            self.removeBlock.emit(self.block)
            self.hide()
            self.editingFinished.emit()

    def applyChanges(self):
        """
        Apply any changes the user has made.
        """
        self.block.comments = self.comment_edit.getComments()
        self.param_editor.save()
        self.block.changed_by_user = True
        self._blockChanged(enabled=False)
        self.blockChanged.emit(self.block)

    def _applyAndClose(self):
        """
        Apply any changes the user has made then close the window
        """
        if self.apply_button.isEnabled():
            self.applyChanges()
            self.appliedAndClosed.emit(self.block)
        self.close()

    def resetChanges(self):
        """
        Reset any changes the user has made.
        """
        self.comment_edit.setComments(self.block.comments)
        self.param_editor.reset()
        self._blockChanged(enabled=False)

    def updateWatchers(self):
        """
        This should be called after creating a BlockEditor.
        This isn't called in the constructor because the caller
        will typically need to hook up the needBlockList signal first.
        """
        self.param_editor.updateWatchers()

    def closeEvent(self, event):
        """
        The user is done editing.
        """
        self.editingFinished.emit()

if __name__ == "__main__":
    from PyQt5.QtWidgets import QApplication, QMainWindow
    from InputTree import InputTree
    from ExecutableInfo import ExecutableInfo
    import sys
    if len(sys.argv) != 4:
        print("Usage: %s <exe> <input file> <block path>" % sys.argv[0])
        sys.exit(1)
    qapp = QApplication(sys.argv)
    main_win = QMainWindow()
    exe_info = ExecutableInfo()
    exe_info.clearCache()
    exe_info.setPath(sys.argv[1])
    tree = InputTree(exe_info)
    tree.setInputFile(sys.argv[2])
    b = tree.getBlockInfo(sys.argv[3])
    w = BlockEditor(b, tree.app_info.type_to_block_map)
    main_win.setCentralWidget(w)
    main_win.show()
    main_win.resize(640, 480)
    sys.exit(qapp.exec_())
