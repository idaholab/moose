#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from peacock.Input.InputFileEditorPlugin import InputFileEditorPlugin
from PyQt5.QtWidgets import QMainWindow, QFileDialog, QApplication
from peacock.Input.ExecutableInfo import ExecutableInfo
from peacock.utils import Testing
import os
from mock import patch
from PyQt5.QtCore import QSettings

class Tests(Testing.PeacockTester):
    qapp = QApplication([])

    def setUp(self):
        super(Tests, self).setUp()
        self.block_changed_count = 0
        self.block = None
        self.input_file = os.path.abspath("../../common/transient.i")

    def tearDown(self):
        super(Tests, self).tearDown()
        Testing.remove_file("delete_me.i")
        Testing.remove_file("delete_me2.i")

    def blockChanged(self, block):
        self.block = block
        self.block_changed_count += 1

    def newWidget(self, app_init=True):
        main_win = QMainWindow()
        w = InputFileEditorPlugin()
        main_win.setCentralWidget(w)
        main_win.show()
        app_info = ExecutableInfo()
        if app_init:
            app_info.setPath(Testing.find_moose_test_exe())
            self.assertTrue(app_info.valid())
        menubar = main_win.menuBar()
        menubar.setNativeMenuBar(False)
        menu = menubar.addMenu("Input File")
        w.addToMenu(menu)
        w.clearRecentlyUsed()
        self.checkMenus(w, False)
        main_win.show()
        w.executableInfoChanged(app_info)
        w.blockChanged.connect(self.blockChanged)
        return main_win, w

    def checkMenus(self, editor, val, save=False, recent=False, recent_count=0):
        self.assertEqual(editor._save_as_action.isEnabled(), val)
        self.assertEqual(editor._save_action.isEnabled(), save)
        self.assertEqual(editor._clear_action.isEnabled(), val)
        self.assertEqual(editor._check_action.isEnabled(), val)
        self.assertEqual(editor._view_file.isEnabled(), val)
        self.assertEqual(editor._open_action.isEnabled(), val)
        self.assertEqual(editor._recently_used_menu._menu.isEnabled(), recent)
        self.assertEqual(editor._recently_used_menu.entryCount(), recent_count)

    def testInputFileEditorPlugin(self):
        main_win, w = self.newWidget()
        self.checkMenus(w, True)
        w.setInputFile(self.input_file)
        self.checkMenus(w, True, save=True, recent=True, recent_count=1)

        self.assertEqual(w.check_widget.isVisible(), False)
        w._check_action.triggered.emit()
        self.assertEqual(w.check_widget.isVisible(), True)
        self.assertIn("Syntax OK", w.check_widget.output.toPlainText())

        self.assertEqual(w.input_file_view.isVisible(), False)
        w._view_file.triggered.emit()
        self.assertEqual(w.input_file_view.isVisible(), True)
        self.assertIn("Kernel", w.input_file_view.toPlainText())

        w.writeInputFile("delete_me.i")

        w._clearInputFile()
        self.checkMenus(w, True, recent=True, recent_count=1)

    @patch.object(QFileDialog, "getOpenFileName")
    def testOpen(self, mock_open):
        main_win, w = self.newWidget()
        mock_open.return_value = "/no_exist", None
        self.checkMenus(w, True)
        w._open_action.triggered.emit()
        self.checkMenus(w, True)

        mock_open.return_value = self.input_file, None
        w._open_action.triggered.emit()
        self.checkMenus(w, True, save=True, recent=True, recent_count=1)

    @patch.object(QFileDialog, "getSaveFileName")
    def testSaveAs(self, mock_save):
        main_win, w = self.newWidget()
        mock_save.return_value = "delete_me.i", None
        self.checkMenus(w, True)
        w._save_as_action.triggered.emit()
        self.checkMenus(w, True, save=True, recent=True, recent_count=1)
        self.assertEqual(w.tree.input_filename, os.path.abspath("delete_me.i"))

        w._save_action.triggered.emit()
        self.checkMenus(w, True, save=True, recent=True, recent_count=1)

        mock_save.return_value = "delete_me2.i", None
        w._save_as_action.triggered.emit()
        self.checkMenus(w, True, save=True, recent=True, recent_count=2)
        self.assertEqual(w.tree.input_filename, os.path.abspath("delete_me2.i"))

    def testPrefs(self):
        main_win, w = self.newWidget()
        widgets = w.preferenceWidgets()
        self.assertEqual(len(widgets), 1)
        pref = widgets[0]
        pref.setValue(2)
        settings = QSettings()
        pref.save(settings)
        key = "input/maxRecentlyUsed"
        val = settings.value(key, type=int)
        self.assertEqual(val, 2)
        pref.setValue(15)
        pref.save(settings)
        val = settings.value(key, type=int)
        self.assertEqual(val, 15)

if __name__ == '__main__':
    Testing.run_tests()
