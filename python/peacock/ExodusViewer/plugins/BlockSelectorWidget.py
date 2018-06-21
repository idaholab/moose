#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from PyQt5 import QtCore, QtWidgets
import peacock

class BlockSelectorWidget(peacock.base.MooseWidget, QtWidgets.QWidget):
    """
    A generic widget for controlling visible blocks, nodesets, and sidesets of the current Exodus result.

    Args:
         type[int]: The block type from vtk (see BlockControls.py).
    """

    #: pyqtSignal: Emitted when the "all" checkbox is pressed.
    clicked = QtCore.pyqtSignal()

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

        # Create the label and box
        self.ListHeader = QtWidgets.QCheckBox(self)
        self.MainLayout.addWidget(self.ListHeader)

        self.ListWidget = QtWidgets.QListWidget(self)
        self.MainLayout.addWidget(self.ListWidget)

        # Call setup methods
        self.setup()

    def updateBlocks(self, reader):
        """
        Initialization function, which is called by the main tab widget.

        Args:
            reader[chigger.ExodusReader]: The current reader object.

        """

        # Current state
        state = dict()
        for i in range(self.ListWidget.count()):
            item = self.ListWidget.item(i)
            state[item.data(QtCore.Qt.UserRole)] = item.checkState()

        self.ListWidget.clear()
        blocks = reader.getBlockInformation()[self._type]
        for i, block in enumerate(blocks.itervalues()):
            item = QtWidgets.QListWidgetItem(self.ListWidget)
            item.setData(QtCore.Qt.UserRole, block.name)
            if  block.name in state:
                item.setCheckState(state[block.name])
            elif self.ListHeader.checkState() == QtCore.Qt.Checked:
                item.setCheckState(QtCore.Qt.Checked)
            else:
                item.setCheckState(QtCore.Qt.Unchecked)
            if block.name.isdigit():
                item.setText(block.name)
            else:
                item.setText('{} ({})'.format(block.name, block.number))
            self.ListWidget.insertItem(i, item)

        w = max(100, self.ListWidget.sizeHintForColumn(0) + 2 * self.ListWidget.frameWidth())
        h = self.ListWidget.sizeHintForRow(0) * min(10, self.ListWidget.count()) + 2 * self.ListWidget.frameWidth()
        self.ListWidget.setMaximumWidth(w)
        self.ListWidget.setMaximumHeight(h)

    def getBlocks(self):
        """
        Callback when the list items are changed
        """
        blocks = []
        if self.isEnabled():
            for i in range(self.ListWidget.count()):
                item = self.ListWidget.item(i)
                if item.checkState() == QtCore.Qt.Checked:
                    blocks.append(str(item.data(QtCore.Qt.UserRole)))
        return blocks if blocks else None

    def _setupListHeader(self, qobject):
        """
        Setup method for list title
        """

        if self._title:
            qobject.setText(self._title)
            qobject.font().setPointSize(6)
            qobject.setChecked(self._initial_status)
        qobject.clicked.connect(self._callbackListHeader)

    @QtCore.pyqtSlot(bool)
    def _callbackListHeader(self, value):
        """
        Callback for ListHeader widget.
        """

        if value:
            state = QtCore.Qt.Checked
        else:
            state = QtCore.Qt.Unchecked

        self.ListHeader.setCheckState(state)
        for i in range(self.ListWidget.count()):
            self.ListWidget.item(i).setCheckState(state)

        self.clicked.emit()

    def _setupListWidget(self, qobject):
        """
        Setup method for the list box.
        """
        qobject.itemClicked.connect(self._callbackListWidget)

    @QtCore.pyqtSlot(QtWidgets.QListWidgetItem)
    def _callbackListWidget(self, item):
        """
        Callback for list widget.
        """
        self.ListHeader.setCheckState(QtCore.Qt.Unchecked)
        self.clicked.emit()

    def checkState(self):
        """
        Inspects the items in the list widget.
        """
        state = []
        for i in range(self.ListWidget.count()):
            state.append(self.ListWidget.item(i).checkState())
        return state

    def setCheckState(self, state):
        """
        Sets the items in the list widget.
        """
        if (len(state) != self.ListWidget.count()):
            return

        for i in range(len(state)):
            self.ListWidget.item(i).setCheckState(state[i])
        self.clicked.emit()

    def setAllChecked(self, on=True):
        """
        Set all items to the same check state
        """
        state = QtCore.Qt.Checked
        if not on:
            state = QtCore.Qt.Unchecked
        self.ListHeader.setCheckState(state)
        for i in range(self.ListWidget.count()):
            self.ListWidget.item(i).setCheckState(state)
        self.clicked.emit()

# Update the store/load methods for this class
peacock.utils.WidgetUtils.WIDGET_SETTINGS_CACHE[BlockSelectorWidget] = [('checkState', 'setCheckState')]
