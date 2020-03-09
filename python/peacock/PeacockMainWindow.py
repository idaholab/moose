#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from .Input import OutputNames
from .PythonConsoleWidget import PythonConsoleWidget
from .ExodusViewer.ExodusViewer import ExodusViewer
from .PostprocessorViewer.PostprocessorViewer import PostprocessorViewer
from .utils import WidgetUtils
from .BasePeacockMainWindow import BasePeacockMainWindow
import mooseutils
from .PostprocessorViewer.VectorPostprocessorViewer import VectorPostprocessorViewer
from .Input.InputFileEditorWithMesh import InputFileEditorWithMesh
from .Execute.ExecuteTabPlugin import ExecuteTabPlugin
import os

class PeacockMainWindow(BasePeacockMainWindow):
    """
    Main Peacock window.

    This includes all the various tabs as well as some menus
    """

    PLUGINS = [InputFileEditorWithMesh, ExecuteTabPlugin, ExodusViewer, PostprocessorViewer, VectorPostprocessorViewer]

    def __init__(self, **kwds):
        super(PeacockMainWindow, self).__init__(plugins=self.PLUGINS, **kwds)
        self.setObjectName("PeacockMainWindow")
        self.setWindowTitle("Peacock")
        self.console = PythonConsoleWidget()
        self.exe_path = ""
        self.input_file_path = ""
        self.tab_plugin.ExecuteTabPlugin.executableInfoChanged.connect(self._executableChanged)
        self.tab_plugin.ExecuteTabPlugin.startJob.connect(self._startJob)
        self.tab_plugin.ExecuteTabPlugin.needInputFile.connect(self.tab_plugin.InputFileEditorWithMesh.InputFileEditorPlugin.writeInputFile)
        self.tab_plugin.InputFileEditorWithMesh.inputFileChanged.connect(self._inputFileChanged)
        self.tab_plugin.ExecuteTabPlugin.ExecuteOptionsPlugin.workingDirChanged.connect(self.tab_plugin.InputFileEditorWithMesh.onWorkingDirChanged)
        self.tab_plugin.ExecuteTabPlugin.ExecuteOptionsPlugin.useTestObjectsChanged.connect(self.tab_plugin.InputFileEditorWithMesh.onUseTestObjectsChanged)
        self.setPythonVariable('main_window', self)
        self.setPythonVariable('tabs', self.tab_plugin)
        self.setup()

    @staticmethod
    def commandLineArgs(parser):
        BasePeacockMainWindow.commandLineArgs(parser, PeacockMainWindow.PLUGINS)

    def initialize(self, options):
        curr_dir = ""
        if options.working_dir:
            curr_dir = os.path.abspath(options.working_dir)
        super(PeacockMainWindow, self).initialize(options)

        if options.working_dir:
            # if the input file is set then it will change directory to where
            # it exists. We need to honor the command line switch for the working dir.
            self.tab_plugin.ExecuteTabPlugin.ExecuteOptionsPlugin.setWorkingDir(curr_dir)
        self._setTitle()

    def _showConsole(self):
        """
        Toggles showing the python console widget
        """
        if self.console.isVisible():
            self.console.hide()
        else:
            self.console.show()

    def setPythonVariable(self, name, value):
        """
        This just passes on the arguments to the python console.

        This is used to add objects to the python console.
        Input:
            name: name of the variable
            value: value of the variable
        """
        self.console.setVariable(name, value)

    def _executableChanged(self, app_info):
        """
        Slot called when the executable has changed.
        """
        self.setPythonVariable("app_info", app_info)
        self.exe_path = app_info.path
        self._setTitle()

    def _inputFileChanged(self, input_filename):
        """
        Slot for when the input file has changed.
        Note that comes from the input tab widget so we don't
        need to update it.
        Input:
            input_filename: Name of the new input_filename
        """
        self.input_file_path = input_filename
        full_filename = os.path.abspath(input_filename)
        new_dir = os.path.dirname(full_filename)
        self.tab_plugin.ExecuteTabPlugin.ExecuteOptionsPlugin.setWorkingDir(new_dir)
        self.tab_plugin.ExecuteTabPlugin.ExecuteRunnerPlugin.setInputFile(full_filename)
        self._setTitle()

    def setTab(self, tabName):
        """
        Set the selected tab by name.

        Normally you select by index or QWidget.
        Input:
            tabname: str: Title of the tab to be selected.
        """
        for idx in range(self.tab_plugin.count()):
            if self.tab_plugin.tabText(idx) == tabName:
                self.tab_plugin.setCurrentIndex(idx)
                return

    def _setTitle(self):
        """
        Sets the title of the window.
        """
        title = "Peacock"
        if self.exe_path:
            title += " : executable - %s" % os.path.abspath(self.exe_path)
        if self.input_file_path:
            title += " : input file - %s" % os.path.abspath(self.input_file_path)
        self.setWindowTitle(title)

    def _startJob(self, csv, inputfile, t):
        """
        This gets called when a job has started on the execute tab
        Input:
            csv[bool]: Whether CSV output has been requested
            inputfile[str]: Path of the input file that will be used
            t[float]: Time at which job started
        """
        tree = self.tab_plugin.InputFileEditorWithMesh.InputFileEditorPlugin.tree
        output_files = OutputNames.getOutputFiles(tree, inputfile)
        if output_files:
            mooseutils.mooseMessage("Exodus filenames: %s" % output_files)
            self.tab_plugin.ExodusViewer.onStartJob(csv, inputfile, t)
            self.tab_plugin.ExodusViewer.onSetFilenames(output_files)

        if csv or OutputNames.csvEnabled(tree):
            pp_files = OutputNames.getPostprocessorFiles(tree, inputfile)
            if pp_files:
                mooseutils.mooseMessage("Postprocessor filenames: %s" % pp_files)
                self.tab_plugin.PostprocessorViewer.onStartJob(csv, inputfile, t)
                self.tab_plugin.PostprocessorViewer.onSetFilenames(pp_files)

        vpp_files = OutputNames.getVectorPostprocessorFiles(tree, inputfile)
        if vpp_files:
            mooseutils.mooseMessage("VectorPostprocessor filenames: %s" % vpp_files)
            self.tab_plugin.VectorPostprocessorViewer.onSetFilenames(vpp_files)

    def _addMenus(self):
        """
        Internal method to allow plugins to add menus to the main menu bar.
        """
        super(PeacockMainWindow, self)._addMenus()
        menubar = self.menuBar()
        debug_menu = menubar.addMenu("Debug")
        WidgetUtils.addAction(debug_menu, "Show Python Console", self._showConsole, "Ctrl+P", True)

if __name__ == "__main__":
    from PyQt5.QtWidgets import QApplication
    import argparse, sys

    app = QApplication(sys.argv)
    parser = argparse.ArgumentParser()
    PeacockMainWindow.commandLineArgs(parser)
    main = PeacockMainWindow()
    main.show()
    main.initialize(parser.parse_args())
    sys.exit(app.exec_())
