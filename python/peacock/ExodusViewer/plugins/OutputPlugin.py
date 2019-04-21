#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import sys
import types
import vtk

from PyQt5 import QtWidgets, QtCore
from .ExodusPlugin import ExodusPlugin

from peacock.utils.TextSubWindow import TextSubWindow

class OutputPlugin(QtWidgets.QGroupBox, ExodusPlugin):
    """
    Plugin responsible for triggering the creation of png/py files and live script viewing.
    """

    #: pyqtSignal: Write the chigger script portions from this plugin
    write = QtCore.pyqtSignal(str)

    #: pyqtSignal: Create an observer for the given event with the supplied method callback function
    addObserver = QtCore.pyqtSignal(type(vtk.vtkCommand.RenderEvent), types.MethodType)

    def __init__(self):
        super(OutputPlugin, self).__init__()
        self.MainLayout = QtWidgets.QHBoxLayout(self)
        self.LiveScriptWindow = TextSubWindow()
        self.hide()

        self.LiveScript = None
        self.ExportPNG = None
        self.ExportPython = None

        self.setup()
        self._observer = None

    def addToMenu(self, menu):
        """
        Adds output options to the 'Results' menu.
        """
        self.LiveScript = menu.addAction('Show live script')
        self.LiveScript.setCheckable(True)
        self.LiveScript.setChecked(False)
        self.LiveScript.setEnabled(False)
        self.LiveScript.toggled.connect(self._callbackLiveScript)

        menu.addSeparator()
        export = menu.addMenu('Export')

        self.ExportPNG = export.addAction('PNG')
        self.ExportPNG.triggered.connect(self._callbackExportPNG)
        self.ExportPNG.setEnabled(False)

        self.ExportPython = export.addAction('Python')
        self.ExportPython.triggered.connect(self._callbackExportPython)
        self.ExportPython.setEnabled(False)

    def updateLiveScriptWindow(self, *args):
        """
        Updates the chigger script live view.
        """
        if self.LiveScriptWindow.isVisible() and hasattr(self, "_plugin_manager"):
            # don't reset the text if it is the same. This allows for easier select/copy
            s = self._plugin_manager.repr()
            if s != self.LiveScriptWindow.toPlainText():
                self.LiveScriptWindow.setText(s)

    def onResetWindow(self):
        """
        Disable the live view when there are no results.
        """
        self.LiveScript.setEnabled(False)
        self.ExportPNG.setEnabled(False)
        self.ExportPython.setEnabled(False)

    def onUpdateWindow(self, *args):
        """
        Adds an observer for the live window update to the VTK renderer.
        """
        self.addObserver.emit(vtk.vtkCommand.RenderEvent, self.updateLiveScriptWindow)
        self.LiveScript.setEnabled(True)
        self.ExportPNG.setEnabled(True)
        self.ExportPython.setEnabled(True)

    def _setupLiveScriptWindow(self, qobject):
        qobject.setReadOnly(True)
        qobject.hide()
        qobject.windowClosed.connect(self._callbackCloseLiveScriptWindow)

    def _callbackLiveScript(self, *args):
        if self.LiveScript.isChecked() and hasattr(self, "_plugin_manager"):
            self.LiveScriptWindow.show()
            self.updateLiveScriptWindow()
        else:
            self.LiveScriptWindow.hide()

    def _callbackExportPNG(self):
        """
        Write a png file of figure.
        """
        dialog = QtWidgets.QFileDialog()
        dialog.setWindowTitle('Write *.png of figure')
        dialog.setNameFilter('PNG files (*.png)')
        dialog.setFileMode(QtWidgets.QFileDialog.AnyFile)
        dialog.setAcceptMode(QtWidgets.QFileDialog.AcceptSave)
        dialog.setOption(QtWidgets.QFileDialog.DontUseNativeDialog)
        dialog.setDefaultSuffix("png")

        if dialog.exec_() == QtWidgets.QDialog.Accepted:
            filename = str(dialog.selectedFiles()[0])
            self.write.emit(filename)

    def _callbackExportPython(self):
        """
        Open dialog and write script.
        """
        dialog = QtWidgets.QFileDialog()
        dialog.setWindowTitle('Write Python Script')
        dialog.setNameFilter('Python Files (*.py)')
        dialog.setFileMode(QtWidgets.QFileDialog.AnyFile)
        dialog.setAcceptMode(QtWidgets.QFileDialog.AcceptSave)
        dialog.setOption(QtWidgets.QFileDialog.DontUseNativeDialog)

        if dialog.exec_() == QtWidgets.QDialog.Accepted:
            filename = str(dialog.selectedFiles()[0])
            self.write.emit(filename)

    def _callbackCloseLiveScriptWindow(self, *args):
        self.LiveScript.setChecked(False)


def main(size=None):
    """
    Run the ContourPlugin all by its lonesome.
    """
    from peacock.ExodusViewer.ExodusPluginManager import ExodusPluginManager
    from peacock.ExodusViewer.plugins.VTKWindowPlugin import VTKWindowPlugin
    widget = ExodusPluginManager(plugins=[lambda: VTKWindowPlugin(size=size), OutputPlugin])
    main_window = QtWidgets.QMainWindow()
    main_window.setCentralWidget(widget)
    menubar = main_window.menuBar()
    menubar.setNativeMenuBar(False)
    widget.addToMainMenu(menubar)
    main_window.show()
    return widget, widget.VTKWindowPlugin, main_window

if __name__ == '__main__':
    from peacock.utils import Testing
    app = QtWidgets.QApplication(sys.argv)
    filename = Testing.get_chigger_input('mug_blocks_out.e')
    widget, window, main_window = main()
    window.onSetFilename(filename)
    window.onSetVariable('diffused')
    window.onWindowRequiresUpdate()
    sys.exit(app.exec_())
