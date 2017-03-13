#!/usr/bin/env python
from PyQt5.QtWidgets import QFileDialog, QPlainTextEdit
from PyQt5.QtCore import Qt, pyqtSignal
from peacock.utils import WidgetUtils
from peacock.base.Plugin import Plugin
from peacock.utils.RecentlyUsedMenu import RecentlyUsedMenu
from InputSettings import InputSettings
from CheckInputWidget import CheckInputWidget
from InputFileEditor import InputFileEditor
import os

class InputFileEditorPlugin(InputFileEditor, Plugin):
    """
    The widget to edit the input file.
    In addition to InputFileEditor, this class adds menus and
    is available as a Plugin.
    """
    highlightBC = pyqtSignal(list, list, list)

    def __init__(self, **kwds):
        super(InputFileEditorPlugin, self).__init__(**kwds)

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
        self.blockSelected.connect(self._updateChanged)

        self.input_file_view = QPlainTextEdit()
        self.input_file_view.setReadOnly(True)
        self.input_file_view.setWindowFlags(Qt.Window)
        self.input_file_view.resize(640, 480)
        self.has_changed = False

        self.setup()

    def _updateChanged(self, block, tree):
        self.has_changed = True

    def executableInfoChanged(self, app_info):
        super(InputFileEditorPlugin, self).executableInfoChanged(app_info)
        self._setMenuStatus()
        self.has_changed = False

    def setInputFile(self, input_file):
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
        input_name, other = QFileDialog.getOpenFileName(self, "Chooose input file", os.getcwd(), "Input File (*.i)")
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
        input_name, other = QFileDialog.getSaveFileName(self, "Chooose input file", os.getcwd(), "Input File (*.i)")
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
        self._recently_used_menu = RecentlyUsedMenu(recentMenu, InputSettings.RECENTLY_USED_KEY, InputSettings.MAX_RECENT_KEY, InputSettings.MAX_RECENT_DEFAULT)
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
