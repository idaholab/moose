#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import os
import sys
import glob
from PyQt5 import QtCore, QtWidgets
import peacock
from .ExodusPlugin import ExodusPlugin
from peacock.ExodusViewer.plugins.ExodusFilterProxyModel import ExodusFilterProxyModel

class ExodusComboBox(QtWidgets.QComboBox):
    """
    A ComboBox that disables filenames if they do not exist using glob.

    NOTE: glob is used so that this will work with MultiApps when that is added.
    """

    def showPopup(self):
        """
        Override to enable/disable files based on existence.
        """
        for i in range(self.count()):
            self.model().item(i).setEnabled(bool(glob.glob(self.itemData(i))))
        super(ExodusComboBox, self).showPopup()

    def hasItem(self, full_file):
        """
        Return true if the file already exists.
        """
        return full_file in [self.itemData(i) for i in range(self.count())]

    def itemIndex(self, full_file):
        """
        Return the index given the full filename.
        """
        return [self.itemData(i) for i in range(self.count())].index(full_file)

class FilePlugin(QtWidgets.QGroupBox, ExodusPlugin):

    """
    The plugin provides an interface for selecting the file, variable, and component to render.

    This plugin provides a signal (stateChanged) when any of the three items change, this allows
    other ExodusPlugin objects to load and store state of widgets based on what is being plotted.
    """

    #: pyqtSignal: Emitted when the window needs to be updated
    windowRequiresUpdate = QtCore.pyqtSignal()

    #: pyqtSignal: Emitted when the filename is changed
    setFilename = QtCore.pyqtSignal(str)

    #: pyqtSignal: Emitted when the variable is changed
    setVariable = QtCore.pyqtSignal(str)

    #: pyqtSignal: Emitted when the variable component is changed
    setComponent = QtCore.pyqtSignal(int)

    def __init__(self, parent=None):
        super(FilePlugin, self).__init__()
        self.MainLayout = QtWidgets.QVBoxLayout(self)

        self.OpenFiles = QtWidgets.QPushButton()
        self.FileList = ExodusComboBox()
        self.VariableList = QtWidgets.QComboBox()
        self.ComponentList = QtWidgets.QComboBox()

        # The open button should always be available
        self.setEnabled(True)
        self.FileList.setEnabled(False)
        self.VariableList.setEnabled(False)
        self.ComponentList.setEnabled(False)

        # Top
        self.TopLayout = QtWidgets.QHBoxLayout()
        self.TopLayout.addWidget(self.OpenFiles)
        self.TopLayout.addWidget(self.FileList)
        self.MainLayout.addLayout(self.TopLayout)

        # Bottom
        self.BottomLayout = QtWidgets.QHBoxLayout()
        self.BottomLayout.addWidget(self.VariableList)
        self.BottomLayout.addWidget(self.ComponentList)
        self.MainLayout.addLayout(self.BottomLayout)

        self.MainLayout.setSpacing(0)
        self.TopLayout.setSpacing(10)
        self.BottomLayout.setSpacing(10)

        self.setup()

    def getFilenames(self):
        """
        Return the list of available files. (see ExodusViewer)
        """
        return [self.FileList.itemData(i) for i in range(self.FileList.count())]

    def onSetFilenames(self, filenames):
        """
        Updates the list of available files for selection.

        This is the entry point for loading a file via the FilePlugin.

        Args:
            filenames[list]: The filenames to include in the FileList widget.
        """

        # Clear the existing list of files
        self.FileList.clear()

        # Populate the list of files
        for full_file in filenames:
            self.FileList.addItem(os.path.basename(full_file), full_file)

        # Reset the current index to the first item
        self.FileList.blockSignals(True)
        self.FileList.setCurrentIndex(0)
        self.FileList.blockSignals(False)

    def onSetupResult(self, result):
        """
        Initialize the list of available variables from the reader.
        """
        reader = result[0].getExodusReader()
        self._initVariableList(reader)

    def onUpdateWindow(self, window, reader, result):
        """
        Check that the variable names have not changed, update if they have.
        """
        variables = reader.getVariableInformation(var_types=[reader.NODAL, reader.ELEMENTAL])
        names = [var.name for var in variables.itervalues()]
        current = [self.VariableList.itemText(i) for i in range(self.VariableList.count())]
        if names != current:
            self._initVariableList(reader)

    def _initVariableList(self, reader):
        """
        Initialize the variable list from the supplied reader.
        """
        variables = reader.getVariableInformation(var_types=[reader.NODAL, reader.ELEMENTAL])
        self.VariableList.blockSignals(True)
        self.VariableList.clear()
        for vinfo in variables.itervalues():
            self.VariableList.addItem(vinfo.name, vinfo)

        # Set the current variable from existing state or the first item
        key = (self._filename, None, None)
        if self.hasState(self.VariableList, key=key):
            self.load(self.VariableList, key=key)
        else:
            self.VariableList.setCurrentIndex(0)
        self.VariableList.blockSignals(False)
        self._variable = str(self.VariableList.currentText())

        # Set the current component from existing state or the first item
        self.ComponentList.blockSignals(True)
        key = (self._filename, self._variable, None)
        if self.hasState(self.ComponentList, key=key):
            self.load(self.ComponentList, key=key)
        else:
            self.ComponentList.setCurrentIndex(0)
        self.ComponentList.blockSignals(False)
        self._component = int(self.ComponentList.currentData())

        # Enable the widget
        self.FileList.setEnabled(True)
        self.VariableList.setEnabled(True)
        self.updateOptions()

    def updateOptions(self):
        """
        Update the widget based on current selections.
        """
        # Update component widgets
        index = self.VariableList.currentIndex()
        data = self.VariableList.itemData(index)
        if data is None:
            self.ComponentList.setEnabled(False)
        elif data and data.num_components == 1:
            self._component = -1
            self.ComponentList.blockSignals(True)
            self.ComponentList.setCurrentIndex(0)
            self.ComponentList.setEnabled(False)
            self.ComponentList.blockSignals(False)
        else:
            self.ComponentList.setEnabled(True)

        self.setVariable.emit(self._variable)
        self.setComponent.emit(self._component)

    def _setupFileList(self, qobject):
        """
        The setup method for FileList widget.

        Args:
            qobject: The widget being setup.
        """
        qobject.currentIndexChanged.connect(self._callbackFileList)

    def _callbackFileList(self, index):
        """
        Callback for file selection.

        Args:
            index[int]: The index of the selected item.
        """
        self._filename = str(self.FileList.itemData(index))
        self.setFilename.emit(self._filename)
        self.windowRequiresUpdate.emit()

    def _setupVariableList(self, qobject):
        """
        Setup method for variable selection.
        """
        qobject.currentIndexChanged.connect(self._callbackVariableList)
        qobject.setFocusPolicy(QtCore.Qt.StrongFocus)

    def _callbackVariableList(self, index):
        """
        Called when a variable is selected.
        """
        self._variable = str(self.VariableList.currentText())
        self.store(self.VariableList, key=(self._filename, None, None))
        self.load(self.ComponentList, key=(self._filename, self._variable, None))
        self.updateOptions()
        self.windowRequiresUpdate.emit()

    def _setupComponentList(self, qobject):
        """
        Setup for component selection.
        """
        qobject.addItem('Magnitude', -1)
        qobject.addItem('x', 0)
        qobject.addItem('y', 1)
        qobject.addItem('z', 2)
        qobject.setEnabled(False)
        self.ComponentList.currentIndexChanged.connect(self._callbackComponentList)

    def _callbackComponentList(self):
        """
        Called when the component is selected.
        """
        self._component = self.ComponentList.currentData()
        self.store(self.ComponentList, key=(self._filename, self._variable, None))
        self.updateOptions()
        self.windowRequiresUpdate.emit()

    def _setupOpenFiles(self, qobject):
        """
        The setup method for OpenFiles widget.

        Args:
            qobject: The widget being setup.
        """
        qobject.setEnabled(True)
        qobject.clicked.connect(self._callbackOpenFiles)
        qobject.setIcon(peacock.utils.WidgetUtils.createIcon('open_file.ico'))
        qobject.setIconSize(QtCore.QSize(16, 16))
        qobject.setFixedSize(qobject.iconSize())
        qobject.setToolTip("Select Exodus file(s) to open.")
        qobject.setStyleSheet("QPushButton {border:none}")

    def _callbackOpenFiles(self):
        """
        Callback for opening an additional file.
        """
        fd = QtWidgets.QFileDialog()
        fd.setWindowTitle('Select ExodusII File(s)')
        fd.setNameFilter('ExodusII Files (*.e)')
        fd.setDirectory(os.getcwd())

        fd.setFileMode(QtWidgets.QFileDialog.ExistingFiles)
        fd.setOption(QtWidgets.QFileDialog.DontUseNativeDialog)
        fd.setProxyModel(ExodusFilterProxyModel())

        if fd.exec_() == QtWidgets.QDialog.Accepted:
            filenames = [str(fname) for fname in list(fd.selectedFiles())]
            if self.FileList.count() == 0:
                self.onSetFilenames(filenames)
        else:
            return

        # Current index
        index = self.FileList.currentIndex()

        # Block signals so that the fileChanged signal is not emitted with each change
        self.FileList.blockSignals(True)

        # Append the list of available files
        for full_file in filenames:
            if not self.FileList.hasItem(full_file):
                self.FileList.addItem(os.path.basename(full_file), full_file)

            # If the file exists, then update the current index to this new file
            if os.path.exists(full_file):
                index = self.FileList.itemIndex(full_file)

        # Restore signals and update the index, if it changed
        self.FileList.blockSignals(False)
        if index != self.FileList.currentIndex():
            self.FileList.setCurrentIndex(index)

def main(size=None):
    """
    Run the FilePlugin all by its lonesome.
    """
    from peacock.ExodusViewer.ExodusPluginManager import ExodusPluginManager
    from peacock.ExodusViewer.plugins.VTKWindowPlugin import VTKWindowPlugin
    widget = ExodusPluginManager(plugins=[lambda: VTKWindowPlugin(size=size), FilePlugin])
    widget.show()

    return widget, widget.VTKWindowPlugin

if __name__ == '__main__':
    from peacock.utils import Testing
    app = QtWidgets.QApplication(sys.argv)
    filenames = Testing.get_chigger_input_list('mug_blocks_out.e', 'vector_out.e', 'displace.e', 'foo.e')
    #filenames = Testing.get_chigger_input_list('diffusion_1.e', 'diffusion_2.e')
    widget, _ = main(size=[600,600])
    widget.FilePlugin.onSetFilenames(filenames)
    sys.exit(app.exec_())
