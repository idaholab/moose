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

class MeshBlockSelectorWidget(peacock.base.MooseWidget, QtWidgets.QWidget):
    """
    A generic widget for controlling visible blocks, nodesets, and sidesets of the current mesh.

    Args:
         type[int]: The block type from vtk (see BlockControls.py).
    """

    selectionChanged = QtCore.pyqtSignal()

    def __init__(self, block_type, title, **kwargs):
        super(MeshBlockSelectorWidget, self).__init__(**kwargs)

        self._title = title
        self._type = block_type

        self.MainLayout = QtWidgets.QVBoxLayout()
        self.MainLayout.setContentsMargins(0, 0, 0, 0)
        self.setLayout(self.MainLayout)

        self.Label = QtWidgets.QLabel(self)
        self.MainLayout.addWidget(self.Label)
        self.Label.setText(self._title)

        self.Options = QtWidgets.QComboBox(self)
        self.Options.setFocusPolicy(QtCore.Qt.StrongFocus)
        self.MainLayout.addWidget(self.Options)
        self.Options.currentTextChanged.connect(self.comboTextChanged)
        self._blocks = {}

        self.setup()

    def updateBlocks(self, reader, force=False):
        """
        Initialization function, which is called by the main tab widget.

        Args:
            reader[chigger.ExodusReader]: The current reader object.

        """

        blocks = reader.getBlockInformation()[self._type]
        if blocks != self._blocks or force:
            self.Options.clear()
            self._blocks = blocks
            self.Options.addItem("")
            for block in sorted(blocks.values(), key=lambda b: b.name):
                self.Options.addItem(block.name)
            self.Options.setCurrentIndex(0)

    def getBlocks(self):
        """
        Callback when the list items are changed
        """
        if self.isEnabled() and self.Options.currentText() != "":
            return [self.Options.currentText()]
        else:
            return None

    def reset(self):
        """
        Set the current text to "" without emitting any signals.
        """
        self.Options.blockSignals(True)
        self.Options.setCurrentText("")
        self.Options.blockSignals(False)

    def comboTextChanged(self, text):
        self.selectionChanged.emit()
