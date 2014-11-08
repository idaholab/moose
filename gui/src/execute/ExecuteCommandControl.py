import os, sys, traceback, glob
from PySide import QtCore, QtGui

from src.base import *
from src.utils import *

##
# Defines the top control buttons for the Execute Tab
#
# The purpose of this class is to provide the controls for creating the executable, the
# complete executable is accessed via "pull" named "executable".
class ExecuteCommandControl(QtGui.QFrame, MooseWidget):

  ## A signal emitted when the run button is pressed, see _callbackRun
  _signal_run = QtCore.Signal(str, list)

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

  ##
  # Executes when 'Run' is clicked (auto connected via addObject)
  #
  # This does not perform the actual execution of the command, it emits the
  # 'run' signal. This signal is connected to the execute method in the top
  # level ExecuteWidget
  def _callbackRun(self):

    # Extract the terms from the GUI
    executable = self.object('Executable').text()
    mpi = self.object('MPI').text()
    threads = self.object('Threads').text()
    args = self.object('Arguments').text().split(' ')

    # Check that program exists
    if not os.path.exists(executable):
      self.peacockError('The', executable, 'does not exist.')

    # Add MPI to the executable
    if len(mpi) > 0 and int(mpi) > 1:
      executable = 'mpiexec -n ' + str(mpi) + ' ' + executable

    # Add input file
    args += ['-i', 'peacock_run_tmp.i']

    # Add thread number
    if len(threads) and int(threads) > 1:
      args += ['--n-threads=', str(threads)]

    # Emit the signal with the executable and arguments
    self._signal_run.emit(executable, args)

  ##
  # Executes when 'Select' button is pressed (auto connected via addObject)
  def _callbackSelect(self):
    file_name = QtGui.QFileDialog.getOpenFileName(self, 'Select Executable...')
    self.object('Executable').setText(file_name[0])

  ##@{
  ##
  # Setup methods for the various controls of this widget
  def _setupRun(self, q_object):
    q_object.setText('Run')
    q_object.setToolTip('Run the executable with the supplied arguments')

  def _setupSelect(self, q_object):
    q_object.setText('Select')
    q_object.setToolTip('Select the executable to run')

  def _setupExecutable(self, q_object):

    # Utilize the user supplied executable
    cmd_line_exec = self.options['executable']
    if cmd_line_exec:
      q_object.setText(cmd_line_exec)
      return

    # Search for a moose executeable
    auto_exec = self._getExecutableHelper()
    if auto_exec:
      q_object.setText(auto_exec)
      return

    # Produce error
    print 'Warning: no MOOSE executable located'


  def _setupMPI(self, q_object):
    q_object.setMaximumWidth(40)
    q_object.setToolTip('Number of MPI processes to be used.')

  def _setupThreads(self, q_object):
    q_object.setMaximumWidth(40)
    q_object.setToolTip('Number of threads to be used.')

  def _setupMPILabel(self, q_object):
    self._labelHelper(q_object, 'MPI')

  def _setupThreadsLabel(self, q_object):
    self._labelHelper(q_object, 'Threads')

  def _setupArgumentsLabel(self, q_object):
    self._labelHelper(q_object, 'Arguments')

  def _setupExecutableLabel(self, q_object):
    self._labelHelper(q_object, 'Executable')
  ##@}

  ##
  # A helper method for setting up labels
  def _labelHelper(self, label_obj, edit_object_name):
    label_obj.setText(edit_object_name + ':')
    label_obj.setAlignment(QtCore.Qt.AlignRight)
    label_obj.setBuddy(self._objects[edit_object_name])

  ##
  # A helper method for locating a moose executable
  def _getExecutableHelper(self):

    # Locate the executable
    executable = None

   # Start with the current directory and continue until home directory is reached
    root = os.getcwd()
    while root != os.sep:

      # Determine current folder, assume it is the app name
      folder = root.split(os.sep)[-1]

      # Add the executable
      if os.path.join('moose', 'test') in root:
        full_exec = os.path.join(root, 'moose_test-*')
      else:
        full_exec = os.path.join(root, folder + '-*')

      # Perform wild-card search of executable, return the first hit if it exists
      app = glob.glob(full_exec)
      if app:
        if os.path.isfile(app[0]):
          executable = app[0]
          break

      root = os.path.realpath(os.path.join(root, '..'))

    # Return the path
    return executable
