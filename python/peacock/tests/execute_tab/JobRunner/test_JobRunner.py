#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from peacock.Execute.JobRunner import JobRunner
from peacock.utils import Testing
from PyQt5 import QtWidgets

class Tests(Testing.PeacockTester):
    qapp = QtWidgets.QApplication([])

    def setUp(self):
        super(Tests, self).setUp()
        self.runner = JobRunner()
        self.runner.finished.connect(self.finishedCalled)
        self.runner.outputAdded.connect(self.outputCalled)
        self.runner.started.connect(self.startCalled)
        self.runner.timeStepUpdated.connect(self.timestepCalled)
        self.runner.error.connect(self.errorCalled)
        self.start_count = 0
        self.finished_count = 0
        self.finished_code = 0
        self.finished_status = 0
        self.output_count = 0
        self.output = ""
        self.timestep_count = 0
        self.timestep = 0
        self.error_count = 0

    def errorCalled(self, err, err_msg):
        self.error_count += 1
        self.error_msg = err_msg

    def startCalled(self):
        self.start_count += 1

    def finishedCalled(self, code, status):
        self.finished_count += 1
        self.finished_code = code
        self.finished_status = status

    def outputCalled(self, output):
        self.output_count += 1
        self.output += output

    def timestepCalled(self, timestep):
        self.timestep_count += 1
        self.timestep = timestep

    def testRunError(self):
        self.runner.run("/no_exist", [])
        self.assertEqual(self.error_count, 1)
        self.assertIn("Failed", self.error_msg)
        self.runner.process.waitForFinished()
        self.assertEqual(self.output_count, 3)
        self.assertIn("Failed to start", self.output)

    def testRunBasic(self):
        self.runner.run("printf", ["hello\n"])
        self.runner.process.waitForFinished()
        self.assertEqual(self.error_count, 0)
        self.assertEqual(self.start_count, 1)
        self.assertEqual(self.timestep_count, 0)
        self.assertEqual(self.finished_count, 1)
        self.assertEqual(self.output_count, 4)
        self.assertIn("hello", self.output)

    def testRunTimeStep(self):
        self.runner.run("printf", ["Time Step 42\n"])
        self.runner.process.waitForFinished()
        self.assertEqual(self.error_count, 0)
        self.assertEqual(self.start_count, 1)
        self.assertEqual(self.timestep_count, 1)
        self.assertEqual(self.timestep, 42)
        self.assertEqual(self.finished_count, 1)
        self.assertEqual(self.output_count, 4)
        self.assertIn("Time Step 42", self.output)


    def testRunTerminalCodes(self):
        self.runner.run("printf", ["\33[31mfoo\33[39m\n"])
        self.runner.process.waitForFinished()
        self.assertEqual(self.error_count, 0)
        self.assertEqual(self.start_count, 1)
        self.assertEqual(self.finished_count, 1)
        self.assertEqual(self.output_count, 4)
        self.assertIn('<span style="color:red;">foo</span>', self.output)


if __name__ == '__main__':
    Testing.run_tests()
