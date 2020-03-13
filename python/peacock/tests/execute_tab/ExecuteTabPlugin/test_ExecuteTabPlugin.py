#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from peacock.Execute.ExecuteTabPlugin import ExecuteTabPlugin
from PyQt5.QtWidgets import QMainWindow, QApplication
from PyQt5.QtTest import QTest
from PyQt5.QtCore import Qt, QSettings
from peacock.utils import Testing
from peacock.Input.InputTree import InputTree
from peacock.Input import TimeStepEstimate
import argparse
import re

class Tests(Testing.PeacockTester):
    qapp = QApplication([])

    def setUp(self):
        super(Tests, self).setUp()
        self.test_exe = Testing.find_moose_test_exe()
        self.test_input_file = "../../common/transient.i"
        self.start_input_file = None
        self.start_csv = None
        self.exe_info = None

    def newWidget(self, args=None):
        self.exe_info = None
        main_win = QMainWindow()
        w = ExecuteTabPlugin()
        main_win.setCentralWidget(w)
        w.needInputFile.connect(self.needInputFile)
        w.startJob.connect(self.startJob)
        w.executableInfoChanged.connect(self.exeInfoChanged)
        menubar = main_win.menuBar()
        if args:
            parser = argparse.ArgumentParser()
            w.commandLineArgs(parser)
            parsed_args = parser.parse_args(args)
            parsed_args.arguments = []
            w.initialize(parsed_args)
        w.addToMainMenu(menubar)
        main_win.show()
        return main_win, w

    def startJob(self, use_csv, input_file, t):
        self.start_input_file = input_file
        self.start_csv = use_csv

    def exeInfoChanged(self, exe_info):
        self.exe_info = exe_info

    def needInputFile(self, input_file):
        self.input_file = input_file
        data = None
        with open(self.test_input_file, "r") as fin:
            data = fin.read()
            with open(input_file, "w") as fout:
                fout.write(data)

    def testBasic(self):
        main_win, w = self.newWidget()
        w.ExecuteOptionsPlugin.setExecutablePath(self.test_exe)
        self.assertEqual(w.ExecuteRunnerPlugin.run_button.isEnabled(), True)
        self.assertEqual(self.exe_info.valid(), True)
        self.assertEqual(w.ConsoleOutputViewerPlugin.toPlainText(), "")
        w.ExecuteRunnerPlugin.runClicked()
        self.assertEqual(self.start_input_file, self.input_file)
        self.assertEqual(self.start_csv, True)
        w.ExecuteRunnerPlugin.runner.process.waitForFinished(-1)
        self.assertNotEqual(w.ConsoleOutputViewerPlugin.toPlainText(), "")
        tree = InputTree(self.exe_info)
        tree.setInputFile(self.test_input_file)
        num_steps = TimeStepEstimate.findTimeSteps(tree)
        w.onNumTimeStepsChanged(num_steps)

    def testCommandLine(self):
        main_win, w = self.newWidget(["-e", self.test_exe])
        self.assertEqual(w.ExecuteRunnerPlugin.run_button.isEnabled(), True)
        self.assertEqual(self.exe_info.valid(), True)

        main_win, w = self.newWidget(["--no-exe-search"])
        self.assertEqual(w.ExecuteRunnerPlugin.run_button.isEnabled(), False)
        self.assertEqual(self.exe_info, None)

        main_win, w = self.newWidget(["--method", "opt"])
        self.assertEqual(w.ExecuteRunnerPlugin.run_button.isEnabled(), False)
        self.assertEqual(self.exe_info, None)

    def testOptions(self):
        main_win, w = self.newWidget()
        w.ExecuteOptionsPlugin.setExecutablePath(self.test_exe)
        self.assertEqual(w.ExecuteRunnerPlugin.run_button.isEnabled(), True)
        self.assertEqual(self.exe_info.valid(), True)
        self.assertEqual(w.ConsoleOutputViewerPlugin.toPlainText(), "")
        QTest.mouseClick(w.ExecuteOptionsPlugin.mpi_checkbox, Qt.LeftButton)
        QTest.mouseClick(w.ExecuteOptionsPlugin.threads_checkbox, Qt.LeftButton)
        w.ExecuteRunnerPlugin.runClicked()
        self.assertEqual(self.start_input_file, self.input_file)
        self.assertEqual(self.start_csv, True)
        w.ExecuteRunnerPlugin.runner.process.waitForFinished(-1)
        output = w.ConsoleOutputViewerPlugin.toPlainText()
        self.assertNotEqual(output, "")
        m = re.search("Num Processors:\s*2", output)
        self.assertIsNotNone(m)
        m = re.search("Num Threads:\s*2", output)
        self.assertIsNotNone(m)
        tree = InputTree(self.exe_info)
        tree.setInputFile(self.test_input_file)
        num_steps = TimeStepEstimate.findTimeSteps(tree)
        w.onNumTimeStepsChanged(num_steps)

    def testPrefs(self):
        settings = QSettings()
        settings.setValue("execute/maxRecentWorkingDirs", 2)
        settings.setValue("execute/maxRecentExes", 3)
        settings.setValue("execute/maxRecentArgs", 4)
        settings.setValue("execute/mpiEnabled", True)
        settings.setValue("execute/mpiArgs", "foo bar")
        settings.setValue("execute/threadsEnabled", True)
        settings.setValue("execute/threadsArgs", "threads args")
        settings.sync()

        main_win, w = self.newWidget()
        ops = w.ExecuteOptionsPlugin
        self.assertEqual(ops.mpi_checkbox.isChecked(), True)
        self.assertEqual(ops.threads_checkbox.isChecked(), True)
        self.assertEqual(ops.mpi_line.text(), "foo bar")
        self.assertEqual(ops.threads_line.text(), "threads args")

        settings.setValue("execute/mpiEnabled", False)
        settings.setValue("execute/mpiArgs", "some args")
        settings.setValue("execute/threadsEnabled", False)
        settings.setValue("execute/threadsArgs", "other args")
        settings.sync()

        main_win, w = self.newWidget()
        ops = w.ExecuteOptionsPlugin
        self.assertEqual(ops.mpi_checkbox.isChecked(), False)
        self.assertEqual(ops.threads_checkbox.isChecked(), False)
        self.assertEqual(ops.mpi_line.text(), "some args")
        self.assertEqual(ops.threads_line.text(), "other args")

if __name__ == '__main__':
    Testing.run_tests()
