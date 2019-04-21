#!/usr/bin/env python2
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from PyQt5.QtWidgets import QWidget, QMessageBox
from PyQt5.QtCore import pyqtSignal
from peacock.base.Plugin import Plugin
from peacock.utils import WidgetUtils
from .JobRunner import JobRunner
import mooseutils
import time
import math
import os
import sys

class ExecuteRunnerPlugin(QWidget, Plugin):
    """
    Provides the run button to start execution. While running it shows the progress bar and a kill button.
    """
    needInputFile = pyqtSignal(str)
    needCommand = pyqtSignal()
    outputAdded = pyqtSignal(str)
    runProgress = pyqtSignal(int, int)
    saveLog = pyqtSignal()
    clearLog = pyqtSignal()

    """
    Called when a job is started.
    Args:
        bool: Whether CSV is enabled
        str: The path to the input file
        float: System time when the run starts
    """
    startJob = pyqtSignal(bool, str, float)

    def __init__(self):
        super(ExecuteRunnerPlugin, self).__init__()

        self._preferences.addBool("execute/clearLog",
                "Clear log before running",
                False,
                "Clear the output from previous runs before starting a new run",
                )

        self.top_layout = WidgetUtils.addLayout(vertical=True)

        self.run_layout = WidgetUtils.addLayout()
        self.run_layout.addStretch()

        self.run_button = WidgetUtils.addButton(self.run_layout, None, "Run", self.runClicked, enabled=False)
        self.kill_button = WidgetUtils.addButton(self.run_layout, None, "Kill", self.killClicked, enabled=False)
        self.clear_button = WidgetUtils.addButton(self.run_layout, None, "Clear log", self.clearLog, enabled=True)
        self.save_button = WidgetUtils.addButton(self.run_layout, None, "Save log", self.saveLog, enabled=True)

        self.run_layout.addStretch()

        self.progress_layout = WidgetUtils.addLayout()
        self.progress_label = WidgetUtils.addLabel(self.progress_layout, None, "Progress: ")
        self.progress_bar = WidgetUtils.addProgressBar(self.progress_layout, None)
        self._showProgressBar(False)

        self.setLayout(self.top_layout)
        self.top_layout.addLayout(self.run_layout)
        self.top_layout.addLayout(self.progress_layout)

        self.runner = JobRunner()

        self._total_steps = 0
        self.runner.finished.connect(self.runFinished)
        self.runner.outputAdded.connect(self.outputAdded)
        self.runner.timeStepUpdated.connect(lambda t: self.runProgress.emit(t, self._total_steps))
        self.runner.started.connect(lambda : self.runProgress.emit(0, self._total_steps))
        self.runner.timeStepUpdated.connect(self._updateProgressBar)
        self.exe_path = None
        self.exe_args = []
        self.has_csv = False
        self._input_file = ""

        self.setup()

    def setInputFile(self, input_file):
        if input_file:
            self._input_file = os.path.basename(input_file)
        else:
            self._input_file = ""

    def _tempInputFile(self):
        if self._input_file:
            tmp = "peacock_run_exe_tmp_%s" % self._input_file
        else:
            tmp = "peacock_run_exe_tmp.i"
        return os.path.abspath(tmp)

    def _showProgressBar(self, show):
        """
        Toggle showing the progress bar.
        Input:
            show: bool: Whether to show the progress bar.
        """
        if show:
            self.progress_label.show()
            self.progress_bar.show()
        else:
            self.progress_label.hide()
            self.progress_bar.hide()

    def _updateProgressBar(self, val):
        """
        Updates the progress of a running executable.
        Input:
            val: int of current progress
        """
        # add 1 so the first time step is progress
        self.progress_bar.setValue(val+1)

    def onNumTimeStepsChanged(self, num_steps):
        """
        Update the estimated number of time steps so that we
        can have a decent estimate for the progress bar
        Input:
            num_steps[int]: new number of time steps
        """
        self._total_steps = num_steps

    def runClicked(self):
        """
        Run button clicked.
        """
        input_file = self._tempInputFile()
        self.cleanup()
        self.needInputFile.emit(input_file)
        if not os.path.exists(input_file):
            mooseutils.mooseError("Input file didn't get written", dialog=True)
            return
        self.needCommand.emit()
        if self.runner.isRunning():
            self.runner.kill()
        self.kill_button.setDisabled(False)
        self._showProgressBar(True)
        self.progress_bar.setMaximum(11)
        self.progress_bar.setValue(0)

        if self._preferences.value("execute/clearLog"):
            self.clearLog.emit()

        start_time = math.floor(time.time()) if sys.platform == 'darwin' else time.time()
        self.startJob.emit(self.has_csv, input_file, start_time)
        self.runner.run(self.exe_path, self.exe_args + ["-i", os.path.relpath(input_file)])

    def setCommand(self, exe, args, csv_enabled):
        self.exe_path = exe
        self.exe_args = args
        self.has_csv = csv_enabled

    def killClicked(self):
        """
        Kill button clicked
        """
        button = QMessageBox.question(self, "Kill process?", "Terminate current run?", QMessageBox.Yes, QMessageBox.No)
        if button == QMessageBox.Yes:
            self.runner.kill()

    def runFinished(self, code, status):
        """
        Called when the executable has finished running.
        Input:
            code: Exit code
            status: A string of the status
        """
        self.kill_button.setDisabled(True)
        self.run_button.setDisabled(False)
        self.runProgress.emit(self._total_steps, self._total_steps)
        self._showProgressBar(False)

    def cleanup(self):
        try:
            os.remove(self._tempInputFile())
        except:
            pass

    def runEnabled(self, val):
        self.run_button.setEnabled(val)

if __name__ == "__main__":
    from peacock.utils import Testing
    from PyQt5.QtWidgets import QApplication
    qapp = QApplication(sys.argv)
    exe = Testing.find_moose_test_exe()
    w = ExecuteRunnerPlugin()
    w.setCommand(exe, [], False)
    def needInputFile(input_file):
        this_dir = os.path.dirname(os.path.abspath(__file__))
        peacock_dir = os.path.dirname(this_dir)
        test_file = os.path.join(peacock_dir, "tests", "common", "transient.i")
        with open(test_file, "r") as fin:
            data = fin.read()
            with open(input_file, "w") as fout:
                fout.write(data)
    w.needInputFile.connect(needInputFile)
    def print_out(t):
        print(t)
    w.outputAdded.connect(print_out)
    w.show()
    w.setEnabled(True)
    w.runEnabled(True)
    sys.exit(qapp.exec_())
