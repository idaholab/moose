#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import os
from PyQt5.QtWidgets import QWidget, QSizePolicy
from PyQt5.QtCore import Qt, pyqtSignal
from peacock.utils import WidgetUtils
from peacock.base.MooseWidget import MooseWidget
import mooseutils
from .BlockTree import BlockTree
from .BlockEditor import BlockEditor
from .InputTree import InputTree
from .ExecutableInfo import ExecutableInfo

class InputFileEditor(QWidget, MooseWidget):
    """
    Holds the widget to edit the input file.
    Contains the tree as well as the parameter editing portion.
    Signals:
        blockChanged[BlockInfo, InputTree]: Emitted when a block is changed
        inputFileChanged[str]: Emitted when the input file has changed
        blockSelected[BlockInfo, InputTree]: Emitted when a block is selected
    """
    blockChanged = pyqtSignal(object, object)
    inputFileChanged = pyqtSignal(str)
    blockSelected = pyqtSignal(object, object)

    def __init__(self, **kwds):
        super(InputFileEditor, self).__init__(**kwds)
        self.tree = InputTree(ExecutableInfo())
        self.top_layout = WidgetUtils.addLayout(vertical=True)
        self.setLayout(self.top_layout)
        self.block_tree = BlockTree(self.tree)
        self.top_layout.addWidget(self.block_tree)
        self.block_tree.blockClicked.connect(self._blockClicked)
        self.block_tree.blockDoubleClicked.connect(self._blockEditorRequested)
        self.block_tree.changed.connect(lambda block: self.blockChanged.emit(block, self.tree))
        self.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Expanding)

        self.block_editor = None

        self.setup()

    def _blockEditorRequested(self, block):
        """
        Called when a block editor is requested.
        Only one editor can be opened.
        Input:
            block[BlockInfo]: The block to edit.
        """
        if self.block_editor:
            # raise it to front in case it is behind windows
            self.block_editor.raise_()
            return

        self.block_tree.blockSignals(True)
        self.block_editor = BlockEditor(block, self.tree.app_info.type_to_block_map, parent=self)
        self.block_editor.needBlockList.connect(self.onNeedBlockList)
        self.block_editor.blockChanged.connect(lambda block: self.blockChanged.emit(block, self.tree))
        self.block_editor.blockRenamed.connect(self.block_tree.renameBlock)
        self.block_editor.cloneBlock.connect(self.block_tree.copyBlock)
        self.block_editor.removeBlock.connect(self.block_tree.removeBlock)
        self.block_editor.editingFinished.connect(self.onEditingFinished)
        self.block_editor.appliedAndClosed.connect(self.onEditorAppliedAndClosed)
        self.block_editor.setWindowFlags(Qt.Window)
        self.block_editor.resize(640, 480)
        self.block_tree.blockSignals(False)
        self.block_editor.updateWatchers()
        self.block_editor.show()
        self.block_editor.raise_()

    def onEditorAppliedAndClosed(self, block):
        if block:
            self.block_tree.includeBlock(block, True)

    def onEditingFinished(self):
        """
        Editing the block is finished.
        """
        self.block_editor.setParent(None) # Don't know if this is required
        self.block_editor = None

    def _blockClicked(self, block):
        """
        Called when an item in the tree is clicked.
        Input:
            block[BlockInfo]: Block that was clicked
        """
        self.blockSelected.emit(block, self.tree)
        if self.block_editor:
            # raise it to front in case it is behind windows
            self.block_editor.raise_()

    def onNeedBlockList(self, paths):
        """
        Get a list of children for the requested paths
        then call setWatchedBlockList on the current editor
        Input:
            paths[list]: A list of paths to get the children for
        """
        if not self.block_editor:
            return

        for p in paths:
            b = self.tree.getBlockInfo(p)
            if b:
                children = []
                for c in b.children_list:
                    info = b.children[c]
                    if info.user_added:
                        children.append(c)
                self.block_editor.setWatchedBlockList(p, children)

    def executableInfoChanged(self, app_info):
        """
        The ExecutableInfo object has changed.
        Input:
            app_info[ExecutableInfo]: The new information
        """
        if app_info.valid():
            self._closeBlockEditor()
            old_tree = self.tree
            self.tree = InputTree(app_info)
            if old_tree.root:
                self.tree.setInputFileData(old_tree.getInputFileString(), old_tree.input_filename)
            else:
                self.tree.setInputFile(old_tree.input_filename)
            self.block_tree.setInputTree(self.tree)

    def setInputFile(self, input_file):
        """
        The input file has changed.
        Input:
            input_file[str]: The new input file
        """
        self._closeBlockEditor()

        if self.tree.app_info.valid():
            input_file = os.path.abspath(input_file)
            if self.tree.setInputFile(input_file):
                self.block_tree.setInputTree(self.tree)
                self.inputFileChanged.emit(input_file)
                return True
            elif input_file:
                mooseutils.mooseError("Failed to read input file", dialog=True)
        else:
            self.tree.input_filename = input_file
        return False

    def _closeBlockEditor(self):
        """
        Just closes the block editor if it is open
        """
        if self.block_editor:
            self.block_editor.close()
            self.block_editor = None

    def closing(self):
        """
        Called when the parent is about to close.
        """
        self._closeBlockEditor()

    def writeInputFile(self, filename):
        """
        Write the input tree to a file.
        Input:
            filename: Where to write the file.
        """
        if not self.tree.app_info.valid() or not filename:
            return
        content = self.tree.getInputFileString()
        try:
            with open(filename, "w") as f:
                f.write(content)
        except IOError as e:
            mooseutils.mooseWarning("Failed to write input file %s: %s" % (filename, e))

if __name__ == "__main__":
    from PyQt5.QtWidgets import QApplication, QMainWindow
    import sys
    if len(sys.argv) != 3:
        print("Usage: %s <exe> <input file>" % sys.argv[0])
        sys.exit(1)
    qapp = QApplication(sys.argv)
    main_win = QMainWindow()
    w = InputFileEditor()
    main_win.setCentralWidget(w)
    test_file = sys.argv[2]
    exe_info = ExecutableInfo()
    #exe_info.clearCache()
    exe_info.setPath(sys.argv[1])
    w.executableInfoChanged(exe_info)
    w.setInputFile(test_file)
    main_win.show()
    sys.exit(qapp.exec_())
