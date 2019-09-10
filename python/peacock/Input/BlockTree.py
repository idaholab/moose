#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from PyQt5.QtWidgets import QTreeWidget, QTreeWidgetItem, QAbstractItemView, QAction, QMenu, QSizePolicy, QMessageBox
from PyQt5.QtGui import QBrush, QColor
from PyQt5.QtCore import Qt, pyqtSignal
from peacock.utils import WidgetUtils
from peacock.base.MooseWidget import MooseWidget
try:
    from cStringIO import StringIO
except ImportError:
    from io import StringIO

class BlockTree(QTreeWidget, MooseWidget):
    """
    This shows all the hard paths that are available.
    Allows for including blocks in the input file, moving blocks
    to reorder their position in the input file, adding and removing blocks.
    Signals:
        changed[BlockInfo]: Emitted when a block has changed
        blockSelected[BlockInfo]: Emitted when a block is selected
        blockClicked[BlockInfo]: Emitted when a block is clicked
        blockDoubleClicked[BlockInfo]: Emitted when a block is double clicked
    """
    changed = pyqtSignal(object)
    blockSelected = pyqtSignal(object)
    blockClicked = pyqtSignal(object)
    blockDoubleClicked = pyqtSignal(object)

    def __init__(self, tree, **kwds):
        """
        Input:
            tree[InputTree]: The input tree to read blocks from.
        """
        super(BlockTree, self).__init__(**kwds)

        self.tree = tree
        self.root_item = self.invisibleRootItem()
        self.header().close()
        self.setFocusPolicy(Qt.WheelFocus)
        self._item_block_map = {}
        self._path_item_map = {}

        self.setAcceptDrops(True)
        self.setDropIndicatorShown(True)
        self.setDragDropMode(QAbstractItemView.InternalMove)
        self._mime_type = "application/x-qabstractitemmodeldatalist"
        self._current_drag = None

        self.setContextMenuPolicy(Qt.CustomContextMenu)
        self.customContextMenuRequested.connect(self._treeContextMenu)
        self.itemClicked.connect(self.onItemClicked)
        self.itemDoubleClicked.connect(self.onItemDoubleClicked)
        self.itemChanged.connect(self.onItemChanged)
        self.setSizePolicy(QSizePolicy.MinimumExpanding, QSizePolicy.MinimumExpanding)
        self.setExpandsOnDoubleClick(False)
        self.setItemsExpandable(True)
        self.setMouseTracking(False)
        self.setSelectionMode(QAbstractItemView.SingleSelection)

        self.clone_shortcut = WidgetUtils.addShortcut(self, "Ctrl+N", self._newBlockShortcut, shortcut_with_children=True)
        self.populateFromTree()
        self.setup()
        self.add_action = QAction("Add", None)
        self.remove_action = QAction("Remove", None)

    def populateFromTree(self):
        """
        Populate the items from the InputTree
        """
        self.clear()
        self._item_block_map = {self.root_item.__str__(): "/"}
        self._path_item_map = {"/": self.root_item}

        self.blockSignals(True)
        for p in sorted(self.tree.path_map.keys()):
            b = self.tree.path_map[p]
            self.addBlock(b)
        self.blockSignals(False)

    def setInputTree(self, tree):
        """
        Set the InputTree to populate items from.
        Input:
            tree[InputTree]: The new InputTree
        """
        self.tree = tree
        self.populateFromTree()

    def _newBlockShortcut(self):
        """
        Called on the keyboard shortcut to copy a block
        """
        item = self.currentItem()
        if item:
            block = self._item_block_map.get(item.__str__())
            if block:
                self.copyBlock(block)

    def onItemClicked(self, item, col):
        """
        A block has been clicked.
        Input:
            item[QTreeWidgetItem]: The item that was clicked
            col[int]: The column that was clicked.
        """
        block = self._item_block_map.get(item.__str__())
        if block:
            self.blockClicked.emit(block)

    def onItemDoubleClicked(self, item, col):
        """
        A block has been double clicked.
        Input:
            item[QTreeWidgetItem]: The item that was double clicked
            col[int]: The column that was double clicked.
        """
        block = self._item_block_map.get(item.__str__())
        if block:
            self.blockDoubleClicked.emit(block)

    def onItemChanged(self, item, col):
        """
        A block has changed. This is usually the check state that changed.
        Input:
            item[QTreeWidgetItem]: The item that was changed.
            col[int]: The column that was changed.
        """
        block = self._item_block_map.get(item.__str__())
        if block:
            block.included = item.checkState(0) == Qt.Checked
            self.changed.emit(block)

    def dragEnterEvent(self, event):
        """
        Start a drag event.
        Input:
            event[QDragEnterEvent]: The event
        """
        data = event.mimeData()
        items = self.selectedItems()
        if not items or not data.hasFormat(self._mime_type):
            return
        block = self._item_block_map.get(self._getItemParent(items[0]))
        if block and block.parent and block.path != "/":
            self._current_drag = items[0]
            event.acceptProposedAction()

    def indexOfItem(self, item):
        """
        Gets the index of the item in the child list
        Input:
            item[QTreeWidgetItem]: The item to get the index for
        Return:
            int: The current index
        """
        return self._getItemParent(item).indexOfChild(item)

    def dropEvent(self, event):
        """
        A drop is being made.
        Input:
            event[QDropEvent]: The event
        """
        item = self.itemAt(event.pos())
        data = event.mimeData()
        if self._current_drag and item and data.hasFormat(self._mime_type):
            current_block = self._item_block_map.get(self._current_drag)
            to_block = self._item_block_map.get(item.__str__())
            if current_block and to_block and current_block.parent == to_block.parent and to_block.parent.path != "/":
                idx = self.indexOfItem(self._current_drag)
                super(BlockTree, self).dropEvent(event)
                idx = self.indexOfItem(self._current_drag) # The parent class should have moved it
                self.tree.moveBlock(current_block.path, idx)
        self._current_drag = None

    def renameBlock(self, block, newname):
        """
        Rename a block. This updates the QTreeWidgetItem
        as well as the InputTree.
        Input:
            block[BlockInfo]: The block to rename.
            newname[str]: New name of the block
        """
        item = self._path_item_map.get(block.path)
        if item:
            del self._path_item_map[block.path]
            self.tree.renameUserBlock(block.parent.path, block.name, newname)
            self._path_item_map[block.path] = item
            block = self._item_block_map[item.__str__()]
            item.setText(0, block.name)

    def removeBlock(self, block):
        """
        Removes a block. This removes the QTreeWidgetItem
        and removes it from the InputTree.
        Input:
            block[BlockInfo]: The block to rename.
            newname[str]: New name of the block
        """
        item = self._path_item_map.get(block.path)
        if item:
            self.tree.removeBlock(block.path)
            del self._path_item_map[block.path]
            del self._item_block_map[item.__str__()]
            self._getItemParent(item).removeChild(item)

    def _getItemParent(self, item):
        """
        Get the parent of an item.
        For top level items it seems item.parent() returns None
        when it really should be the root item.
        Input:
            item[QTreeWidgetItem]: item to get the parent of
        Return:
            QTreeWidgetItem: Parent of the passed in item.
        """
        parent = item.parent()
        if not parent:
            return self.root_item
        return parent

    def addBlock(self, block, select=False):
        """
        Add a new block to the tree
        Input:
            block[BlockInfo]: Block to add
        """
        item = self._path_item_map.get(block.path)
        if not item:
            pitem = self._path_item_map.get(block.parent.path)
            if pitem:
                new_item = self._newItem(pitem, block)
                if select:
                    self.setCurrentItem(new_item)

    def _newItem(self, parent_item, block):
        """
        Creates a new QTreeWidgetItem and adds it.
        Input:
            parent_item[QTreeWidgetItem]: Parent item to add the new item to
            block[BlockInfo]: block to add
        """
        new_child = QTreeWidgetItem()
        new_child.setText(0, block.name)
        new_child.setToolTip(0, block.toolTip())
        if parent_item == self.root_item:
            new_child.setFlags(Qt.ItemIsEnabled | Qt.ItemIsSelectable | Qt.ItemIsUserCheckable )
            self.addTopLevelItem(new_child)
        else:
            parent_item.addChild(new_child)
            new_child.setFlags(Qt.ItemIsEnabled | Qt.ItemIsSelectable | Qt.ItemIsDragEnabled |Qt.ItemIsUserCheckable )
        state = Qt.Unchecked
        if block.included:
            state = Qt.Checked
        new_child.setCheckState(0, state)
        self._path_item_map[block.path] = new_child
        self._item_block_map[new_child.__str__()] = block
        if block.star:
            new_child.setForeground(0, QBrush(QColor("blue")))

        if parent_item != self.root_item:
            default_flags = parent_item.flags()
            parent_item.setFlags(default_flags | Qt.ItemIsDropEnabled )
        for child_name in block.children_list:
            child = block.children[child_name]
            self._newItem(new_child, child)
        return new_child

    def copyBlock(self, block):
        """
        Copys a block. This creates a new QTreeWidgetItem as well as
        adds a new block to InputTree.
        Input:
            block[BlockInfo]: Block to copy from.
        """
        item = self._path_item_map.get(block.path)
        if block.star:
            new_name = block.findFreeChildName()
            new_block = self.tree.addUserBlock(block.path, new_name)
        elif block.user_added:
            parent = self._getItemParent(item)
            parent_block = self._item_block_map.get(parent.__str__())
            new_name = parent_block.findFreeChildName()
            new_block = self.tree.cloneUserBlock(block.path, new_name)
        else:
            return

        self.blockSignals(True)
        self.expandItem(item)
        new_block.included = True
        self.addBlock(new_block, True)
        self.blockSignals(False)

        self.changed.emit(new_block)
        self._includeParents(new_block)
        self.blockDoubleClicked.emit(new_block) # start editing right away

    def _includeParents(self, block):
        """
        Recursively set all parent blocks to be included
        Input:
            block[BlockInfo]: Include the parent of this block
        """
        item = self._path_item_map.get(block.path)
        if item:
            parent = item.parent()
            if parent:
                parent_block = self._item_block_map.get(parent.__str__())
                if parent.checkState(0) == Qt.Unchecked:
                    parent.setCheckState(0, Qt.Checked)
                    parent_block.included = True
                    self.changed.emit(parent_block)
                self._includeParents(parent_block)

    def includeBlock(self, block, include=True):
        item = self._path_item_map.get(block.path)
        if item:
            changed = block.included != include
            block.included = include
            if include:
                if item.checkState(0) == Qt.Unchecked:
                    item.setCheckState(0, Qt.Checked)
                    changed = True
                self._includeParents(block)
            elif item.checkState(0) == Qt.Checked:
                item.setCheckState(0, Qt.Unchecked)
                changed = True
            if changed:
                self.changed.emit(block)

    def _treeContextMenu(self, point):
        """
        Context menu on the tree. This allows for quick access to adding and cloning nodes.
        Input:
            point[QPoint]: Point where the context menu was requested
        """
        item = self.itemAt(point)
        if not item:
            return
        block = self._item_block_map.get(item.__str__())
        if not block:
            return

        if not block.star and not block.user_added:
            return

        menu = QMenu()
        menu.addAction(self.add_action)
        if block.star:
            self.add_action.setText("Add")
        else:
            self.add_action.setText("Clone")
            menu.addAction(self.remove_action)

        result = menu.exec_(self.mapToGlobal(point))
        if result == self.add_action:
            self.copyBlock(block)
        elif result == self.remove_action:
            text = "Are you sure you want to delete %s" % block.path
            button = QMessageBox.question(self, "Confirm remove", text, QMessageBox.Yes, QMessageBox.No)
            if button == QMessageBox.Yes:
                self.removeBlock(block)

    def _dumpItem(self, output, item, level=0, sep='  '):
        """
        Dumps an item to a string.
        Input:
            output[StringIO]: Where to write to
            item[QTreeWidgetItem]: item to display
            level[int]: indent level
            sep[str]: indent string
        """
        b = self._item_block_map.get(item.__str__())
        output.write("%s%s: %s: %s\n" % (sep*level, item.text(0), b.star, item.checkState(0) == Qt.Checked))
        child_count = item.childCount()
        for i in range(child_count):
            child = item.child(i)
            self._dumpItem(output, child, level+1, sep)

    def dumpTreeToString(self):
        """
        Dump the tree to a string.
        Return:
            str: A display of the current QTreeWidget
        """
        output = StringIO()
        for i in range(self.root_item.childCount()):
            child = self.root_item.child(i)
            self._dumpItem(output, child)
        return output.getvalue()

if __name__ == "__main__":
    from PyQt5.QtWidgets import QApplication, QMainWindow
    from InputTree import InputTree
    from ExecutableInfo import ExecutableInfo
    import sys
    if len(sys.argv) != 3:
        print("Usage: %s <exe> <input file>" % sys.argv[0])
        sys.exit(1)

    qapp = QApplication(sys.argv)
    main_win = QMainWindow()
    exe_info = ExecutableInfo()
    exe_info.setPath(sys.argv[1])
    tree = InputTree(exe_info)
    tree.setInputFile(sys.argv[2])
    w = BlockTree(tree)
    main_win.setCentralWidget(w)
    main_win.show()
    sys.exit(qapp.exec_())
