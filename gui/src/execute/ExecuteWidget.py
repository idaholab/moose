import os, sys, subprocess
from PySide import QtCore, QtGui

from src.base import *
from src.utils import *
from ExecuteCommandControl import *
from ExecuteWidgetConsole import *

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

    # Create the 'execute' menu items
    self.addObject(QtGui.QMenu(), handle='ExecuteMenu')

    # Connect the 'Run" button to the execute method
    self.connectSignal('run', self.execute)

    # Perform the setup for this object
    self.setup()


  ##
  # Method for running an executable and passing the output to the Console widget
  # @param cmd The program to run
  # @param args The arguments to pass to the program
  def execute(self, cmd, args):

#    cmd = '/Users/slauae/projects/moose/test/moose_test-oprof'
#    args = ['-i', '/Users/slauae/projects/moose/test/tests/kernels/simple_transient_diffusion/simple_transient_diffusion.i', 'Mesh/uniform_refine=1']

    # A debugging message
    self._debug('Running Command: ' + cmd)

    # Create the process object where the command will be executed
    process = QtCore.QProcess(self)

    # Get the object where the console information will be displayed and connect the process that will
    # display with the standard out/error
    console_obj = self.object('ExecuteConsole')
    process.readyReadStandardOutput.connect(lambda: console_obj.updateConsole(process))
    process.readyReadStandardError.connect(lambda: console_obj.updateConsole(process))

    # Run the command with supplied arguments
    process.start(cmd, args)


  ##
  # Setup the execute menu
  def _setupExecuteMenu(self, q_object):
    q_object.setTitle('Execute')

    # Add 'Run' menu item
    action = QtGui.QAction('&'+'Run', self)
    action.triggered.connect(self.execute)
    action.setShortcut(QtGui.QKeySequence(QtCore.Qt.CTRL + QtCore.Qt.Key_R))
    q_object.addAction(action)

    # Add 'Select' menu item
    action = QtGui.QAction('&'+'Select', self)
    action.triggered.connect(self.callback('Select'))
    action.setShortcut(QtGui.QKeySequence(QtCore.Qt.CTRL + QtCore.Qt.Key_F))
    q_object.addAction(action)
