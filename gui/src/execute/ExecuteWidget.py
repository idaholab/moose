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
    self.addObject(ExecuteWidgetConsole(**kwargs), handle='ExecuteConsole')

    # Create the menu items
    self.addObject(QtGui.QMenu(), handle='ExecuteMenu')

    #### DEMO: One method for adding a menu action ####
    # I would rather see each action added in the menu setup
    #self.addObject(QtGui.QAction('&'+'Select executable', self),
    #               handle='SelectMenuAction', parent='ExecuteMenu')

    # Connect the 'Run" button to the execute method
    self.connectSignal('run', self.execute)

    # Perform the setup for this object
    self.setup()

  ##
  # Method for running an executable and passing the output to the Console widget
  # @param command The program to run
  def execute(self, cmd):

    cmd = '/Users/slauae/projects/moose/test/moose_test-oprof'
    args = ['-i', '/Users/slauae/projects/moose/test/tests/kernels/simple_transient_diffusion/simple_transient_diffusion.i', 'Mesh/uniform_refine=1']

    # Execute the command
    self._debug('Command: ' + cmd)

    self.runProcess = QtCore.QProcess(self)

   # pwd = os.getcwd()
   # os.chdir(os.path.dirname(cmd))
   # self.runProcess.setWorkingDirectory(os.path.dirname(cmd))

    console_obj = self.object('ExecuteConsole')
    self.runProcess.readyReadStandardOutput.connect(lambda: console_obj.updateConsole(self.runProcess))

    self.runProcess.start('/Users/slauae/projects/moose/test/moose_test-oprof', args)




  def writeOutput(self):
    while self.runProcess.canReadLine():
         #   if self.currentProcess is None:
         #       break

      text = self.runProcess.readLine()
      print text,

#output = process.stdout.read()
#    print output

    #proc = QtCore.QProcess(self.object('Console'))
    #proc.setProcessChannelMode(QtCore.QProcess.MergedChannels)
    #proc.start(cmd)

    # Connect the Console (display) to the process
    #self.object('Console').connect(proc, QtCore.SIGNAL("readyReadStandardOutput()"),
    #                               lambda: self.object('ExecuteConsole').updateConsole(proc))

#private:

  ##
  # Setup the execute menu
  #def _setupExecuteMenu(self, q_object):
  #  q_object.setTitle('Execute')
  #
  #  # Add 'Run' menu item
  #  action = QtGui.QAction('&'+'Run', self)
  #  action.triggered.connect(self.callback('Run'))
  #  action.setShortcut(QtGui.QKeySequence(QtCore.Qt.CTRL + QtCore.Qt.Key_R))
  #  q_object.addAction(action)
  #
  #  # Add 'Select' menu item
  #  #### I think we should do the same for 'Select' rather how it was done above

  ##
  # Add the 'Select' menu action
  #def _setupSelectMenuAction(self, q_object):
  #  q_object.triggered.connect(self.callback('Select'))
  #  q_object.setShortcut(QtGui.QKeySequence(QtCore.Qt.CTRL + QtCore.Qt.Key_E))
