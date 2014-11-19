import os, sys
from PySide import QtCore, QtGui

from src.base import *
from src.utils import *

##
# Widget for displaying executable results
class ExecuteConsoleControl(QtGui.QWidget, MooseWidget):

  _signal_kill = QtCore.Signal()
  _signal_clear = QtCore.Signal()

  def __init__(self, **kwargs):
    QtGui.QWidget.__init__(self)
    MooseWidget.__init__(self, **kwargs)

    # Create the buttons for killing the execution and clearing the console window
    self.addObject(QtGui.QHBoxLayout(), handle='ConsoleControlButtonLayout')
    self.addObject(QtGui.QPushButton(), handle='KillConsole', parent='ConsoleControlButtonLayout')
    self.addObject(QtGui.QPushButton(), handle='ClearConsole', parent='ConsoleControlButtonLayout')

    self.setup()

  def _setupConsoleControlButtonLayout(self, q_object):
    q_object.addStretch(1)

  def _setupKillConsole(self, q_object):
    q_object.setDisabled(True)
    q_object.setText('Kill')
    q_object.setToolTip('Kill the current running process')

  def _setupClearConsole(self, q_object):
    q_object.setText('Clear')
    q_object.setToolTip('Clear the current console content')

  def _callbackKillConsole(self):
    self._signal_kill.emit()

  def _callbackClearConsole(self):
    self._signal_clear.emit()
