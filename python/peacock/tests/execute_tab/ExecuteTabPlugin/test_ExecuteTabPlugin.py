#!/usr/bin/env python
from peacock.Execute.ExecuteTabPlugin import ExecuteTabPlugin
from PyQt5.QtWidgets import QMainWindow
from PyQt5.QtTest import QTest
from PyQt5.QtCore import Qt
from peacock.utils import Testing
from peacock.Input.InputTree import InputTree
from peacock.Input import TimeStepEstimate
import argparse
import re

class Tests(Testing.PeacockTester):
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
        kwargs = {}
        if args:
            parser = argparse.ArgumentParser()
            w.commandLineArgs(parser)
            parsed_args = parser.parse_args(args)
            parsed_args.arguments = []
            kwargs["cmd_line_options"] = parsed_args
        w.initialize(**kwargs)
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

if __name__ == '__main__':
    Testing.run_tests()
