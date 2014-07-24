import os, sys
from PySide import QtCore, QtGui

from base import *

##
# Widget for executing and displaying MOOSE executable results
class ExecuteWidgetConsole(PeacockWidget):

# public:
  def __init__(self, **kwargs):
    PeacockWidget.__init__(self, **kwargs)

    # Create progress bar and console window
    self.addObject(QtGui.QProgressBar(self), handle='ExecutionProgress', label='Progress:')
    self.addObject(QtGui.QTextEdit(), handle='Console')

    self.setup()

  ##
  # Collect the output and connect the advancement monitor for the bar
  def updateConsole(self, proc):
    current_output = str(proc.readAllStandardOutput())
    self.object('Console').append(current_output.rstrip('\n'))
    #self.progress(current_output)


#private:

  ##
  # Setup method for the Console display
  def _setupConsole(self, q_object):
    q_object.setMinimumHeight(300)
    q_object.setMinimumWidth(800)
    q_object.setFontFamily('Courier')
    q_object.setFontPointSize(10)
    q_object.setReadOnly(True)

    q_object.verticalScrollBar().setValue(q_object.verticalScrollBar().maximum())
