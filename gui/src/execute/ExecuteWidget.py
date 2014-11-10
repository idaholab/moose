import os, sys, subprocess
from PySide import QtCore, QtGui

from src.base import *
from src.utils import *
from ExecuteCommandControl import *
from ExecuteConsole import *
from ExecuteConsoleControl import *

##
# The Peacock Execute Tab
class ExecuteWidget(QtGui.QWidget, MooseWidget):

# public:
  def __init__(self, **kwargs):
    QtGui.QWidget.__init__(self)
    MooseWidget.__init__(self, **kwargs)

    # Storage for menu actions that are needed by execute and kill, these are populated
    # in _setupExecuteMenu
    self._kill_warning = None
    self._clear_warning = None

    # Add the controls and console display
    self.addObject(ExecuteCommandControl(**kwargs), handle='ExecuteCommandControl')
    self.addObject(ExecuteConsole(**kwargs), handle='ExecuteConsole')
    self.addObject(ExecuteConsoleControl(**kwargs), handle='ExecuteConsoleControl')

    # Create the 'execute' menu items
    self.addObject(QtGui.QMenu(), handle='ExecuteMenu')

    # Connect the control buttons
    self.connectSignal('run', self.execute)
    self.connectSignal('kill', self.kill)
    self.connectSignal('clear', self.clear)

    # Create the process object where the command will be executed
    self.process = QtCore.QProcess(self)

    # Perform the setup for this object
    self.setup()

  ##
  # Method for running an executable and passing the output to the Console widget
  # @param cmd The program to run
  # @param args The arguments to pass to the program
  def execute(self, cmd, args):

    # A debugging message
    self._debug('Running Command: ' + cmd)

    # Get the object where the console information will be displayed and connect the process that will
    # display with the standard out/error
    console_obj = self.object('ExecuteConsole')
    self.process.readyReadStandardOutput.connect(lambda: console_obj.updateConsole(self.process))
    self.process.readyReadStandardError.connect(lambda: console_obj.updateConsole(self.process))

    # Run the command with supplied arguments
    self.process.start(cmd, args)

  def kill(self):
    self._debug('Kill')

    if self._kill_warning.isChecked():
      self.peacockWarning('Kill the current running process?')


    self.process.kill()

  def clear(self):
    self._debug('Clear')
    self.object('Console').clear()

  ##
  # Setup the execute menu
  def _setupExecuteMenu(self, q_object):
    q_object.setTitle('Execute')

    # Add 'Run' menu item
    action = QtGui.QAction('&' + 'Run', self)
    action.triggered.connect(self.execute)
    action.setShortcut(QtGui.QKeySequence(QtCore.Qt.CTRL + QtCore.Qt.Key_R))
    q_object.addAction(action)

    # Add 'Kill' menu item
    action = QtGui.QAction('&' + 'Kill', self)
    action.triggered.connect(self.kill)
    action.setShortcut(QtGui.QKeySequence(QtCore.Qt.CTRL + QtCore.Qt.Key_K))
    q_object.addAction(action)

    # Add 'Select' menu item
    action = QtGui.QAction('&'+'Select', self)
    action.triggered.connect(self.callback('Select'))
    action.setShortcut(QtGui.QKeySequence(QtCore.Qt.CTRL + QtCore.Qt.Key_F))
    q_object.addAction(action)

    # Add the warning toggles
    q_object.addSeparator()
    self._clear_warning = QtGui.QAction('Clear Warning', self, checkable=True)
    self._clear_warning.setChecked(True)
    q_object.addAction(self._clear_warning)

    self._kill_warning = QtGui.QAction('Kill Warning', self, checkable=True)
    self._kill_warning.setChecked(True)
    q_object.addAction(self._kill_warning)
