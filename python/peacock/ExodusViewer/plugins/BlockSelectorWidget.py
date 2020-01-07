#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from PyQt5 import QtCore, QtWidgets, QtGui
import peacock


class BlockQStyledItemDelegate(QtWidgets.QStyledItemDelegate):
    def paint(self, painter, option, index):
        self.showDecorationSelected = False;
        super(BlockQStyledItemDelegate, self).paint(painter, option, index)

class BlockSelectorWidget(peacock.base.MooseWidget, QtWidgets.QWidget):
    """
    A generic widget for controlling visible blocks, nodesets, and sidesets of the current
    Exodus result.

    Args:
         type[int]: The block type from vtk (see BlockControls.py).
    """

    #: pyqtSignal: Emitted when items changed
    itemsChanged = QtCore.pyqtSignal()

    def __init__(self, block_type, **kwargs):
        self._initial_status = kwargs.pop('enabled', True) # when true, all boxes are checked
        self._title = kwargs.pop('title', None)
        super(BlockSelectorWidget, self).__init__(**kwargs)

        # Setup this widget
        self.setContentsMargins(0, 0, 0, 0)
        self.setSizePolicy(QtWidgets.QSizePolicy.Maximum, QtWidgets.QSizePolicy.Maximum)

        # Member variables
        self._type = block_type

        # Layout for title and list box
        self.MainLayout = QtWidgets.QVBoxLayout()
        self.MainLayout.setContentsMargins(0, 0, 0, 0)
        self.setLayout(self.MainLayout)

        self.StandardItemModel = None#QtGui.QStandardItemModel()
        self.ListWidget = QtWidgets.QComboBox()
        self.MainLayout.addWidget(self.ListWidget)

        # Call setup methods
        self.setup()

        # This widget handles store/load manually, see BlockPlugin for use.
        self._state = dict()

    def updateBlocks(self, reader):
        """
        Initialization function, which is called by the main tab widget.

        Args:
            reader[chigger.ExodusReader]: The current reader object.
        """
        # Clear the current model
        self.StandardItemModel = QtGui.QStandardItemModel()
        self.StandardItemModel.itemChanged.connect(self._callbackItemChanged)
        self.StandardItemModel.blockSignals(True)

        # Create the title item
        title = QtGui.QStandardItem(self._title)
        title.setFlags(QtCore.Qt.ItemIsUserCheckable|QtCore.Qt.ItemIsEnabled)
        title.setData(QtCore.Qt.Unchecked, QtCore.Qt.CheckStateRole)
        title.setBackground(QtGui.QBrush(QtGui.QColor(200, 200, 200)))
        if self._initial_status:
            title.setCheckState(QtCore.Qt.Checked)
        self.StandardItemModel.setItem(0, 0, title)

        # Populate the items
        blocks = reader.getBlockInformation()[self._type]
        for i, block in enumerate(blocks.values()):
            item = QtGui.QStandardItem()
            item.setFlags(QtCore.Qt.ItemIsUserCheckable|QtCore.Qt.ItemIsEnabled)
            item.setData(QtCore.Qt.Unchecked, QtCore.Qt.CheckStateRole)
            item.setData(block.name, QtCore.Qt.UserRole)
            if self._initial_status:
                item.setCheckState(QtCore.Qt.Checked)

            if block.name.isdigit():
                item.setText(block.name)
            else:
                item.setText('{} ({})'.format(block.name, block.number))

            self.StandardItemModel.setItem(i+1, item)

        self.ListWidget.setModel(self.StandardItemModel)
        self.ListWidget.setItemDelegate(BlockQStyledItemDelegate())
        self.StandardItemModel.blockSignals(False)

    def getBlocks(self):
        """
        Callback when the list items are changed
        """

        blocks = []
        for i in range(1, self.StandardItemModel.rowCount()):
            item = self.StandardItemModel.item(i)
            if item.checkState() == QtCore.Qt.Checked:
                blocks.append(str(item.data(QtCore.Qt.UserRole)))

        return blocks if blocks else None

    def _callbackItemChanged(self, item):
        """
        Callback for list widget.
        """
        self.StandardItemModel.blockSignals(True)
        if item == self.StandardItemModel.item(0):
            state = item.checkState()
            for i in range(1, self.StandardItemModel.rowCount()):
                self.StandardItemModel.item(i).setCheckState(state)
        else:
            self.StandardItemModel.item(0).setCheckState(QtCore.Qt.Unchecked)
        self.StandardItemModel.blockSignals(False)
        self.itemsChanged.emit()

    def hasState(self, key):
        return key in self._state

    def store(self, key):
        self._state[key] = [self.StandardItemModel.item(i).checkState() for i in range(self.StandardItemModel.rowCount())]

    def load(self, key):
        state = self._state.get(key, None)
        if state:
            for i, checked in enumerate(state):
                item = self.StandardItemModel.item(i)
                if item is not None:
                    item.setCheckState(checked)
