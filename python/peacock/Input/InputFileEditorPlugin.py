#!/usr/bin/env python2
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html
import os
from PyQt5.QtWidgets import QFileDialog, QPlainTextEdit, QSizePolicy, QMessageBox
from PyQt5 import QtCore
from peacock.utils import WidgetUtils
from peacock.base.Plugin import Plugin
from peacock.utils.RecentlyUsedMenu import RecentlyUsedMenu
from .CheckInputWidget import CheckInputWidget
from .InputFileEditor import InputFileEditor

class InputFileEditorPlugin(InputFileEditor, Plugin):
    """
    The widget to edit the input file.
    In addition to InputFileEditor, this class adds menus and
    is available as a Plugin.
    """
    def __init__(self, **kwds):
        super(InputFileEditorPlugin, self).__init__(**kwds)
        self.setSizePolicy(QSizePolicy.Preferred, QSizePolicy.Preferred)

        self._menus_initialized = False
        self._recently_used_menu = None
        self._save_action = None
        self._open_action = None
        self._save_as_action = None
        self._clear_action = None
        self._check_action = None
        self._view_file = None

        self.check_widget = CheckInputWidget()
        self.check_widget.needInputFile.connect(self.writeInputFile)
        self.check_widget.hide()
        self.blockChanged.connect(self._updateChanged)

        self.input_file_view = QPlainTextEdit()
        self.input_file_view.setReadOnly(True)
        self.input_file_view.setWindowFlags(QtCore.Qt.Window)
        self.input_file_view.resize(640, 480)
        self.has_changed = False

        self._preferences.addInt("input/maxRecentlyUsed",
                "Max number of input files",
                20,
                1,
                50,
                "Set the maximum number of recent input files that have been used.",
                )

        self.setup()

    def _updateChanged(self, block, tree):
        self.has_changed = True

    def _askToSave(self, app_info, reason):
        if self.has_changed and app_info.valid() and self.tree and self.tree.input_filename and self.tree.incompatibleChanges(app_info):
            msg = "%s\nYou have unsaved changes in your input file, do you want to save?" % reason
            reply = QMessageBox.question(self, "Save?", msg, QMessageBox.Save, QMessageBox.Discard)
            if reply == QMessageBox.Save:
                self._saveInputFile()

    def executableInfoChanged(self, app_info):
        self._askToSave(app_info, "Reloading syntax from executable.")
        super(InputFileEditorPlugin, self).executableInfoChanged(app_info)
        self._setMenuStatus()
        self.has_changed = False

    def setInputFile(self, input_file):
        self._askToSave(self.tree.app_info, "Changing input files.")
        val = super(InputFileEditorPlugin, self).setInputFile(input_file)
        if self._menus_initialized:
            path = os.path.abspath(input_file)
            if os.path.exists(path):
                self._recently_used_menu.update(path)
            else:
                self._recently_used_menu.removeEntry(path)
            self._setMenuStatus()
        self.has_changed = False
        return val

    def _openInputFile(self):
        """
        Ask the user what input file to open.
        """
        input_name, other = QFileDialog.getOpenFileName(self, "Choose input file", os.getcwd(), "Input File (*.i)")
        if input_name:
            input_name = os.path.abspath(input_name)
            success = self.setInputFile(input_name)
            if success:
                self._recently_used_menu.update(input_name)
            else:
                self._recently_used_menu.removeEntry(input_name)

    def _saveInputFile(self):
        """
        Save the current input tree to the current filename.
        """
        self.writeInputFile(self.tree.input_filename)
        self.has_changed = False

    def _saveInputFileAs(self):
        """
        Ask the user what file to save the input tree to.
        """
        input_name, other = QFileDialog.getSaveFileName(self, "Choose input file", os.getcwd(), "Input File (*.i)")
        if input_name:
            input_name = os.path.abspath(input_name)
            self.writeInputFile(input_name)
            self.setInputFile(input_name)
            self._recently_used_menu.update(input_name)
            self.has_changed = False

    def _checkInputFile(self):
        """
        Show the input file check window.
        """
        self.check_widget.show()
        self.check_widget.check(self.tree.app_info.path)

    def _viewInputFile(self):
        """
        View the text of the current input tree.
        """
        if self.input_file_view.isVisible():
            self.input_file_view.hide()
        else:
            data = self.tree.getInputFileString()
            self.input_file_view.setPlainText(data)
            self.input_file_view.show()

    def _clearInputFile(self):
        self.tree.resetInputFile()
        self.tree.input_filename = None
        self.block_tree.setInputTree(self.tree)
        self.inputFileChanged.emit("")
        self._setMenuStatus()
        self.has_changed = False

    def _setMenuStatus(self):
        """
        Set the status of the menus.
        """
        if not self._menus_initialized:
            return
        enabled = self.tree.app_info.valid()
        if self.tree.input_filename:
            self._save_action.setEnabled(enabled)
        else:
            self._save_action.setEnabled(False)

        self._save_as_action.setEnabled(enabled)
        self._open_action.setEnabled(enabled)
        self._recently_used_menu.setEnabled(enabled)
        self._clear_action.setEnabled(enabled)
        self._check_action.setEnabled(enabled)
        self._view_file.setEnabled(enabled)

    def addToMenu(self, menu):
        """
        Register the menus specific to the InputTab.
        Input:
            menu[QMenu]: The menu to add the items to.
        """
        self._open_action = WidgetUtils.addAction(menu, "Open", self._openInputFile, "Ctrl+O")
        recentMenu = menu.addMenu("Recently opened")
        self._recently_used_menu = RecentlyUsedMenu(recentMenu,
                "input/recentlyUsed",
                "input/maxRecentlyUsed",
                20,
                )
        self._recently_used_menu.selected.connect(self.setInputFile)

        self._save_action = WidgetUtils.addAction(menu, "Save", self._saveInputFile)
        self._save_as_action = WidgetUtils.addAction(menu, "Save As", self._saveInputFileAs)
        self._clear_action = WidgetUtils.addAction(menu, "Clear", self._clearInputFile)
        self._check_action = WidgetUtils.addAction(menu, "Check", self._checkInputFile, "Ctrl+K")
        self._view_file = WidgetUtils.addAction(menu, "View current input file", self._viewInputFile, "Ctrl+V", True)
        self._menus_initialized = True
        self._setMenuStatus()

    def closing(self):
        self.check_widget.cleanup()

    def clearRecentlyUsed(self):
        if self._menus_initialized:
            self._recently_used_menu.clearValues()

    def onCurrentChanged(self, index):
        """
        This is called when the tab is changed.
        If the block editor window is open we want to raise it
        to the front so it doesn't get lost.
        """
        if index == self._index:
            if self.block_editor:
                self.block_editor.raise_()

if __name__ == "__main__":
    from PyQt5.QtWidgets import QApplication, QMainWindow
    from ExecutableInfo import ExecutableInfo
    import sys
    if len(sys.argv) != 3:
        print("Usage: %s <exe> <input file>" % sys.argv[0])
        sys.exit(1)

    qapp = QApplication(sys.argv)
    main_win = QMainWindow()
    w = InputFileEditorPlugin()
    main_win.setCentralWidget(w)
    exe_info = ExecutableInfo()
    exe_info.setPath(sys.argv[1])
    w.initialize()
    w.executableInfoChanged(exe_info)
    w.setInputFile(sys.argv[2])
    menubar = main_win.menuBar()
    menubar.setNativeMenuBar(False)
    input_menu = menubar.addMenu("Input File")
    w.addToMenu(input_menu)
    main_win.show()
    sys.exit(qapp.exec_())
