#!/usr/bin/python

from PySide import QtCore, QtGui
import csv


class PeacockErrorInterface:
  def __init__(self, **kwargs):

    self._testing = kwargs.pop('testing', False)
    self._last_error_message = None
    self._error_dialog = None
    self._has_dialog = False

    if isinstance(self, QtGui.QWidget):
      self._error_dialog = QtGui.QErrorMessage(self)
      self._has_dialog = True



  def peacockError(self, *args, **kwargs):

    message = ' '.join(args)
    self._last_error_message = message

    # Eat the message if running in testing mode
    if self._testing:
      return

    if self._has_dialog and kwargs.pop('dialog', True):
      self._error_message.showMessage(message)

    if kwargs.pop('screen', True):
      print message


  def getLastErrorMessage(self):
    return self._last_error_message
