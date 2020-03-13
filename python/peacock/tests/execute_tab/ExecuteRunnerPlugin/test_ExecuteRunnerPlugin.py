#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from peacock.Execute.ExecuteRunnerPlugin import ExecuteRunnerPlugin
from PyQt5.QtTest import QTest
from PyQt5.QtCore import Qt
from PyQt5 import QtWidgets
from peacock.utils import Testing
from peacock.Input.ExecutableInfo import ExecutableInfo
from peacock.Input.InputTree import InputTree
from peacock.Input import TimeStepEstimate

class Tests(Testing.PeacockTester):
    qapp = QtWidgets.QApplication([])
    def setUp(self):
        super(Tests, self).setUp()
        self.test_exe = Testing.find_moose_test_exe()
        self.test_input_file = "../../common/transient.i"
        self.input_count = 0
        self.input_file = None
        self.output = ""
        self.total_steps = 0
        self.current_step = 0
        self.progress_count = 0
        self.csv_enabled = False
        self.input_file_in_use = None

    def createWidget(self, args=[], csv_enabled=False):
        w = ExecuteRunnerPlugin()
        exe_info = ExecutableInfo()
        exe_info.setPath(self.test_exe)
        tree = InputTree(exe_info)
        tree.setInputFile(self.test_input_file)
        num_steps = TimeStepEstimate.findTimeSteps(tree)
        w.onNumTimeStepsChanged(num_steps)
        self.assertEqual(w._total_steps, 8)
        w.needCommand.connect(lambda: self.needCommand(w, args, csv_enabled))
        w.needInputFile.connect(self.needInputFile)
        w.outputAdded.connect(self.outputAdded)
        w.runProgress.connect(self.runProgress)
        w.startJob.connect(self.startJob)
        w.setEnabled(True)
        return w

    def needInputFile(self, input_file):
        self.input_count += 1
        self.input_file = input_file
        data = None
        with open(self.test_input_file, "r") as f:
            data = f.read()

        with open(input_file, "w") as f:
            f.write(data)

    def needCommand(self, runner, args, csv):
        runner.setCommand(self.test_exe, args, csv)

    def outputAdded(self, text):
        self.output += text

    def runProgress(self, current, total):
        self.current_step = current
        self.total_steps = total
        self.progress_count += 1

    def startJob(self, current, total, t):
        self.current_step = current
        self.total_steps = total

    def testBasic(self):
        w = self.createWidget()
        self.assertFalse(w.run_button.isEnabled())
        w.runEnabled(True)
        self.assertTrue(w.run_button.isEnabled())
        QTest.mouseClick(w.run_button, Qt.LeftButton)
        self.assertEqual(self.input_count, 1)
        self.assertEqual(w.exe_path, self.test_exe)
        self.assertEqual(w.exe_args, [])
        w.runner.process.waitForFinished(-1)
        self.assertNotEqual(self.output, "")
        self.assertGreater(len(self.output), 20)
        self.assertEqual(self.progress_count, 8)
        self.assertEqual(self.total_steps, 8)
        self.assertEqual(self.current_step, 8)

if __name__ == '__main__':
    Testing.run_tests()
