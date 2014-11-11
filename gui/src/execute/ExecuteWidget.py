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

    # Add the controls and console display
    self.addObject(ExecuteCommandControl(**kwargs), handle='ExecuteCommandControl')
    self.addObject(ExecuteConsole(**kwargs), handle='ExecuteConsole')
    self.addObject(ExecuteConsoleControl(**kwargs), handle='ExecuteConsoleControl')

    # Create the 'Execute' menu and the menu items
    menu = self.addObject(QtGui.QMenu(), handle='ExecuteMenu')
    self.addObject(QtGui.QAction('&'+'Run', self), handle='ExecuteMenuRun', parent='ExecuteMenu')
    self.addObject(QtGui.QAction('&'+'Kill', self), handle='ExecuteMenuKill', parent='ExecuteMenu')
    menu.addSeparator()
    self.addObject(QtGui.QAction('&'+'Select', self), handle='ExecuteMenuSelect', parent='ExecuteMenu')
    menu.addSeparator()
    self._kill_warning = self.addObject(QtGui.QAction('Kill Warning', self),
                                         handle='ExecuteMenuKillWarning', parent='ExecuteMenu')
    self._clear_warning = self.addObject(QtGui.QAction('Clear Warning', self),
                                         handle='ExecuteMenuClearWarning', parent='ExecuteMenu')

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

    # Set enable/disable flags
    self.object('KillConsole').setEnabled(True)
    self.object('Run').setEnabled(False)
    self.object('Select').setEnabled(False)

    # Reset progress bar
    self.object('ExecuteConsole').resetProgress()

    # A debugging message
    self._debug('Running Command: ' + cmd)

    # Get the object where the console information will be displayed and connect the process that will
    # display with the standard out/error
    console_obj = self.object('ExecuteConsole')
    self.process.readyReadStandardOutput.connect(lambda: console_obj.updateConsole(self.process))
    self.process.readyReadStandardError.connect(lambda: console_obj.updateConsole(self.process))
    self.process.finished.connect(self._finished)

    # Run the command with supplied arguments
    self.process.start(cmd, args)

  ##
  # Method for killing the currently running executable
  def kill(self):
    self._debug('Killing current process')

    kill_process = True
    if self._kill_warning.isChecked():
      kill_process = self.peacockWarning('Kill the current running process?', cancel=True)

    if kill_process:
      self.process.kill()

  ##
  # Method for clearing the console window
  def clear(self):
    self._debug('Clear console')

    clear_console = True
    if self._clear_warning.isChecked():
      clear_console = self.peacockWarning('Clear the current console text?', cancel=True)

    if clear_console:
      self.object('Console').clear()

  ##
  # Method is called when the QProcess command is finished
  def _finished(self):
    self.object('KillConsole').setEnabled(False)
    self.object('Run').setEnabled(True)
    self.object('Select').setEnabled(True)

  ##
  # Setup the execute menu
  def _setupExecuteMenu(self, q_object):
    q_object.setTitle('Execute')

  ##
  # Setup method for the 'Run' Execute menu item
  def _setupExecuteMenuRun(self, q_object):
    # Connect the 'Run' menu item the to 'Run' button
    q_object.setShortcut(QtGui.QKeySequence(QtCore.Qt.CTRL + QtCore.Qt.Key_R))
    q_object.triggered.connect(self.callback('Run'))

  ##
  # Setup method for the 'Kill' Execute menu item
  def _setupExeuteMenuKill(self, q_object):
    q_object.setShortcut(QtGui.QKeySequence(QtCore.Qt.CTRL + QtCore.Qt.Key_K))
    q_object.triggered.connect(self.callback('Kill'))

  ##
  # Setup method for the 'Select' Execute menu item
  def _setupExeuteMenuSelect(self, q_object):
    q_object.setShortcut(QtGui.QKeySequence(QtCore.Qt.CTRL + QtCore.Qt.Key_F))
    q_object.triggered.connect(self.callback('Select'))

  ##
  # Setup method for the 'Kill warning' toggle
  def _setupExecuteMenuKillWarning(self, q_object):

    # Indicates that the menu item contains a check mark
    q_object.setCheckable(True)

    # Declare a preference for storing the kill warning status
    self.prefs.declare('kill_warning', True)

    # Apply the preference
    q_object.setChecked(self.prefs['kill_warning'])

  ##
  # Executes when the 'Kill warning' item is selected from the Execute menu
  def _callbackExecuteMenuKillWarning(self):
    self.prefs['kill_warning'] = self._kill_warning.isChecked()

  ##
  # Setup method for the 'Kill warning' toggle
  def _setupExecuteMenuClearWarning(self, q_object):

    # Indicates that the menu item contains a check mark
    q_object.setCheckable(True)

    # Declare a preference for storing the clear warning status
    self.prefs.declare('clear_warning', True)

    # Apply the preference
    q_object.setChecked(self.prefs['clear_warning'])

  ##
  # Executes when the 'Kill warning' item is selected from the Execute menu
  def _callbackExecuteMenuClearWarning(self):
    self.prefs['clear_warning'] = self._clear_warning.isChecked()
