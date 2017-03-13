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
        self.setSizePolicy(QtWidgets.QSizePolicy.Minimum, QtWidgets.QSizePolicy.Fixed)

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
            state[item.text()] = item.checkState()

        self.ListWidget.clear()
        blocks = reader.getBlockInformation()[self._type]
        for i, block in enumerate(blocks.itervalues()):
            item = QtWidgets.QListWidgetItem(self.ListWidget)
            if  block.name in state:
                item.setCheckState(state[block.name])
            elif self.ListHeader.checkState() == QtCore.Qt.Checked:
                item.setCheckState(QtCore.Qt.Checked)
            else:
                item.setCheckState(QtCore.Qt.Unchecked)
            item.setText(block.name)
            self.ListWidget.insertItem(i, item)

    def getBlocks(self):
        """
        Callback when the list items are changed
        """
        blocks = []
        if self.isEnabled():
            for i in range(self.ListWidget.count()):
                item = self.ListWidget.item(i)
                if item.checkState() == QtCore.Qt.Checked:
                    blocks.append(str(item.text()))
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
        qobject.setMaximumHeight(70)
        qobject.setSizePolicy(QtWidgets.QSizePolicy.Minimum, QtWidgets.QSizePolicy.Preferred)


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

# Update the store/load methods for this class
peacock.utils.WidgetUtils.WIDGET_SETTINGS_CACHE[BlockSelectorWidget] = [('checkState', 'setCheckState')]
