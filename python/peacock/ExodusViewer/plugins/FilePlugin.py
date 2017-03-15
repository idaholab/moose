import os
import sys
import glob
import vtk
from PyQt5 import QtCore, QtWidgets
import peacock
from ExodusPlugin import ExodusPlugin
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
    Plugin for controlling the current file being visualized.
    """

    #: pyqtSignal: Emitted when a file is changed via the file select.
    fileChanged = QtCore.pyqtSignal(str)

    #: pyqtSignal: Emitted when the file has changed and a camera has been stashed
    cameraChanged = QtCore.pyqtSignal(vtk.vtkCamera)

    def __init__(self):
        super(FilePlugin, self).__init__()

        # Setup this widget widget
        self.setTitle('Select File(s):')
        self.setEnabled(True) # The base disables, this needs to be enabled so the open button is available

        self.MainLayout = QtWidgets.QHBoxLayout()
        self.setLayout(self.MainLayout)

        self.OpenFiles = QtWidgets.QPushButton()
        self.AvailableFiles = ExodusComboBox()

        self.MainLayout.addWidget(self.OpenFiles)
        self.MainLayout.addWidget(self.AvailableFiles)

        self.FileOpenDialog = QtWidgets.QFileDialog()
        self.FileOpenDialog.setWindowTitle('Select ExodusII File(s)')
        self.FileOpenDialog.setNameFilter('ExodusII Files (*.e)')
        self.FileOpenDialog.setDirectory('/Users/slauae/projects/gui/tests/chigger/input')

        self.FileOpenDialog.setFileMode(QtWidgets.QFileDialog.ExistingFiles)
        self.FileOpenDialog.setOption(QtWidgets.QFileDialog.DontUseNativeDialog)
        self.FileOpenDialog.setProxyModel(ExodusFilterProxyModel())

        # A cache for camera settings
        self._cameras = dict()

        self.setup()

    def initialize(self, filenames):
        """
        Updates the list of available files for selection.

        Args:
            filenames[list]: The filenames to include in the AvailableFiles widget.
        """
        super(FilePlugin, self).initialize()

        # Clear the existing list of files
        self.AvailableFiles.blockSignals(True)
        self.AvailableFiles.clear()
        self.AvailableFiles.blockSignals(False)

        # Populate the list of files
        for full_file in filenames:
            self.AvailableFiles.addItem(os.path.basename(full_file), full_file)

        # Reset the current index to the first item
        self.AvailableFiles.setCurrentIndex(0)

    def _setupAvailableFiles(self, qobject):
        """
        The setup method for AvailableFiles widget.

        Args:
            qobject: The widget being setup.
        """
        qobject.currentIndexChanged.connect(self._callbackAvailableFiles)

    def _callbackAvailableFiles(self, index):
        """
        Callback for file selection.

        Args:
            index[int]: The index of the selected item.
        """
        full_file = str(self.AvailableFiles.itemData(index))
        self.fileChanged.emit(full_file)

        if full_file in self._cameras:
            self.cameraChanged.emit(self._cameras[full_file])
        elif self._result:
            self._cameras[full_file] = self._result.getVTKRenderer().GetActiveCamera()

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

        if self.FileOpenDialog.exec_() == QtWidgets.QDialog.Accepted:
            filenames = [str(fname) for fname in list(self.FileOpenDialog.selectedFiles())]
            if self.AvailableFiles.count() == 0:
                self.initialize(filenames)
        else:
            return

        # Current index
        index = self.AvailableFiles.currentIndex()

        # Block signals so that the fileChanged signal is not emitted with each change
        self.AvailableFiles.blockSignals(True)

        # Append the list of available files
        for full_file in filenames:
            if not self.AvailableFiles.hasItem(full_file):
                self.AvailableFiles.addItem(os.path.basename(full_file), full_file)

            # If the file exists, then update the current index to this new file
            if os.path.exists(full_file):
                index = self.AvailableFiles.itemIndex(full_file)

        # Restore signals and update the index, if it changed
        self.AvailableFiles.blockSignals(False)

        if index != self.AvailableFiles.currentIndex():
            self.AvailableFiles.setCurrentIndex(index)


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
    filenames = Testing.get_chigger_input_list('mug_blocks_out.e', 'vector_out.e', 'displace.e')
    widget, _ = main()
    widget.initialize(filenames)
    sys.exit(app.exec_())
