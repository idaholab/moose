#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from peacock.base.PluginManager import PluginManager
from peacock.base.TabPlugin import TabPlugin
from PyQt5.QtWidgets import QWidget, QVBoxLayout
from PyQt5.QtCore import pyqtSignal
from peacock.utils import ExeFinder
from .ExecuteOptionsPlugin import ExecuteOptionsPlugin
from .ExecuteRunnerPlugin import ExecuteRunnerPlugin
from .ConsoleOutputViewerPlugin import ConsoleOutputViewerPlugin
from peacock.Input.ExecutableInfo import ExecutableInfo
import os

class ExecuteTabPlugin(QWidget, PluginManager, TabPlugin):
    """
    The GUI for running an executable.
    It contains a JobRunner object that actually
    runs the process and it communicates via signals.
    Signals:
        executableInfoChanged: The executable changed. Argument is the new ExecutableInfo
        needInputFile: Emitted when this widget needs an input file written to disk. Argument is the path to the file to be written.
        start_job: Emitted when a job is started. Arguments are whether CSV output is enabled, and input filename
    """
    needInputFile = pyqtSignal(str)
    executableInfoChanged = pyqtSignal(ExecutableInfo)
    startJob = pyqtSignal(bool, str, float)

    @staticmethod
    def commandLineArgs(parser):
        """
        Add the option to specify the executable and method on the command line
        Input:
            parser[argparse.ArgumentParser]: The parse to add options to
        """
        group = parser.add_argument_group("Executable", "Finding or setting the initial executable")
        group.add_argument('-e', '--executable', dest='executable', type=str, help='Executable file')
        group.add_argument('--run', dest='auto_run', action='store_true', help='If an input file and executable are also given on the command line, then immediately run it.')
        group.add_argument('--no-exe-search', action='store_false', dest='exe_search', help='Do not do an automatic executable search')
        group.add_argument('-m', '--method',
                dest='method',
                choices=('opt', 'dbg', 'oprof', 'devel'),
                help="Works the same as setting the $METHOD environment variable. This setting will take precedence over $METHOD")

    def __init__(self, plugins=[ExecuteOptionsPlugin, ExecuteRunnerPlugin, ConsoleOutputViewerPlugin]):
        super(ExecuteTabPlugin, self).__init__(plugins=plugins)
        self.MainLayout = QVBoxLayout()

        self.setLayout(self.MainLayout)

        self.setup()
        self.ExecuteOptionsPlugin.executableInfoChanged.connect(self.onExecutableInfoChanged)
        self.ExecuteRunnerPlugin.needInputFile.connect(self.needInputFile)
        self.ExecuteRunnerPlugin.needCommand.connect(self.onNeedCommand)
        self.ExecuteRunnerPlugin.startJob.connect(self.startJob)

        self.search_from_dir = os.getcwd()

    def tabName(self):
        return "Execute"

    def onExecutableInfoChanged(self, exe_info):
        """
        Executable has changed.
        Input:
            exe_info[ExecutableInfo]: New ExecutableInfo object
        """
        if exe_info.valid():
            self.executableInfoChanged.emit(exe_info)
        self.ExecuteRunnerPlugin.runEnabled(exe_info.valid())

    def onNeedCommand(self):
        cmd, args = self.ExecuteOptionsPlugin.buildCommandWithNoInputFile()
        csv = self.ExecuteOptionsPlugin.csv_checkbox.isChecked()
        if self.ExecuteOptionsPlugin.test_checkbox.isChecked():
            args.append("--allow-test-objects")
        self.ExecuteRunnerPlugin.setCommand(cmd, args, csv)

    def onNumTimeStepsChanged(self, num_steps):
        """
        This will get auto connected to InputFileEditorWithMesh:numTimeStepsChanged signal
        Input:
            num_steps[int]: new number of time steps
        """
        self.ExecuteRunnerPlugin.onNumTimeStepsChanged(num_steps)

    def setExe(self, options):
        """
        Tries to find an executable.
        It first looks in the command line options.
        If not found it will search up the directory path.
        Input:
            options[argparse namespace]: The command line options as returned by ArgumentParser.parse_args()
        """
        if options.executable and not os.path.isabs(options.executable):
            options.executable = os.path.abspath(os.path.join(options.start_dir, options.executable))
        exe_path = ExeFinder.getExecutablePath(options, start_dir=self.search_from_dir)
        if exe_path:
            self.ExecuteOptionsPlugin.setExecutablePath(exe_path)

    def initialize(self, options):
        """
        Initialize this widget.
        It will search kwargs for "cmd_line_options" to help initialize
        this object from arguments on the command line.
        """
        super(ExecuteTabPlugin, self).initialize(options)
        self.setExe(options)
        self.setEnabled(True)

    def clearRecentlyUsed(self):
        """
        Clear the recently used menus
        """
        self.ExecuteOptionsPlugin.clearRecentlyUsed()

    def addToMainMenu(self, menubar):
        """
        Adds menu entries specific to this tab to the menubar.
        """
        executeMenu = menubar.addMenu("E&xecute")
        self.ExecuteOptionsPlugin.addToMenu(executeMenu)

    def closing(self):
        """
        Gets called when the user tries to quit.

        Make sure things are saved before quitting.
        """
        self.ExecuteRunnerPlugin.closing()

if __name__ == "__main__":
    from PyQt5.QtWidgets import QApplication, QMainWindow
    from peacock.utils import Testing
    import argparse
    import sys

    parser = argparse.ArgumentParser(description='Execute tab')
    main_win = QMainWindow()
    ExecuteTabPlugin.commandLineArgs(parser)
    exe = Testing.find_moose_test_exe()
    def needInputFile(input_file):
        this_dir = os.path.dirname(os.path.abspath(__file__))
        peacock_dir = os.path.dirname(this_dir)
        test_file = os.path.join(peacock_dir, "tests", "common", "transient.i")
        with open(test_file, "r") as fin:
            data = fin.read()
            with open(input_file, "w") as fout:
                fout.write(data)

    parsed = parser.parse_args(["-e", exe])
    qapp = QApplication(sys.argv)
    w = ExecuteTabPlugin()
    w.needInputFile.connect(needInputFile)
    main_win.setCentralWidget(w)
    menubar = main_win.menuBar()
    menubar.setNativeMenuBar(False)
    w.addToMainMenu(menubar)
    main_win.show()
    w.initialize(parsed)
    sys.exit(qapp.exec_())
