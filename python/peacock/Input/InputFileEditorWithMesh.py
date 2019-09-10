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
from PyQt5.QtWidgets import QMessageBox, QWidget, QVBoxLayout, QHBoxLayout
from PyQt5.QtCore import pyqtSignal
from peacock.ExodusViewer.plugins.BackgroundPlugin import BackgroundPlugin
from peacock.base.PluginManager import PluginManager
from peacock.base.TabPlugin import TabPlugin
from .plugins import MeshViewerPlugin
from .plugins import MeshCameraPlugin
from .BlockHighlighterPlugin import BlockHighlighterPlugin
from .InputFileEditorPlugin import InputFileEditorPlugin
from . import BCHighlighter
from . import TimeStepEstimate

class InputFileEditorWithMesh(QWidget, PluginManager, TabPlugin):
    """
    Takes the InputFileEditor and adds the mesh view along with some controls.
    Signals:
        numTimeStepsChanged[int]: The estimated number of time steps has changed.
        inputFileChanged[str]: The path of the new input file
    """
    numTimeStepsChanged = pyqtSignal(int)
    inputFileChanged = pyqtSignal(str)
    updateView = pyqtSignal(object, bool)

    @staticmethod
    def commandLineArgs(parser):
        parser.add_argument('-i', "--input-file",
                dest='input_file',
                type=str,
                help='Input file')

    def __init__(self, size=None, plugins=None):
        if not plugins:
            plugins = [lambda: InputFileEditorPlugin(layout='LeftLayout'),
                       lambda: MeshViewerPlugin(size=size, layout='WindowLayout'),
                       lambda: BackgroundPlugin(layout='WindowBottomLayout',
                                                settings_key="input",
                                                set_result_color=True),
                       lambda: BlockHighlighterPlugin(layout='WindowBottomLayout'),
                       lambda: MeshCameraPlugin(layout='WindowBottomLayout')]

        super(InputFileEditorWithMesh, self).__init__(plugins=plugins)
        self.setTabName('Input file')

        # The layouts for this widget
        self.exe_info = None

        # This should be set to the VTKWindowPlugin based plugin name in setupVTKWindow()
        self.vtkwin = None

        self.MainLayout = QHBoxLayout(self)
        self.LeftLayout = QVBoxLayout()
        self.WindowLayout = QVBoxLayout()

        self.WindowBottomLayout = QHBoxLayout()

        self.MainLayout.addLayout(self.LeftLayout)
        self.MainLayout.addLayout(self.WindowLayout)
        self.setup()
        self.setupVTKWindow()
        self.WindowLayout.addLayout(self.WindowBottomLayout)

        self.InputFileEditorPlugin.blockChanged.connect(self.blockChanged)
        self.InputFileEditorPlugin.blockSelected.connect(self.highlightChanged)
        self.InputFileEditorPlugin.inputFileChanged.connect(self._updateFromInputFile)
        self.MeshCameraPlugin.reloadMesh.connect(self.onMeshChanged)

        self.fixLayoutWidth('LeftLayout')

    def setupVTKWindow(self):
        """
        Sets up the connections for the VTKWindow based plugin.
        """
        self.vtkwin = self.MeshViewerPlugin # very important!
        self.MeshViewerPlugin.needInputFile.connect(self.InputFileEditorPlugin.writeInputFile)
        self.updateView.connect(self.MeshViewerPlugin.meshChanged)
        self.MeshViewerPlugin.meshEnabled.connect(self.setViewerEnabled)

    def _updateFromInputFile(self, path):
        """
        When the input file is changed then we need to update the mesh and
        estimated number of time steps
        Input:
            path[str]: path to new input file.
        """
        self.inputFileChanged.emit(path)
        self.updateView.emit(self.InputFileEditorPlugin.tree, True)
        if not self.InputFileEditorPlugin.tree.app_info.valid() or not self.InputFileEditorPlugin.tree.input_filename:
            self.numTimeStepsChanged.emit(0)
            return

        exe_node = self.InputFileEditorPlugin.tree.getBlockInfo("/Executioner")
        if exe_node and not exe_node.included:
            self.numTimeStepsChanged.emit(0)
            return
        self.blockChanged(exe_node)

    def onExecutableInfoChanged(self, exe_info):
        """
        When the executable has changed we need to update the mesh
        Input:
            exe_info[ExecutableInfo]: new information from the executable
        """
        self.InputFileEditorPlugin.executableInfoChanged(exe_info)
        self.updateView.emit(self.InputFileEditorPlugin.tree, True)

    def onMeshChanged(self):
        """
        Re-create the mesh file.
        """
        self.updateView.emit(self.InputFileEditorPlugin.tree, True)

    def blockChanged(self, block):
        """
        Called when a block in the input file changed.
        Input:
            block[BlockInfo]: block that changed
        """
        if block.path.startswith("/BCs/"):
            self.highlightChanged(block)
        elif block.path == "/Mesh" or block.path.startswith("/Mesh/"):
            self.updateView.emit(self.InputFileEditorPlugin.tree, False)
        elif block.path == "/MeshModifiers" or block.path.startswith("/MeshModifiers/"):
            self.updateView.emit(self.InputFileEditorPlugin.tree, False)
        elif block.path == "/MeshGenerators" or block.path.startswith("/MeshGenerators/"):
            self.updateView.emit(self.InputFileEditorPlugin.tree, False)
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
        self.BlockHighlighterPlugin.setEnabled(enabled)
        self.BackgroundPlugin.setEnabled(enabled)

    def onWorkingDirChanged(self, path):
        """
        Since the mesh may depend on files, we need to try to regenerate the mesh
        with the working directory is changed.
        Input:
            path[str]: New working directory
        """
        self.updateView.emit(self.InputFileEditorPlugin.tree, True)

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

    def setInputFile(self, input_file):
        """
        Utility function so that callers don't have to know
        what plugin to set the input file on.
        Input:
            input_file[str]: Path to the input file
        """
        return self.InputFileEditorPlugin.setInputFile(input_file)

    def initialize(self, options):
        """
        Initializes this widget,
        kwargs can contain 'cmd_line_options' with a argparse namespace of options
        that were parsed on the command line.
        """
        super(InputFileEditorWithMesh, self).initialize(options)
        self.setViewerEnabled(False)

        # Locate input file in arguments without command
        if not options.input_file and options.arguments:
            for arg in options.arguments:
                if arg.endswith(".i"):
                    options.input_file = os.path.abspath(arg)
                    break

        # Load the supplied input file.
        if options.input_file:
            p = os.path.abspath(options.input_file)
            self.setInputFile(p)
            self.inputFileChanged.emit(p)

    def closing(self):
        """
        Called when the application is about to close.
        """
        for child in self._plugins.values():
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
        self.BackgroundPlugin.addToMenu(inputMenu)

    def clearRecentlyUsed(self):
        """
        Clears all the items in the recently used menu

        """
        self.InputFileEditorPlugin.clearRecentlyUsed()

    def onUseTestObjectsChanged(self, use_test_objs):
        """
        Pass along whether to use test objects.
        """
        self.MeshViewerPlugin.useTestObjects(use_test_objs)

if __name__ == "__main__":
    import collections
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
    menubar = main_win.menuBar()
    menubar.setNativeMenuBar(False)
    w.addToMainMenu(menubar)
    exe_info = ExecutableInfo()
    #exe_info.clearCache()
    exe_info.setPath(sys.argv[1])
    w.setInputFile(sys.argv[2])
    w.setEnabled(True)
    OptionsProxy = collections.namedtuple('Options', 'input_file arguments')
    w.initialize(OptionsProxy(input_file=sys.argv[2], arguments=[]))
    w.onExecutableInfoChanged(exe_info)
    main_win.show()
    sys.exit(qapp.exec_())
