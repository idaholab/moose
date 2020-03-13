#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from PyQt5.QtWidgets import QWidget, QFileDialog, QMessageBox, QApplication
from PyQt5.QtCore import pyqtSignal, Qt, QFileSystemWatcher
import os, shlex
from peacock.Input.ExecutableInfo import ExecutableInfo
from peacock.utils import WidgetUtils
from peacock.base.Plugin import Plugin
import mooseutils
from peacock.utils.RecentlyUsedMenu import RecentlyUsedMenu

class ExecuteOptionsPlugin(QWidget, Plugin):
    """
    Handles setting the various arguments for running.
    Signals:
        executableChanged(str): Path of the new executable is emitted when changed
        executableInfoChanged(ExecutableInfo): Emitted when the executable path is changed
        workingDirChanged(str): Path of the current directory is changed
    """
    executableChanged = pyqtSignal(str)
    executableInfoChanged = pyqtSignal(ExecutableInfo)
    workingDirChanged = pyqtSignal(str)
    useTestObjectsChanged = pyqtSignal(bool)

    def __init__(self, **kwds):
        super(ExecuteOptionsPlugin, self).__init__(**kwds)

        self._preferences.addInt("execute/maxRecentWorkingDirs",
                "Max recent working directories",
                10,
                1,
                50,
                "Set the maximum number of recent working directories that have been used.",
                )
        self._preferences.addInt("execute/maxRecentExes",
                "Max recent executables",
                10,
                1,
                50,
                "Set the maximum number of recent executables that have been used.",
                )
        self._preferences.addInt("execute/maxRecentArgs",
                "Max recent command line arguments",
                10,
                1,
                50,
                "Set the maximum number of recent command line arguments that have been used.",
                )
        self._preferences.addBool("execute/allowTestObjects",
                "Allow using test objects",
                False,
                "Allow using test objects by default",
                )
        self._preferences.addBool("execute/mpiEnabled",
                "Enable MPI by default",
                False,
                "Set the MPI checkbox on by default",
                )
        self._preferences.addString("execute/mpiArgs",
                "Default mpi command",
                "mpiexec -n 2",
                "Set the default MPI command to run",
                )
        self._preferences.addBool("execute/threadsEnabled",
                "Enable threads by default",
                False,
                "Set the threads checkbox on by default",
                )
        self._preferences.addString("execute/threadsArgs",
                "Default threads arguments",
                "--n-threads=2",
                "Set the default threads arguments",
                )

        self.all_exe_layout = WidgetUtils.addLayout(grid=True)
        self.setLayout(self.all_exe_layout)

        self.working_label = WidgetUtils.addLabel(None, self, "Working Directory")
        self.all_exe_layout.addWidget(self.working_label, 0, 0)
        self.choose_working_button = WidgetUtils.addButton(None, self, "Choose", self._chooseWorkingDir)
        self.all_exe_layout.addWidget(self.choose_working_button, 0, 1)
        self.working_line = WidgetUtils.addLineEdit(None, self, None, readonly=True)
        self.working_line.setText(os.getcwd())
        self.all_exe_layout.addWidget(self.working_line, 0, 2)

        self.exe_label = WidgetUtils.addLabel(None, self, "Executable")
        self.all_exe_layout.addWidget(self.exe_label, 1, 0)
        self.choose_exe_button = WidgetUtils.addButton(None, self, "Choose", self._chooseExecutable)
        self.all_exe_layout.addWidget(self.choose_exe_button, 1, 1)
        self.exe_line = WidgetUtils.addLineEdit(None, self, None, readonly=True)
        self.all_exe_layout.addWidget(self.exe_line, 1, 2)

        self.args_label = WidgetUtils.addLabel(None, self, "Extra Arguments")
        self.all_exe_layout.addWidget(self.args_label, 2, 0)
        self.args_line = WidgetUtils.addLineEdit(None, self, None)
        self.all_exe_layout.addWidget(self.args_line, 2, 2)

        self.test_label = WidgetUtils.addLabel(None, self, "Allow test objects")
        self.all_exe_layout.addWidget(self.test_label, 3, 0)
        self.test_checkbox = WidgetUtils.addCheckbox(None, self, "", self._allowTestObjects)
        self.test_checkbox.setChecked(self._preferences.value("execute/allowTestObjects"))
        self.all_exe_layout.addWidget(self.test_checkbox, 3, 1, alignment=Qt.AlignHCenter)

        self.mpi_label = WidgetUtils.addLabel(None, self, "Use MPI")
        self.all_exe_layout.addWidget(self.mpi_label, 4, 0)
        self.mpi_checkbox = WidgetUtils.addCheckbox(None, self, "", None)
        self.mpi_checkbox.setChecked(self._preferences.value("execute/mpiEnabled"))
        self.all_exe_layout.addWidget(self.mpi_checkbox, 4, 1, alignment=Qt.AlignHCenter)
        self.mpi_line = WidgetUtils.addLineEdit(None, self, None)
        self.mpi_line.setText(self._preferences.value("execute/mpiArgs"))
        self.mpi_line.cursorPositionChanged.connect(self._mpiLineCursorChanged)
        self.all_exe_layout.addWidget(self.mpi_line, 4, 2)

        self.threads_label = WidgetUtils.addLabel(None, self, "Use Threads")
        self.all_exe_layout.addWidget(self.threads_label, 5, 0)
        self.threads_checkbox = WidgetUtils.addCheckbox(None, self, "", None)
        self.threads_checkbox.setChecked(self._preferences.value("execute/threadsEnabled"))
        self.all_exe_layout.addWidget(self.threads_checkbox, 5, 1, alignment=Qt.AlignHCenter)
        self.threads_line = WidgetUtils.addLineEdit(None, self, None)
        self.threads_line.setText(self._preferences.value("execute/threadsArgs"))
        self.threads_line.cursorPositionChanged.connect(self._threadsLineCursorChanged)
        self.all_exe_layout.addWidget(self.threads_line, 5, 2)

        self.csv_label = WidgetUtils.addLabel(None, self, "Postprocessor CSV Output")
        self.all_exe_layout.addWidget(self.csv_label, 6, 0)
        self.csv_checkbox = WidgetUtils.addCheckbox(None, self, "", None)
        self.all_exe_layout.addWidget(self.csv_checkbox, 6, 1, alignment=Qt.AlignHCenter)
        self.csv_checkbox.setCheckState(Qt.Checked)

        self.recover_label = WidgetUtils.addLabel(None, self, "Recover")
        self.all_exe_layout.addWidget(self.recover_label, 7, 0)
        self.recover_checkbox = WidgetUtils.addCheckbox(None, self, "", None)
        self.all_exe_layout.addWidget(self.recover_checkbox, 7, 1, alignment=Qt.AlignHCenter)

        self._recent_exe_menu = None
        self._recent_working_menu = None
        self._recent_args_menu = None
        self._force_reload_action = None
        self._exe_watcher = QFileSystemWatcher()
        self._exe_watcher.fileChanged.connect(self.setExecutablePath)

        self._loading_dialog = QMessageBox(parent=self)
        self._loading_dialog.setWindowTitle("Loading executable")
        self._loading_dialog.setStandardButtons(QMessageBox.NoButton) # get rid of the OK button
        self._loading_dialog.setWindowModality(Qt.ApplicationModal)
        self._loading_dialog.setIcon(QMessageBox.Information)
        self._loading_dialog.setText("Loading executable")

        self.setup()

    def setExecutablePath(self, app_path):
        """
        The user select a new executable path.
        Input:
            app_path: The path of the executable.
        """
        if not app_path:
            return

        self._loading_dialog.setInformativeText(app_path)
        self._loading_dialog.show()
        self._loading_dialog.raise_()
        QApplication.processEvents()

        app_info = ExecutableInfo()
        app_info.setPath(app_path, self.test_checkbox.isChecked())

        QApplication.processEvents()

        if app_info.valid():
            self.exe_line.setText(app_path)
            self.executableInfoChanged.emit(app_info)
            self.executableChanged.emit(app_path)
            files = self._exe_watcher.files()
            if files:
                self._exe_watcher.removePaths(files)
            self._exe_watcher.addPath(app_path)
        if self._force_reload_action:
            self._force_reload_action.setEnabled(app_info.valid())
        self._updateRecentExe(app_path, not app_info.valid())
        self._loading_dialog.hide()

    def _chooseExecutable(self):
        """
        Open a dialog to allow the user to choose an executable.
        """
        #FIXME: QFileDialog seems to be a bit broken. Using
        # .setFilter() to filter only executable files doesn't
        # seem to work. Setting a QSortFilterProxyModel doesn't
        # seem to work either.
        # So just use the static method.
        exe_name, other = QFileDialog.getOpenFileName(self, "Chooose executable")
        self.setExecutablePath(exe_name)

    def _allowTestObjects(self):
        """
        Reload the ExecutableInfo based on whether we are allowing test objects or not.
        """
        self.useTestObjectsChanged.emit(self.test_checkbox.isChecked())
        self.setExecutablePath(self.exe_line.text())

    def _workingDirChanged(self):
        """
        Slot called when working directory changed.
        """
        working = str(self.working_line.text())
        self.setWorkingDir(working)

    def _chooseWorkingDir(self):
        """
        Open dialog to choose a current working directory.
        """
        dirname = QFileDialog.getExistingDirectory(self, "Choose directory")
        self.setWorkingDir(dirname)

    def setWorkingDir(self, dir_name):
        """
        Sets the working directory.
        Input:
            dir_name: The path of the working directory.
        """
        if not dir_name:
            return
        old_dirname = str(self.working_line.text())
        try:
            os.chdir(dir_name)
            self.working_line.setText(dir_name)
            if old_dirname != dir_name:
                self.workingDirChanged.emit(dir_name)
            self._updateRecentWorkingDir(dir_name)
        except OSError:
            mooseutils.mooseError("Invalid directory %s" % dir_name, dialog=True)
            self._updateRecentWorkingDir(dir_name, True)

    def _setExecutableArgs(self, args):
        """
        Set the executable arguments.
        Input:
            args: str: A string of all the arguments.
        """
        self.args_line.setText(args)

    def buildCommand(self, input_file):
        cmd, args = self.buildCommandWithNoInputFile()
        args.append("-i")
        args.append(os.path.relpath(input_file))
        return cmd, args

    def buildCommandWithNoInputFile(self):
        """
        Builds the full command line with arguments.
        Return: <string of command to run>, <list of arguments>
        """
        cmd = ""
        args = []
        if self.mpi_checkbox.isChecked():
            mpi_args = shlex.split(str(self.mpi_line.text()))
            if mpi_args:
                cmd = mpi_args[0]
                args = mpi_args[1:]
                args.append(str(self.exe_line.text()))

        if not cmd:
            cmd = str(self.exe_line.text())

        args += shlex.split(str(self.args_line.text()))

        if self.recover_checkbox.isChecked():
            args.append("--recover")

        if self.csv_checkbox.isChecked():
            #args.append("--no-color")
            args.append("Outputs/csv=true")

        if self.threads_checkbox.isChecked():
            args += shlex.split(str(self.threads_line.text()))

        return cmd, args

    def _updateRecentExe(self, path, remove=False):
        """
        Updates the recently used menu with the current executable
        """
        if self._recent_exe_menu:
            abs_path = os.path.normcase(os.path.abspath(path))
            if remove:
                self._recent_exe_menu.removeEntry(abs_path)
            else:
                self._recent_exe_menu.update(abs_path)

    def _updateRecentWorkingDir(self, path, remove=False):
        """
        Updates the recently used menu with the current executable
        """
        full_path = os.path.abspath(path)
        if self._recent_working_menu:
            if remove:
                self._recent_working_menu.removeEntry(full_path)
            else:
                self._recent_working_menu.update(full_path)

    def onPreferencesSaved(self):
        self._recent_args_menu.updateRecentlyOpened()
        self._recent_working_menu.updateRecentlyOpened()
        self._recent_exe_menu.updateRecentlyOpened()

    def _reload_syntax(self):
        self.setExecutablePath(self.exe_line.text())

    def addToMenu(self, menu):
        """
        Adds menu entries specific to the Arguments to the menubar.
        """
        workingMenu = menu.addMenu("Recent &working dirs")
        self._recent_working_menu = RecentlyUsedMenu(workingMenu,
                "execute/recentWorkingDirs",
                "execute/maxRecentWorkingDirs",
                20,
                )
        self._recent_working_menu.selected.connect(self.setWorkingDir)
        self._workingDirChanged()

        exeMenu = menu.addMenu("Recent &executables")
        self._recent_exe_menu = RecentlyUsedMenu(exeMenu,
                "execute/recentExes",
                "execute/maxRecentExes",
                20,
                )
        self._recent_exe_menu.selected.connect(self.setExecutablePath)

        argsMenu = menu.addMenu("Recent &arguments")
        self._recent_args_menu = RecentlyUsedMenu(argsMenu,
                "execute/recentArgs",
                "execute/maxRecentArgs",
                20,
                )
        self._recent_args_menu.selected.connect(self._setExecutableArgs)
        self._force_reload_action = WidgetUtils.addAction(menu, "Reload executable syntax", self._reload_syntax)
        self._force_reload_action.setEnabled(False)

    def clearRecentlyUsed(self):
        if self._recent_args_menu:
            self._recent_args_menu.clearValues()
            self._recent_working_menu.clearValues()
            self._recent_exe_menu.clearValues()
            self._workingDirChanged()

    def _mpiLineCursorChanged(self, old, new):
        self.mpi_checkbox.setChecked(True)

    def _threadsLineCursorChanged(self, old, new):
        self.threads_checkbox.setChecked(True)

if __name__ == "__main__":
    from PyQt5.QtWidgets import QMainWindow
    import sys
    qapp = QApplication(sys.argv)
    main_win = QMainWindow()
    w = ExecuteOptionsPlugin()
    main_win.setCentralWidget(w)
    main_win.show()
    menubar = main_win.menuBar()
    menubar.setNativeMenuBar(False)
    executeMenu = menubar.addMenu("E&xecute")
    w.addToMenu(executeMenu)
    sys.exit(qapp.exec_())
