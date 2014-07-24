#!/usr/bin/env python
import os, sys, traceback

from PySide import QtCore, QtGui

from base import *

##
# An input file widget
class InputWidget(PeacockWidget):

  # Define signals that will be emitted from this object
  signal_button = QtCore.Signal()

  def __init__(self, **kwargs):
    PeacockWidget.__init__(self, **kwargs)

    self.addObject(QtGui.QPushButton('Run'), handle='Button')

    self.setup()

  def _callbackButton(self):
    print '_callbackButton'
    self.signal_button.emit()
