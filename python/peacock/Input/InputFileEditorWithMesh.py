#!/usr/bin/env python
from peacock.ExodusViewer.plugins.MeshPlugin import MeshPlugin
from peacock.ExodusViewer.plugins.BackgroundPlugin import BackgroundPlugin
from peacock.ExodusViewer.plugins.ClipPlugin import ClipPlugin
from peacock.ExodusViewer.plugins.BlockPlugin import BlockPlugin
from peacock.base.PluginManager import PluginManager
from peacock.base.TabPlugin import TabPlugin
from PyQt5.QtWidgets import QMessageBox, QWidget, QVBoxLayout, QGridLayout, QBoxLayout
from PyQt5.QtCore import pyqtSignal
from MeshViewerPlugin import MeshViewerPlugin
from InputFileEditorPlugin import InputFileEditorPlugin
import os
import BCHighlighter
import TimeStepEstimate
from InputSettings import InputSettings

class InputFileEditorWithMesh(QWidget, PluginManager, TabPlugin):
    """
    Takes the InputFileEditor and adds the mesh view along with some controls.
    Signals:
        numTimeStepsChanged[int]: The estimated number of time steps has changed.
        inputFileChanged[str]: The path of the new input file
    """
    numTimeStepsChanged = pyqtSignal(int)
    inputFileChanged = pyqtSignal(str)
    updateView = pyqtSignal(object)

    @staticmethod
    def commandLineArgs(parser):
        parser.add_argument('-i', "--input-file",
                dest='input_file',
                type=str,
                help='Input file')

    def __init__(self, size=None, plugins=None):
        if not plugins:
            plugins = [InputFileEditorPlugin,
                    lambda: MeshViewerPlugin(size=size),
                    MeshPlugin,
                    BlockPlugin,
                    BackgroundPlugin,
                    ClipPlugin,
                ]
        super(InputFileEditorWithMesh, self).__init__(plugins=plugins)
        # The layouts for this widget
        self.exe_info = None

        # This should be set to the VTKWindowPlugin based plugin name
        # in setupVTKWindow()
        self.vtkwin = None

        self.MainLayout = QGridLayout()
        # This seemed to be required for the plugin system.
        # We allocate them but never use them. Instead
        # we add the plugins into our grid layout.
        self.LeftLayout = QVBoxLayout()
        self.RightLayout = QVBoxLayout()
        self.WindowLayout = QVBoxLayout()

        self.setLayout(self.MainLayout)

        self.setup()
        self.setupVTKWindow()
        # We want the block plugin on the right side of the mesh and to be vertical.
        # By default it adds itself to LeftLayout and is horizontal.
        self.MainLayout.addWidget(self.vtkwin, 0, 1, 1, 2)
        self.MainLayout.addWidget(self.BlockPlugin, 0, 3)
        self.BlockPlugin.MainLayout.setDirection(QBoxLayout.TopToBottom)
        self.MainLayout.addWidget(self.InputFileEditorPlugin, 0, 0, 2, 1)
        self.MainLayout.addWidget(self.MeshPlugin, 1, 1)
        self.MainLayout.addWidget(self.BackgroundPlugin, 1, 2)
        self.MainLayout.addWidget(self.ClipPlugin, 1, 3)
        # Get the Mesh render window to do most of the stretching
        self.MainLayout.setColumnStretch(1, 2)
        self.MainLayout.setRowStretch(0, 2)
        self.InputFileEditorPlugin.blockChanged.connect(self.blockChanged)
        self.InputFileEditorPlugin.blockSelected.connect(self.highlightChanged)
        self.InputFileEditorPlugin.inputFileChanged.connect(self._updateFromInputFile)

    def setupVTKWindow(self):
        """
        Sets up the connections for the VTKWindow based plugin.
        """
        self.vtkwin = self.MeshViewerPlugin # very important!
        self.MeshViewerPlugin.windowCreated.connect(self.setDefaultView)
        self.MeshViewerPlugin.needInputFile.connect(self.InputFileEditorPlugin.writeInputFile)
        self.updateView.connect(self.MeshViewerPlugin.meshChanged)
        self.MeshViewerPlugin.meshEnabled.connect(self.setViewerEnabled)

    def tabName(self):
        """
        This will be used as the text on the tab.
        """
        return "Input file"

    def _updateFromInputFile(self, path):
        """
        When the input file is changed then we need to update the mesh and
        estimated number of time steps
        Input:
            path[str]: path to new input file.
        """
        self.updateView.emit(self.InputFileEditorPlugin.tree)
        self.inputFileChanged.emit(path)
        if not self.InputFileEditorPlugin.tree.app_info.valid() or not self.InputFileEditorPlugin.tree.input_filename:
            self.numTimeStepsChanged.emit(0)
            return

        exe_node = self.InputFileEditorPlugin.tree.getBlockInfo("/Executioner")
        if exe_node and not exe_node.included:
            self.numTimeStepsChanged.emit(0)
            return
        self.blockChanged(exe_node)

    def setDefaultView(self, reader, result, window):
        """
        Slot that creates chigger.ExodusReult objects for displaying data via VTK.

        Args:
            value[bool]: Visibility status.
            name[str]: The name of the window ('main', 'gold', 'diff')
            filename[str]: The name of the file to open.
        """

        m = self.MeshPlugin
        m.ViewMeshToggle.setChecked(True)
        idx = m.Representation.findText("Wireframe")
        if idx >= 0:
            m.Representation.setCurrentIndex(idx)
        m.mesh()

    def onExecutableInfoChanged(self, exe_info):
        """
        When the exeuctable has changed we need to update the mesh
        Input:
            exe_info[ExecutableInfo]: new information from the executable
        """
        self.InputFileEditorPlugin.executableInfoChanged(exe_info)
        self.updateView.emit(self.InputFileEditorPlugin.tree)

    def blockChanged(self, block):
        """
        Called when a block in the input file changed.
        Input:
            block[BlockInfo]: block that changed
        """
        if block.path.startswith("/BCs/"):
            self.highlightChanged(block)
        elif block.path == "/Mesh" or block.path.startswith("/Mesh/"):
            self.updateView.emit(self.InputFileEditorPlugin.tree)
        elif block.path == "/Executioner" or block.path.startswith("/Executioner/"):
            num_steps = TimeStepEstimate.findTimeSteps(self.InputFileEditorPlugin.tree)
            self.numTimeStepsChanged.emit(num_steps)

    def highlightChanged(self, block):
        """
        Input:
            block[BlockInfo]: This block will be a child of /BCs
        """
        BCHighlighter.highlightBlock(block, self.vtkwin)

    def setViewerEnabled(self, enabled):
        """
        Toggles all the graphics widgets
        Input:
            enabled[bool]: Whether to set them enabled or disabled
        """
        self.MeshPlugin.setEnabled(enabled)
        self.BlockPlugin.setEnabled(enabled)
        self.BackgroundPlugin.setEnabled(enabled)
        self.ClipPlugin.setEnabled(enabled)

    def onWorkingDirChanged(self, path):
        """
        Since the mesh may depend on files, we need to try to regenerate the mesh
        with the working directory is changed.
        Input:
            path[str]: New working directory
        """
        self.updateView.emit(self.InputFileEditorPlugin.tree)

    def canClose(self):
        """
        Called before we actually close.
        We just want to make sure that everything has been saved before closing.
        """
        if self.InputFileEditorPlugin.has_changed:
            msg = "You have unsaved changes in your input file, are you sure you want to quit?"
            reply = QMessageBox.question(self, "Quit?", msg, QMessageBox.Yes, QMessageBox.No)
            return reply != QMessageBox.No
        return True

    def parseOptions(self, options):
        if not options:
            return
        if options.input_file:
            p = os.path.abspath(options.input_file)
            self.setInputFile(p)
            self.inputFileChanged.emit(p)
            options.input_file = p
            return
        for arg in options.arguments:
            if arg.endswith(".i"):
                p = os.path.abspath(arg)
                self.setInputFile(p)
                self.inputFileChanged.emit(p)
                options.input_file = p
                return

    def setInputFile(self, input_file):
        """
        Utility function so that callers don't have to know
        what plugin to set the input file on.
        Input:
            input_file[str]: Path to the input file
        """
        return self.InputFileEditorPlugin.setInputFile(input_file)

    def initialize(self, *args, **kwargs):
        """
        Initializes this widget.
        kwargs can contain 'cmd_line_options' with a argparse namespace of options
        that were parsed on the command line.
        """
        options = kwargs.pop('cmd_line_options', None)
        super(InputFileEditorWithMesh, self).initialize(*args, **kwargs)
        self.setViewerEnabled(False)
        self.parseOptions(options)

    def closing(self):
        """
        Called when the application is about to close.
        """
        for child in self._plugins.itervalues():
            try:
                child.closing()
            except:
                pass

    def addToMainMenu(self, menubar):
        """
        Register the menus specific to the InputTab.
        Input:
            menubar: The menu bar to add the menus to.
        """
        inputMenu = menubar.addMenu("Input File")
        self.InputFileEditorPlugin.addToMenu(inputMenu)

    def clearRecentlyUsed(self):
        """
        Clears all the items in the recently used menu
        """
        self.InputFileEditorPlugin.clearRecentlyUsed()

    def settingsWidget(self):
        """
        Just returns a widget that allows editing of settings for this widget
        """
        return InputSettings()

if __name__ == "__main__":
    from PyQt5.QtWidgets import QApplication, QMainWindow
    from ExecutableInfo import ExecutableInfo
    import sys
    if len(sys.argv) != 3:
        print("Usage: %s <exe> <input file>" % sys.argv[0])
        sys.exit(1)

    qapp = QApplication(sys.argv)
    main_win = QMainWindow()
    w = InputFileEditorWithMesh()
    main_win.setCentralWidget(w)
    exe_info = ExecutableInfo()
    #exe_info.clearCache()
    exe_info.setPath(sys.argv[1])
    w.setInputFile(sys.argv[2])
    w.setEnabled(True)
    w.initialize()
    w.onExecutableInfoChanged(exe_info)
    main_win.show()
    menubar = main_win.menuBar()
    menubar.setNativeMenuBar(False)
    w.addToMainMenu(menubar)
    sys.exit(qapp.exec_())
