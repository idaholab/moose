import os, sys
from PySide import QtCore, QtGui

from src.base import *
from src.utils import *

##
# Widget for displaying executable results
class ExecuteConsoleControl(QtGui.QWidget, MooseWidget):

  def __init__(self, **kwargs):
    QtGui.QWidget.__init__(self)
    MooseWidget.__init__(self, **kwargs)

    self.addObject(QtGui.QPushButton(), handle='ConsoleKill')


    self.setup()


  def _setupConsoleKill(self, q_object):
    q_object.setText('Kill')
    q_object.setToolTip('Kill the current running process')


  def _callbackConsoleKill(self):
    self.object('Console').kill()
