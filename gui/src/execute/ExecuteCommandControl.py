import os, sys, traceback
from PySide import QtCore, QtGui

from src.base import *
from src.utils import *

##
# Defines the top control buttons for the Execute Tab
#
# The purpose of this class is to provide the controls for creating the executable, the
# complete executable is accessed via "pull" named "executable".
class ExecuteCommandControl(QtGui.QFrame, MooseWidget):

  _signal_run = QtCore.Signal(str)

# public:
  def __init__(self, **kwargs):
    QtGui.QFrame.__init__(self)
    MooseWidget.__init__(self, **kwargs)

    # Create a grid layout for placing buttons and such
    self.addObject(QtGui.QGridLayout(), handle='ControlButtonLayout')

    # Create the MPI and threads edit boxes
    self.addObject(QtGui.QLabel(), 1, 7, handle='MPILabel', parent='ControlButtonLayout')
    self.addObject(QtGui.QLineEdit(), 1, 8, handle='MPI', parent='ControlButtonLayout')
    self.addObject(QtGui.QLabel(), 1, 9, handle='ThreadsLabel', parent='ControlButtonLayout')
    self.addObject(QtGui.QLineEdit(), 1, 10, handle='Threads', parent='ControlButtonLayout')

    # Add the 'Run' button
    self.addObject(QtGui.QPushButton(), 1, 11, handle='Run', parent='ControlButtonLayout')

    # Add the executable and additional arguments fields
    self.addObject(QtGui.QLabel(), 2, 1, 1, 1, handle='ArgumentsLabel', parent='ControlButtonLayout')
    self.addObject(QtGui.QLineEdit(), 2, 2, 1, 10, handle='Arguments', parent='ControlButtonLayout')
    self.addObject(QtGui.QLabel(), 3, 1, 1, 1, handle='ExecutableLabel', parent='ControlButtonLayout')
    self.addObject(QtGui.QLineEdit(), 3, 2, 1, 9, handle='Executable', parent='ControlButtonLayout')

    # Add the executable selection
    self.addObject(QtGui.QPushButton(), 3, 11, 1, handle='Select', parent='ControlButtonLayout')

    # Run the setup methods
    self.setup()

    ## DEMO INFO ##
    self.info()

# protected:

  def _setupRun(self, q_object):
    q_object.setText('Run')

  ##
  # Executes when 'Run' is clicked (auto connected via addObject)
  def _callbackRun(self):

    # Extract the terms from the GUI
    executable = self.object('Executable').text()
    mpi = self.object('MPI').text()
    threads = self.object('Threads').text()
    args = self.object('Arguments').text()

    # Check that program exists
    #if not os.path.exists(executable):
    #  self.peacockError('The', executable, 'does not exist.')

    # Build the command
    cmd = []

    # Add MPI
    if len(mpi) > 0 and int(mpi) > 1:
      cmd.append('mpiexec -n ' + str(mpi))

    # Add executable
    #os.chdir(os.path.dirname(executable))
    cmd.append(executable)

    # Add input file
    cmd.append('-i peacock_run_tmp.i')

    # Add thread number
    if len(threads) and int(threads) > 1:
      cmd.append('--n-threads=' + str(threads))

    # Append the additional arguments
    cmd.append(args)
    self._signal_run.emit(cmd)

  def _setupSelect(self, q_object):
    q_object.setText('Select')

  def _setupMPILabel(self, q_object):
    self._labelHelper(q_object, 'MPI')

  def _setupThreadsLabel(self, q_object):
    self._labelHelper(q_object, 'Threads')

  def _setupArgumentsLabel(self, q_object):
    self._labelHelper(q_object, 'Arguments')

  def _setupExecutableLabel(self, q_object):
    self._labelHelper(q_object, 'Executable')

  ##
  # A helper method for setting up labels
  def _labelHelper(self, label_obj, edit_object_name):
    label_obj.setText(edit_object_name + ':')
    label_obj.setAlignment(QtCore.Qt.AlignRight)
    label_obj.setBuddy(self._objects[edit_object_name])

  ##
  # Executes when 'Select' button is pressed (auto connected via addObject)
  def _callbackSelect(self):
    file_name = QtGui.QFileDialog.getOpenFileName(self, 'Select Executable...')
    self.object('Executable').setText(file_name[0])

  ##
  # Setups the MPI control and label (auto called via setup())
  def _setupMPI(self, q_object):
    q_object.setMaximumWidth(40)
    q_object.setToolTip('Number of MPI processes to be used.')
    #q_object.property('label').setAlignment(QtCore.Qt.AlignRight)

  ##
  # Setups the threads control and label (auto called via setup())
  def _setupThreads(self, q_object):
    q_object.setMaximumWidth(40)
    q_object.setToolTip('Number of threads to be used.')
    #q_object.property('label').setAlignment(QtCore.Qt.AlignRight)

  ##
  # A simple test of the pull functionality
  def _pullExecInfo(self, *args):
    txt = self.object('Executable').text()
    if len(args) > 0:
      txt += ' ' + args[0]
    return txt
