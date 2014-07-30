import os, sys
from PySide import QtCore, QtGui

from base import *
from utils import *
from ExecuteWidgetTopControl import *
from ExecuteWidgetConsole import *

##
# The Peacock Execute Tab
class ExecuteWidget(MooseWidget):

# public:
  def __init__(self, **kwargs):
    MooseWidget.__init__(self, **kwargs)

    # Add the controls and console display
    self.addObject(ExecuteWidgetTopControl(**kwargs), handle='ExecuteTopControls')
    self.addObject(ExecuteWidgetConsole(**kwargs), handle='ExecuteConsole')

    # Create the menu items
    self.addObject(QtGui.QMenu(), handle='ExecuteMenu')

    #### DEMO: One method for adding a menu action ####
    # I would rather see each action added in the menu setup
    self.addObject(QtGui.QAction('&'+'Select executable', self),
                   handle='SelectMenuAction', parent='ExecuteMenu')

    # Connect the 'Run" button to the execute method
    self.connectSignal('run', self.execute)

    # Perform the setup for this object
    self.setup()

  ##
  # Method for running an executable and passing the output to the Console widget
  # @param executable The program to run
  # @param mpi The number of mpi to utilize
  # @param threeads The number of threads to use
  # @param arguments Additional command line arguments
  def execute(self, executable, mpi, threads, arguments=''):

    # Check that program exists
    if not os.path.exists(executable):
      peacockError(self, 'The program', executable, 'does not exist.')

    # Build the command
    cmd = []

    # Add MPI
    if len(mpi) > 0 and int(mpi) > 1:
      cmd.append('mpiexec -n ' + str(mpi))

    # Add executable
    os.chdir(os.path.dirname(executable))
    cmd.append(executable)

    # Add input file
    cmd.append('-i peacock_run_tmp.i')

    # Add thread number
    if len(threads) and int(threads) > 1:
      cmd.append('--n-threads=' + str(threads))

    # Append the additional arguments
    cmd.append(arguments)

    # Build the command and start a process
    cmd = ' '.join(cmd)
    self._debug('Command: ' + cmd)
    proc = QtCore.QProcess(self.object('Console'))
    proc.setProcessChannelMode(QtCore.QProcess.MergedChannels)
    proc.start(cmd)

    # Connect the Console (display) to the process
    self.object('Console').connect(proc, QtCore.SIGNAL("readyReadStandardOutput()"),
                                   lambda: self.object('ExecuteConsole').updateConsole(proc))

#private:

  ##
  # Setup the execute menu
  def _setupExecuteMenu(self, q_object):
    q_object.setTitle('Execute')

    # Add 'Run' menu item
    action = QtGui.QAction('&'+'Run', self)
    action.triggered.connect(self.callback('Run'))
    action.setShortcut(QtGui.QKeySequence(QtCore.Qt.CTRL + QtCore.Qt.Key_R))
    q_object.addAction(action)

    # Add 'Select' menu item
    #### I think we should do the same for 'Select' rather how it was done above

  ##
  # Add the 'Select' menu action
  def _setupSelectMenuAction(self, q_object):
    q_object.triggered.connect(self.callback('Select'))
    q_object.setShortcut(QtGui.QKeySequence(QtCore.Qt.CTRL + QtCore.Qt.Key_E))
