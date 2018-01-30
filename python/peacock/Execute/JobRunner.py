#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from PyQt5.QtCore import pyqtSignal, pyqtSlot, QObject, QProcess
import re, os
import mooseutils
from peacock.base import MooseWidget
from peacock.utils import TerminalUtils

class JobRunner(QObject, MooseWidget):
    """
    Actually runs the process. It will read the output and
    translate any terminal color codes into html.
    It will also attempt to parse the output to check to
    see if we are at a new time step and emit the
    timestep_updated signal.
    Signals:
        started: Emitted when we start running.
        finished: Emitted when we are finished. Arguments are exit code and status message.
        outputAdded: Emitted when there is new output.
        timeStepUpdated: A new time step has started
        error: Emitted when an error is encountered. Arguments are QProcess code and error description
    """
    started = pyqtSignal()
    finished = pyqtSignal(int, str)
    outputAdded = pyqtSignal(str)
    timeStepUpdated = pyqtSignal(int)
    error = pyqtSignal(int, str)

    def __init__(self, **kwds):
        super(JobRunner, self).__init__(**kwds)

        self.process = QProcess(self)
        self.process.setProcessChannelMode(QProcess.MergedChannels)
        self.process.readyReadStandardOutput.connect(self._readOutput)
        self.process.finished.connect(self._jobFinished)
        self.process.started.connect(self.started)
        self.process.error.connect(self._error)
        self._error_map = { QProcess.FailedToStart: "Failed to start",
                QProcess.Crashed: "Crashed",
                QProcess.Timedout: "Timedout",
                QProcess.WriteError: "Write error",
                QProcess.ReadError: "Read error",
                QProcess.UnknownError: "Unknown error",
                }

        self.killed = False
        self.setup()

    def run(self, cmd, args):
        """
        Start the command.

        Arguments:
            cmd: The command to run
            args: A list of string arguments
        """
        self.killed = False
        self._sendMessage("Running command: %s %s" % (cmd, ' '.join(args)))
        self._sendMessage("Working directory: %s" % os.getcwd())
        self.process.start(cmd, args)
        self.process.waitForStarted()

    def _sendMessage(self, msg):
        mooseutils.mooseMessage(msg, color="MAGENTA")
        self.outputAdded.emit('<span style="color:magenta;">%s</span>' % msg)

    @pyqtSlot(QProcess.ProcessError)
    def _error(self, err):
        """
        Slot called when the QProcess encounters an error.
        Inputs:
            err: One of the QProcess.ProcessError enums
        """
        if not self.killed:
            msg = self._error_map.get(err, "Unknown error")
            self.error.emit(int(err), msg)
            mooseutils.mooseMessage(msg, color="RED")
            self.outputAdded.emit(msg)

    @pyqtSlot(int, QProcess.ExitStatus)
    def _jobFinished(self, code, status):
        """
        Slot called when the QProcess is finished.
        Inputs:
            code: Exit code of the process.
            status: QProcess.ExitStatus
        """
        exit_status = "Finished"
        if status != QProcess.NormalExit:
            if self.killed:
                exit_status = "Killed by user"
            else:
                exit_status = "Crashed"
        self.finished.emit(code, exit_status)
        self._sendMessage("%s: Exit code: %s" % (exit_status, code))

    def kill(self):
        """
        Kills the QProcess
        """
        self.killed = True
        mooseutils.mooseMessage("Killing")
        self.process.terminate()
        self.process.waitForFinished(1000)
        if self.isRunning():
            mooseutils.mooseMessage("Failed to terminate job cleanly. Doing a hard kill.")
            self.process.kill()
            self.process.waitForFinished()

    @pyqtSlot()
    def _readOutput(self):
        """
        Slot called when the QProcess produces output.
        """
        lines = []
        while self.process.canReadLine():
            tmp = self.process.readLine().data().decode("utf-8").rstrip()
            lines.append(TerminalUtils.terminalOutputToHtml(tmp))
            match = re.search(r'Time\sStep\s*([0-9]{1,})', tmp)
            if match:
                ts = int(match.group(1))
                self.timeStepUpdated.emit(ts)

        output = '<pre style="display: inline; margin: 0;">%s</pre>' % '\n'.join(lines)
        self.outputAdded.emit(output)

    def isRunning(self):
        return self.process.state() == QProcess.Running
