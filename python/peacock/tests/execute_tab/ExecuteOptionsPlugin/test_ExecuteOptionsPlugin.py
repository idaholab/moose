#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from peacock.Execute.ExecuteOptionsPlugin import ExecuteOptionsPlugin
from PyQt5.QtTest import QTest
from PyQt5.QtCore import Qt
from PyQt5.QtWidgets import QFileDialog, QMainWindow, QApplication
from peacock.utils import Testing
from mock import patch
import os

class Tests(Testing.PeacockTester):
    qapp = QApplication([])
    def setUp(self):
        super(Tests, self).setUp()
        self.test_exe = Testing.find_moose_test_exe()
        self.path = None
        self.working = None
        self.app_info = None

    def exeChanged(self, path):
        self.path = path

    def appInfoChanged(self, app_info):
        self.app_info = app_info

    def workingChanged(self, path):
        self.working = path

    def createWidget(self):
        main_win = QMainWindow()
        w = ExecuteOptionsPlugin()
        main_win.setCentralWidget(w)
        w.executableChanged.connect(self.exeChanged)
        w.executableInfoChanged.connect(self.appInfoChanged)
        w.workingDirChanged.connect(self.workingChanged)
        menubar = main_win.menuBar()
        menu = menubar.addMenu("Execute")
        w.show()
        w.addToMenu(menu)
        return main_win, w

    @patch.object(QFileDialog, "getOpenFileName")
    def testChooseExe(self, mock_open):
        main_win, w = self.createWidget()
        mock_open.return_value = "/no_exist", None
        QTest.mouseClick(w.choose_exe_button, Qt.LeftButton)
        self.assertEqual(w.exe_line.text(), "")

        mock_open.return_value = self.test_exe, None
        QTest.mouseClick(w.choose_exe_button, Qt.LeftButton)
        self.assertEqual(self.path, self.test_exe)
        self.assertEqual(self.app_info.valid(), True)
        self.assertEqual(w.exe_line.text(), self.test_exe)
        cmd, args = w.buildCommand("input.i")
        self.assertEqual(cmd, self.test_exe)
        self.assertEqual(len(args), 3)
        self.assertEqual(args[0], "Outputs/csv=true")
        self.assertEqual(args[1], "-i")
        self.assertEqual(args[2], "input.i")

        QTest.mouseClick(w.mpi_checkbox, Qt.LeftButton)
        self.assertEqual(w.mpi_checkbox.isChecked(), True)
        cmd, args = w.buildCommand("input.i")
        self.assertEqual(cmd, "mpiexec")

        self.assertNotIn("--recover", args)
        QTest.mouseClick(w.recover_checkbox, Qt.LeftButton)
        cmd, args = w.buildCommand("input.i")
        self.assertIn("--recover", args)

        self.assertNotIn("--n-threads=2", args)
        QTest.mouseClick(w.threads_checkbox, Qt.LeftButton)
        cmd, args = w.buildCommand("input.i")
        self.assertIn("--n-threads=2", args)

        cmd, args = w.buildCommand("input.i")
        self.assertEqual(cmd, "mpiexec")
        self.assertIn(self.test_exe, args)
        self.assertIn("-i", args)
        self.assertIn("input.i", args)
        self.assertIn("-n", args)
        self.assertIn("2", args)
        self.assertIn("--recover", args)
        self.assertIn("Outputs/csv=true", args)
        self.assertIn("--n-threads=2", args)

    @patch.object(QFileDialog, "getExistingDirectory")
    def testChooseWorking(self, mock_open):
        main_win, w = self.createWidget()
        mock_open.return_value = "/no_exist"
        QTest.mouseClick(w.choose_working_button, Qt.LeftButton)
        self.assertEqual(w.working_line.text(), os.getcwd())

        mock_open.return_value = "/"
        QTest.mouseClick(w.choose_working_button, Qt.LeftButton)
        self.assertEqual(self.working, "/")
        self.assertEqual(w.working_line.text(), "/")

    def testChangeArgs(self):
        main_win, w = self.createWidget()
        QTest.keyClicks(w.args_line, "-foo -bar")
        QTest.keyClick(w.args_line, Qt.Key_Enter)
        self.assertEqual(w.args_line.text(), "-foo -bar")

        cmd, args = w.buildCommand("input.i")
        self.assertIn("-foo", args)
        self.assertIn("-bar", args)

if __name__ == '__main__':
    Testing.run_tests()
