#!/usr/bin/env python
import os, sys, traceback

from PySide import QtCore, QtGui

from base import *
from utils import *

##
# An input file widget
class InputWidget(MooseWidget):

  # Define signals that will be emitted from this object
  _signal_button = QtCore.Signal()

  def __init__(self, **kwargs):
    MooseWidget.__init__(self, **kwargs)

    self.addObject(QtGui.QPushButton('Run'), handle='Button')
    self.setup()

    self._func = None

  ##
  # Initial Setup for the InputWidget
  # This populates the pull function, this cannot be done in the constructor because
  # this is pulling a function from the ExecuteWidget, which does not exist when the InputWidget
  # is being constructed.
  def _initialSetup(self):
    self._func = self.pull('ExecInfo')

  def _callbackButton(self):
    func = self.pull('ExecInfo')
    print self._func('--testing')
